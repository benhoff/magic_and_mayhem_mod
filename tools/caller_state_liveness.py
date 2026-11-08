"""Conservative bit liveness of an original x86 caller continuation.

Reads are requirements, not evidence that the replacement produces the value.
Unknown transfers and surviving return values remain obligations. No calling
convention is used to discard volatile registers or processor flags.
"""
from collections import deque
import ctypes as C
import re

from original_x86 import Decoder, Instruction

ARITHMETIC = 0x8d5
REGISTERS = ('eax', 'ecx', 'edx', 'ebx', 'esp', 'ebp', 'esi', 'edi')
ALIASES = {r: (r, 0xffffffff) for r in REGISTERS}
for parent, word in zip(REGISTERS, ('ax', 'cx', 'dx', 'bx', 'sp', 'bp', 'si', 'di')):
    ALIASES[word] = (parent, 0xffff)
for parent, low, high in [('eax', 'al', 'ah'), ('ebx', 'bl', 'bh'),
                          ('ecx', 'cl', 'ch'), ('edx', 'dl', 'dh')]:
    ALIASES[low], ALIASES[high] = (parent, 255), (parent, 0xff00)


class AccessDecoder(Decoder):
    """The existing pinned Capstone-5 ABI, with explicit register access."""
    def __init__(self):
        super().__init__()
        lib = self.library
        lib.cs_option.argtypes = [C.c_size_t, C.c_int, C.c_size_t]
        if lib.cs_option(self.handle, 2, 3):  # CS_OPT_DETAIL, CS_OPT_ON
            raise ValueError('Cannot enable instruction detail')
        lib.cs_regs_access.argtypes = [C.c_size_t, C.POINTER(Instruction),
            C.POINTER(C.c_uint16), C.POINTER(C.c_uint8),
            C.POINTER(C.c_uint16), C.POINTER(C.c_uint8)]
        lib.cs_reg_name.argtypes = [C.c_size_t, C.c_uint]
        lib.cs_reg_name.restype = C.c_char_p

    def decode(self, raw, address):
        pointer = C.POINTER(Instruction)()
        count = self.library.cs_disasm(self.handle, raw, len(raw), address, 1,
                                       C.byref(pointer))
        if count != 1:
            raise ValueError('Cannot decode caller instruction at ' + hex(address))
        try:
            insn = pointer[0]
            if insn.address != address or not 1 <= insn.size <= 15:
                raise ValueError('Instruction ABI/extent disagreement')
            read, write = (C.c_uint16 * 64)(), (C.c_uint16 * 64)()
            nr, nw = C.c_uint8(), C.c_uint8()
            if self.library.cs_regs_access(self.handle, pointer, read, C.byref(nr), write, C.byref(nw)):
                raise ValueError('No register-access detail at ' + hex(address))
            return dict(address=address, size=insn.size,
                bytes=bytes(insn.raw[:insn.size]).hex(),
                mnemonic=insn.mnemonic.decode(), operand=insn.operands.decode(),
                reads=[self.library.cs_reg_name(self.handle, r).decode() for r in read[:nr.value]],
                writes=[self.library.cs_reg_name(self.handle, r).decode() for r in write[:nw.value]])
        finally:
            self.library.cs_free(pointer, count)


def masks(names):
    result = {}
    for name in names:
        if name in ALIASES:
            parent, mask = ALIASES[name]
            result[parent] = result.get(parent, 0) | mask
        elif name == 'eflags':
            result[name] = ARITHMETIC
    return result


def effects(row):
    """Only unconditional full arithmetic writers kill incoming flag bits."""
    reads, writes = masks(row['reads']), masks(row['writes'])
    mnemonic, args = row['mnemonic'], row['operand'].split(', ')
    if mnemonic.startswith('cmov') or mnemonic.startswith('cmpxchg'):
        # A failed condition keeps the old destination. Capstone's aggregate
        # write set does not by itself establish an unconditional overwrite.
        for reg, mask in tuple(writes.items()):
            if reg != 'eflags':
                reads[reg] = reads.get(reg, 0) | mask
                writes.pop(reg)
    if mnemonic in ('xor', 'sub', 'sbb') and len(args) == 2 and args[0] == args[1] and args[0] in ALIASES:
        reads.pop(ALIASES[args[0]][0], None)
    if mnemonic == 'and' and len(args) == 2 and args[0] in ALIASES and re.fullmatch('0x[0-9a-f]+', args[1]):
        parent, mask = ALIASES[args[0]]
        # An immediate mask cannot expose the bits it discards.
        immediate = int(args[1], 16) << (8 if mask == 0xff00 else 0)
        reads[parent] = reads.get(parent, 0) & immediate & mask
    # Capstone register access groups all EFLAGS. Keeping all bits live for
    # partial/unknown writers is conservative; never infer a full overwrite.
    if 'eflags' in writes:
        if mnemonic in ('add', 'sub', 'adc', 'sbb', 'cmp', 'test', 'and', 'or', 'xor', 'neg'):
            writes['eflags'] = ARITHMETIC
        elif mnemonic in ('inc', 'dec'):
            writes['eflags'] = ARITHMETIC & ~1
        elif mnemonic in ('clc', 'stc'):
            writes['eflags'] = 1
        else:
            writes.pop('eflags')
    return reads, writes


