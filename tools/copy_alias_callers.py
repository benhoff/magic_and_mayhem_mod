"""Conservative straight-line argument slices; never infer backing disjointness."""
from collections import Counter
import re

from drawing_census import direct_call

REGISTERS = frozenset(('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp'))
WRAPPERS = frozenset((0x58bf40, 0x58c140, 0x58c360, 0x58c4a0,
                      0x58c8a0, 0x58ca90, 0x58cbc0))


def value(constant=0, **terms):
    return (constant & 0xffffffff, tuple(sorted((k, v) for k, v in terms.items() if v)))


def add(left, right, scale=1):
    terms = dict(left[1])
    for key, coefficient in right[1]:
        terms[key] = terms.get(key, 0) + coefficient * scale
    return value(left[0] + right[0] * scale, **terms)


def display(expression):
    return dict(constant=expression[0], terms=dict(expression[1]))


class Block:
    def __init__(self, start):
        self.registers = {r: value(**{f'entry:{start:x}:{r}': 1}) for r in REGISTERS}
        self.memory_epoch = 0
        self.stack = {}
        self.start = start
        self.unsupported = []

    def address(self, text):
        result = value()
        for term in re.findall(r'[+-]?[^+-]+', text.replace(' ', '')):
            sign = -1 if term.startswith('-') else 1
            term = term.lstrip('+-')
            pieces = term.split('*')
            if pieces[0] in self.registers:
                scale = int(pieces[1], 0) if len(pieces) == 2 else 1
                result = add(result, self.registers[pieces[0]], sign * scale)
            elif len(pieces) == 1:
                result = add(result, value(int(term, 0)), sign)
            else:
                raise ValueError('Unsupported address term: ' + term)
        return result

    def operand(self, text):
        if text in self.registers:
            return self.registers[text]
        if re.fullmatch(r'0x[0-9a-f]+|[0-9]+', text):
            return value(int(text, 0))
        match = re.fullmatch(r'DWORD PTR (?:ds:)?(?:\[(.+)\]|(0x[0-9a-f]+))', text)
        if not match:
            raise ValueError('Unsupported operand: ' + text)
        address = self.address(match[1] or match[2])
        return self.stack.get(address, value(**{f'load:{self.memory_epoch}:{address!r}': 1}))

    def apply(self, row):
        mnemonic = row['mnemonic']
        operands = row['operand'].split(',')
        destination = operands[0]
        if mnemonic == 'push':
            pushed = self.operand(destination)
            self.registers['esp'] = add(self.registers['esp'], value(-4))
            self.stack[self.registers['esp']] = pushed
        elif mnemonic == 'pop' and destination in self.registers and destination != 'esp':
            self.registers[destination] = self.operand('DWORD PTR [esp]')
            self.registers['esp'] = add(self.registers['esp'], value(4))
        elif mnemonic == 'mov' and destination in self.registers:
            self.registers[destination] = self.operand(operands[1])
        elif mnemonic == 'lea' and destination in self.registers:
            match = re.fullmatch(r'\[(.+)\]', operands[1])
            if not match:
                raise ValueError('Unsupported LEA')
            self.registers[destination] = self.address(match[1])
        elif mnemonic in ('add', 'sub') and destination in self.registers:
            self.registers[destination] = add(self.registers[destination], self.operand(operands[1]),
                                              -1 if mnemonic == 'sub' else 1)
        elif mnemonic == 'xor' and len(operands) == 2 and destination == operands[1] and destination in REGISTERS:
            self.registers[destination] = value()
        elif mnemonic in ('inc', 'dec') and destination in REGISTERS:
            self.registers[destination] = add(self.registers[destination], value(1 if mnemonic == 'inc' else -1))
        elif mnemonic in ('cmp', 'test', 'nop'):
            pass
        elif mnemonic == 'mov' and 'PTR' in destination:
            # Any store can alias a load or the stack; never assume disjointness.
            self.memory_epoch += 1
            self.stack.clear()
        else:
            raise ValueError('Unsupported instruction: ' + mnemonic)


def inspect(code):
    """Enumerate all seven-wrapper direct calls; unknown flow stays unknown."""
    targets = set()
    for row in code.values():
        if row['mnemonic'].startswith(('j', 'loop')) or row['mnemonic'] == 'call':
            match = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
            if match:
                targets.add(int(match[1], 16))
    block = None
    rows = []
    previous_end = None
    for address, row in sorted(code.items()):
        if block is None or address in targets or address != previous_end:
            block = Block(address)
        previous_end = address + len(bytes.fromhex(row['bytes']))
        if row['mnemonic'] == 'call':
            target = direct_call(row)
            if target in WRAPPERS:
                source = block.registers['ecx']
                destination = block.operand('DWORD PTR [esp]')
                relation = ('same_wrapper_expression_candidate' if source == destination else
                            'distinct_constant_wrappers_backing_unknown' if not source[1] and not destination[1] else
                            'unresolved_wrapper_relationship')
                rows.append(dict(call=address, wrapper=target, block_start=block.start,
                                 source=display(source), destination=display(destination),
                                 relationship=relation, unsupported=block.unsupported,
                                 natural_alias_proved=False, backing_disjoint=False))
            block = None  # Unknown callee effects and stack cleanup are never guessed.
        elif row['mnemonic'].startswith(('j', 'loop', 'ret')):
            block = None
        else:
            try:
                block.apply(row)
            except ValueError as error:
                # Start a new unknown state after an instruction we cannot model.
                prior = block.unsupported + [dict(address=address, reason=str(error))]
                block = Block(previous_end)
                block.unsupported = prior
    return dict(calls=rows, counts=dict(Counter(row['relationship'] for row in rows)),
                direct_calls=len(rows), natural_alias_proved=False, global_absence_proved=False)
