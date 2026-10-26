"""Bounded x86 local value candidates, never a whole-program reachability proof.

Only straight-line 32-bit integer constants are propagated. Calls, branches,
incoming direct targets, gaps and unsupported instructions discard state. Reads
of file-backed memory describe initial-image candidates, not runtime contents.
"""
from collections import Counter
from dataclasses import dataclass
import re
import struct

REGS = {'eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp'}
MASK = 0xffffffff


@dataclass(frozen=True)
class Value:
    number: int
    trace: tuple = ()
    reads: tuple = ()


def value(number, row, *inputs, reads=()):
    trace = tuple(dict.fromkeys(p for v in inputs if v for p in v.trace))
    memory = tuple(dict.fromkeys(p for v in inputs if v for p in v.reads))
    return Value(number & MASK, tuple(dict.fromkeys(trace + (row['address'],))),
                 tuple(dict.fromkeys(memory + reads)))


def literal(text):
    if re.fullmatch(r'-?(?:0x[0-9a-f]+|[0-9]+)', text):
        return int(text, 16 if '0x' in text else 10) & MASK
    return None


def expression(text, registers, row):
    """Evaluate only the explicit Intel base+index*scale+displacement grammar."""
    text = text.replace(' ', '')
    if not text or not re.fullmatch(r'[a-z0-9x*+\-]+', text):
        return None
    parts = re.findall(r'[+\-]?[^+\-]+', text)
    if ''.join(parts) != text:
        return None
    total, inputs = 0, []
    for part in parts:
        sign = -1 if part.startswith('-') else 1
        term = part.lstrip('+-')
        pieces = term.split('*')
        if len(pieces) > 2:
            return None
        scale = 1
        if len(pieces) == 2:
            if pieces[1] not in ('1', '2', '4', '8') or pieces[0] not in REGS:
                return None
            scale = int(pieces[1])
        if pieces[0] in REGS:
            known = registers.get(pieces[0])
            if known is None:
                return None
            inputs.append(known)
            term_value = known.number
        else:
            term_value = literal(pieces[0])
            if term_value is None or len(pieces) != 1:
                return None
        total += sign * term_value * scale
    return value(total, row, *inputs)


def memory_address(operand, registers, row):
    match = re.fullmatch(r'(?:DWORD PTR )?(?:ds:)?\[(.+)\]', operand)
    if match:
        return expression(match[1], registers, row)
    match = re.fullmatch(r'(?:DWORD PTR )?ds:(0x[0-9a-f]+)', operand)
    return value(int(match[1], 16), row) if match else None