def inspect(start, decode, *, registers=None, limit=4096, returns=(), follow_calls=False,
            indirect_targets=None,call_preserved_unused=None):
    """Walk all direct successor paths; unknown control flow never passes."""
    incoming = registers if registers is not None else dict(eax=0xffffffff, ecx=0xffffffff,
                                                           edx=0xffffffff, eflags=ARITHMETIC)
    if not incoming or any(k not in (*REGISTERS, 'eflags') or type(v) is not int or not 0 < v <= 0xffffffff
                           for k, v in incoming.items()):
        raise ValueError('Require explicit nonempty register masks')
    if len(returns) > 32 or any(type(a) is not int or not 0 <= a <= 0xffffffff for a in returns):
        raise ValueError('Invalid bound return context')
    summaries=call_preserved_unused or {}
    for address,facts in summaries.items():
        if type(address)is not int or not 0<=address<=0xffffffff or not facts or any(
                r not in REGISTERS or type(m)is not int or not 0<m<=0xffffffff for r,m in facts.items()):
            raise ValueError('Invalid conditional preserved/unused callee facts')
    work = deque([(start, dict(incoming), tuple(returns))])
    seen, rows, reads, escapes,applied = set(), {}, {}, [],{}
    while work:
        address, live, stack = work.popleft()
        identity = address, tuple(sorted(live.items())), stack
        if identity in seen:
            continue
        seen.add(identity)
        if len(seen) > limit:
            raise ValueError('Caller liveness work bound exhausted')
        try:
            row = decode(address)
        except (ValueError, KeyError) as error:
            escapes.append(dict(address=address, reason=str(error), registers=live))
            continue
        rows[address] = row
        used, killed = effects(row)
        for reg, mask in used.items():
            needed = live.get(reg, 0) & mask
            if needed:
                reads[(address, reg)] = reads.get((address, reg), 0) | needed
        live = {reg: mask & ~killed.get(reg, 0) for reg, mask in live.items()
                if mask & ~killed.get(reg, 0)}
        if not live:
            continue
        m, operand = row['mnemonic'], row['operand']
        if m=='call'and follow_calls and address in summaries:
            # This is an explicit conditional fact, never an ABI inference.
            # The caller must separately prove that these incoming bits are
            # only privately saved/restored and that the actual target/domain
            # satisfies it. Target-operand reads above remain requirements.
            facts=summaries[address]
            uncovered={r:mask&~facts.get(r,0)for r,mask in live.items()if mask&~facts.get(r,0)}
            covered={r:mask&facts.get(r,0)for r,mask in live.items()if mask&facts.get(r,0)}
            if uncovered:escapes.append(dict(address=address,reason='call-outside-conditional-summary',registers=uncovered))
            if covered:
                previous=applied.setdefault(address,{})
                for reg,mask in covered.items():previous[reg]=previous.get(reg,0)|mask
                work.append((address+row['size'],covered,stack))
            continue
        if m in ('ret', 'retl') and stack:
            work.append((stack[-1], live, stack[:-1]))
            continue
        if m == 'call' and follow_calls and re.fullmatch('0x[0-9a-f]+', operand):
            if len(stack) >= 32:
                escapes.append(dict(address=address, reason='call-depth-bound', registers=live))
                continue
            work.append((int(operand, 16), live, (*stack, address + row['size'])))
            continue
        if m == 'call' and follow_calls and indirect_targets and address in indirect_targets:
            targets = indirect_targets[address]
            if not targets or len(set(targets)) != len(targets) or len(stack) >= 32:
                raise ValueError('Invalid or unbounded explicit indirect target set')
            for target in targets:
                if type(target) is not int or not 0 <= target <= 0xffffffff:
                    raise ValueError('Invalid explicit indirect target')
                work.append((target, dict(live), (*stack, address + row['size'])))
            continue
        if m.startswith(('ret', 'iret', 'call')) or m in ('int', 'int3', 'ud2', 'sysenter', 'hlt'):
            escapes.append(dict(address=address, reason='unresolved-' + m, registers=live))
            continue
        following = address + row['size']
        if m.startswith(('j', 'loop')):
            if not re.fullmatch('0x[0-9a-f]+', operand):
                escapes.append(dict(address=address, reason='indirect-transfer', registers=live))
                continue
            work.append((int(operand, 16), dict(live), stack))
            if m == 'jmp':
                continue
        work.append((following, live, stack))
    required = {}
    for (_, reg), mask in reads.items():
        required[reg] = required.get(reg, 0) | mask
    return dict(entry=start, incoming=incoming,
        required=required, reads=[dict(address=a, register=r, mask=m) for (a, r), m in sorted(reads.items())],
        escapes=escapes, instructions=[rows[a] for a in sorted(rows)],
        conditional_call_facts=[dict(address=a,preserved_unused=m)for a,m in sorted(applied.items())],
        conditional=bool(applied),
        closed=not escapes, semantic_equivalence=False,
        scope='Static read-before-overwrite requirements only; output equality, stack-memory and floating effects need separate proof. Any applied callee facts are explicit conditions needing independent target/domain/input-use and preservation proof.')
