"""Queue branch control flow, with explicit shared exits and opaque call sites."""
import re
from drawing_census import direct_call, instructions

EXITS = {0x501184: {0x500848, 0x50084c},
         0x501210: {0x500d05, 0x500cfb, 0x500d32},
         0x50129c: {0x501122, 0x501126}}


def target(row):
    match = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
    return int(match[1], 16) if match else None


def branch(code, entry, exits):
    pending = [entry]; seen = set(); outside = set(); edges = []; calls = []
    while pending:
        address = pending.pop()
        if address in seen: continue
        if address in exits: outside.add(address); continue
        if address not in code: raise ValueError('Branch leaves retained instruction boundaries: ' + hex(address))
        seen.add(address); row = code[address]; end = address + len(bytes.fromhex(row['bytes']))
        if row['mnemonic'] == 'call':
            calls.append({'site': address, 'target': direct_call(row), 'operand': row['operand'],
                          'bytes': row['bytes']})
        if row['mnemonic'].startswith(('j', 'loop')):
            destination = target(row)
            edges.append({'site': address, 'target': destination, 'operand': row['operand']})
            if destination is None: raise ValueError('Unresolved indirect flow inside branch slice')
            pending.append(destination)
            if row['mnemonic'] != 'jmp': pending.append(end)
        elif not row['mnemonic'].startswith('ret'): pending.append(end)
    return {'entry': entry, 'instructions': [code[a] for a in sorted(seen)],
            'calls': sorted(calls, key=lambda r: r['site']),
            'edges': sorted(edges, key=lambda r: r['site']), 'shared_exits': sorted(outside)}


def slices(record):
    code = instructions(record['disassembly']); result = []
    for table in record['tables']:
        address = int(table['table'], 16)
        if address not in EXITS or [r['kind'] for r in table['entries']] != list(range(35)):
            raise ValueError('Unknown/incomplete queue dispatch table')
        entries = []
        for item in table['entries']:
            row = branch(code, int(item['target'], 16), EXITS[address]); row['kind'] = item['kind']
            row['downstream_pixel_semantics'] = 'original_only_pending_separate_comparison'
            entries.append(row)
        result.append({'table': address, 'dispatch': int(table['dispatch'], 16), 'entries': entries})
    if len(result) != 3: raise ValueError('Missing queue dispatch mode')
    return result