def direct_target(row):
    # Decoder output alone is not an independently validated instruction boundary.
    match = re.fullmatch(r'(?:0x)?([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
    return int(match[1], 16) if match else None


def inspect_values(rows, windows, read_word, roots=()):
    """Return all local candidates plus the unresolved indirect-site inventory.

    read_word must refuse writes/unbacked memory on its own terms. Its values
    are initial-image observations and may not be runtime constants. Store/load
    forwarding is deliberately omitted: alias and external writer closure is
    outside this analysis. No candidate can establish actual execution.
    """
    incoming = set(roots)
    for row in rows:
        if row['mnemonic'].startswith(('j', 'loop')) or row['mnemonic'] == 'call':
            target = direct_target(row)
            if target is not None:
                incoming.add(target)
    registers, stack = {}, []
    candidates, transfers, unsupported = [], [], Counter()
    previous_end = None
    reset_counts = Counter()

    def reset(reason):
        registers.clear()
        stack.clear()
        reset_counts[reason] += 1

    def read(operand, row):
        if operand in REGS:
            return registers.get(operand)
        immediate = literal(operand)
        if immediate is not None:
            return value(immediate, row)
        # Never reinterpret a BYTE/WORD/FWORD/QWORD load as a DWORD.
        if not operand.startswith('DWORD PTR '):
            return None
        address = memory_address(operand, registers, row)
        if address is None:
            return None
        try:
            word = read_word(address.number)
        except (ValueError, KeyError, struct.error):
            return None
        return value(word, row, address, reads=(address.number,))

    def emit(known, row, role):
        if known is None:
            return
        for route, (start, end) in windows.items():
            if start <= known.number < end and not start <= row['address'] < end:
                candidates.append(dict(route=route, site=row['address'], role=role,
                                       target=known.number, trace=list(known.trace),
                                       initial_image_reads=list(known.reads),
                                       confirmed_original_caller=False))

    for row in rows:
        address, mnemonic = row['address'], row['mnemonic']
        if previous_end != address or address in incoming:
            reset('gap_or_incoming_target')
        previous_end = address + len(bytes.fromhex(row['bytes']))
        operands = row['operand'].split(' #', 1)[0].split(',') if row['operand'] else []
        operands = [part.strip() for part in operands]
        dest = operands[0] if operands else ''
        result = None
        if mnemonic in ('call', 'jmp'):
            direct = direct_target(row)
            if direct is None:
                result = read(dest, row)
                transfers.append(dict(site=address, bytes=row['bytes'], mnemonic=mnemonic, operand=row['operand'],
                                      candidate_target=result.number if result else None,
                                      initial_image_reads=list(result.reads) if result else [],
                                      trace=list(result.trace) if result else [],
                                      runtime_target_recovered=False))
                emit(result, row, 'indirect_control_transfer')
            reset('call_or_jump')
            continue
        if mnemonic.startswith(('j', 'loop')):
            reset('conditional_transfer')
            continue
        if mnemonic in ('ret', 'retf'):
            if stack:
                emit(stack[-1], row, 'pushed_return_candidate')
            reset('return')
            continue
        if mnemonic == 'push' and len(operands) == 1:
            stack.append(read(dest, row))
            registers.pop('esp', None)
            continue
        if mnemonic == 'pop' and dest in REGS:
            result = stack.pop() if stack else None
            registers.pop('esp', None)
        elif mnemonic in ('mov', 'lea') and len(operands) == 2:
            if mnemonic == 'lea':
                result = memory_address(operands[1], registers, row)
            else:
                result = read(operands[1], row)
            emit(result, row, 'store' if dest not in REGS else 'value')
            if dest not in REGS:
                # An arbitrary store may change a future file-backed memory load.
                # Reset the entire local state instead of asserting alias safety.
                reset('memory_or_partial_register_write')
                continue
        elif mnemonic in ('add', 'sub', 'xor', 'or', 'and', 'shl', 'sal', 'shr', 'sar') and len(operands) == 2 and dest in REGS:
            left, right = registers.get(dest), read(operands[1], row)
            if mnemonic in ('xor', 'sub') and operands[1] == dest:
                result = value(0, row)
            elif left is not None and right is not None:
                a, b = left.number, right.number
                if mnemonic == 'add': number = a+b
                elif mnemonic == 'sub': number = a-b
                elif mnemonic == 'xor': number = a^b
                elif mnemonic == 'or': number = a|b
                elif mnemonic == 'and': number = a&b
                elif mnemonic in ('shl', 'sal'): number = a << (b & 31)
                elif mnemonic == 'shr': number = a >> (b & 31)
                else: number = (a if a < 0x80000000 else a-0x100000000) >> (b & 31)
                result = value(number, row, left, right)
            emit(result, row, 'arithmetic_value')
        elif mnemonic in ('inc', 'dec', 'neg', 'not') and dest in REGS:
            old = registers.get(dest)
            if old is not None:
                number = {'inc': old.number+1, 'dec': old.number-1,
                          'neg': -old.number, 'not': ~old.number}[mnemonic]
                result = value(number, row, old)
            emit(result, row, 'arithmetic_value')
        elif mnemonic in ('cmp', 'test', 'nop', 'cld', 'std'):
            continue
        else:
            unsupported[mnemonic] += 1
            reset('unsupported_instruction')
            continue
        if result is None:
            registers.pop(dest, None)
        else:
            registers[dest] = result
        if dest == 'esp':
            stack.clear()
    return dict(instruction_count=len(rows), candidates=candidates,
                indirect_transfers=transfers,
                reset_counts=dict(sorted(reset_counts.items())),
                unsupported_instructions=dict(sorted(unsupported.items())),
                confirmed_natural_callers=[], global_unreachable_proved=False)
