"""Conservative alternate-pointer discovery over a pinned decoded frontier.

Constant propagation is limited to straight-line blocks. Unknown register
values, incoming edges, bulk writers and external code remain unresolved. The
candidate search is never an absence or unreachability proof.
"""
import ast
import hashlib
import re

from original_x86 import Decoder

SLOTS = (0x6e1f68, 0x6e1f6c, 0x6e1f70, 0x6e1f74, 0x6e1f78)
REGS = {'eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'esp', 'ebp'}


def constant(expression, registers):
    def value(node):
        if isinstance(node, ast.Constant) and isinstance(node.value, int):
            return node.value
        if isinstance(node, ast.Name) and node.id in registers:
            return registers[node.id]
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
            return -value(node.operand)
        if isinstance(node, ast.BinOp):
            a, b = value(node.left), value(node.right)
            if isinstance(node.op, ast.Add): return a + b
            if isinstance(node.op, ast.Sub): return a - b
            if isinstance(node.op, ast.Mult): return a * b
        raise ValueError('Unknown constant')
    try:
        return value(ast.parse(expression, mode='eval').body) & 0xffffffff
    except (ValueError, KeyError, SyntaxError):
        return None


def inspect(image, frontier):
    rows = {row['address']: row for row in frontier['rows']}
    focused = []
    decoder = Decoder()
    try:
        for start, end in ((0x4e415a, 0x4e4177), (0x4e4c49, 0x4e4c53),
                           (0x4fd438, 0x4fd4f5), (0x57dc20, 0x57dc38),
                           (0x594508, 0x59455c), (0x59455c, 0x5945ec)):
            at = start
            while at < end:
                row = decoder.decode(image.read(at, min(15, end-at)), at)
                rows[at] = row
                at += row['size']
            if at != end: raise ValueError('Focused boundary differs')
            focused.append(dict(start=start, end=end,
                sha256=hashlib.sha256(image.read(start, end-start)).hexdigest()))
    finally:
        decoder.close()
    targets = set()
    for row in rows.values():
        if row['mnemonic'].startswith('j'):
            target = constant(row['operand'], {})
            if target is not None: targets.add(target)
    registers = {}; previous_end = None
    candidates, address_escapes, literal_operands = [], [], []
    unresolved_computed_writes = 0
    writes = {'mov', 'movd', 'movq', 'movups', 'movaps', 'movdqu', 'movdqa',
              'add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'not', 'neg',
              'shl', 'shr', 'sar', 'sal', 'adc', 'sbb', 'xchg', 'cmpxchg',
              'fst', 'fstp', 'fist', 'fistp', 'fisttp'}
    for address, row in sorted(rows.items()):
        if image.read(address, row['size']).hex() != row['bytes']:
            raise ValueError('Decoded frontier bytes changed at '+hex(address))
        if previous_end != address or address in targets: registers = {}
        previous_end = address + row['size']
        mnemonic, operand = row['mnemonic'], row['operand']
        args = [a.strip() for a in operand.split(',')]
        for slot in SLOTS:
            if re.search(r'(?<![0-9a-f])'+hex(slot)+r'(?![0-9a-f])', operand):
                literal_operands.append(dict(row, slot=slot))
        memory = re.search(r'\[([^]]+)\]', args[0])
        is_write = mnemonic in writes or mnemonic.startswith('set')
        if memory and is_write:
            destination = constant(memory[1], registers)
            width = 16 if 'xmmword' in args[0] else 8 if 'qword' in args[0] else 4 if 'dword' in args[0] else 2 if 'word' in args[0] else 1
            if destination is None:
                unresolved_computed_writes += 1
            elif any(destination < slot+4 and slot < destination+width for slot in SLOTS):
                candidates.append(dict(row, destination=destination, width=width,
                    constant_registers=dict(registers), classification='bounded_constant_write_candidate'))
        # An address value escaping to a call/other storage is a frontier too;
        # reading the pointer stored in a slot is not taking the slot's address.
        if mnemonic == 'push' and not memory:
            pointer = constant(args[0], registers)
            if pointer in SLOTS: address_escapes.append(dict(row, pointer=pointer))
        if len(args) > 1 and mnemonic == 'mov' and '[' not in args[1]:
            pointer = constant(args[1], registers)
            if pointer in SLOTS: address_escapes.append(dict(row, pointer=pointer))
        dest = args[0]
        if dest in REGS:
            result = None
            if mnemonic == 'mov' and len(args) == 2 and '[' not in args[1]:
                result = constant(args[1], registers)
            elif mnemonic == 'lea' and len(args) == 2:
                m = re.search(r'\[([^]]+)\]', args[1])
                if m: result = constant(m[1], registers)
            elif mnemonic in {'add', 'sub'} and len(args) == 2:
                result = constant(dest+('+' if mnemonic == 'add' else '-')+args[1], registers)
            elif mnemonic == 'xor' and len(args) == 2 and dest == args[1]:
                result = 0
            if mnemonic not in {'cmp', 'test', 'push'}:
                registers.pop(dest, None)
                if result is not None: registers[dest] = result
        elif re.fullmatch(r'[abcd][lh]|[abcd]x|[sd]i|[sb]p', dest):
            parent = {'al':'eax','ah':'eax','ax':'eax','bl':'ebx','bh':'ebx','bx':'ebx',
                      'cl':'ecx','ch':'ecx','cx':'ecx','dl':'edx','dh':'edx','dx':'edx',
                      'si':'esi','di':'edi','sp':'esp','bp':'ebp'}[dest]
            registers.pop(parent, None)
        if mnemonic.startswith(('j', 'ret', 'call', 'loop', 'rep')) or mnemonic in {'leave', 'int'}:
            registers = {}
    pointer_candidates = [c for c in candidates if c['destination'] < 0x6e1f70]
    return dict(success=True, static_only=True, decoded_rows_byte_checked=len(rows),
        focused_ranges=focused, literal_operands=literal_operands,
        bounded_constant_write_candidates=candidates, address_escape_candidates=address_escapes,
        pointer_publication_candidates=pointer_candidates,
        unresolved_computed_write_instructions=unresolved_computed_writes,
        retained_unresolved_indirect_edges=len(frontier['unresolved_indirect_edges']),
        global_initialization=[dict(address=slot, initial=image.read(slot,4,True).hex(),
                                   zero_fill=slot>=image.section(slot)['address']+image.section(slot)['stored']) for slot in SLOTS],
        original_allocation_recovered=False, pointer_relationship_recovered=False,
        initial_pixels_recovered=False, all_writers_recovered=False, retirement_recovered=False,
        no_candidates_proves_absence=False, live_reachability_proved=False,
        limitations=['Straight-line constant propagation is a candidate search, not exhaustive alias analysis.',
                     'Unknown registers, bulk operations, external writes and retained indirect flows stay unresolved.',
                     'Loader zero-fill establishes pointer initialization only, not backing allocation or clear.'])
