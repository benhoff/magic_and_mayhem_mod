"""Build-pinned drawing call census; addresses never imply function ownership."""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD_SHA256 = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
INSTRUCTION = re.compile(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+)+)\s*(\S+)\s*(.*?)\s*$')


def instructions(assembly):
    rows = {}
    for line in assembly.splitlines():
        match = INSTRUCTION.match(line)
        if match:
            address = int(match[1], 16)
            row = {'address': address, 'bytes': ''.join(match[2].split()),
                   'mnemonic': match[3], 'operand': match[4]}
            if address in rows and rows[address] != row:
                raise ValueError('Ambiguous disassembly address')
            rows[address] = row
    return rows


def direct_call(row):
    raw = bytes.fromhex(row['bytes'])
    if row['mnemonic'] != 'call' or len(raw) != 5 or raw[0] != 0xe8:
        return None
    target = (row['address'] + 5 + struct.unpack('<i', raw[1:])[0]) & 0xffffffff
    operand = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
    if not operand or int(operand[1], 16) != target:
        raise ValueError('CALL operand disagrees with exact relative bytes')
    return target


def owners(address, inventory):
    return sorted(f['entry'] for f in inventory['functions']
                  if any(start <= address < end for start, end in f['ranges']))


def calls_for(rows, entries, inventory):
    result = []
    for address, row in sorted(rows.items()):
        target = direct_call(row)
        if target in entries:
            result.append({'call_va': address, 'return_pc': address + 5,
                           'entry': target, 'bytes': row['bytes'],
                           'discovered_owners': owners(address, inventory)})
    return result


def validate_callers(record, build, known_entries):
    if record.get('schema') != 1 or record.get('source_sha256') != build:
        raise ValueError('Supplemental caller census build/schema mismatch')
    if record.get('image_base') != 0x400000 or record.get('input_unchanged') is not True:
        raise ValueError('Supplemental caller census lacks pinned unchanged input')
    result = {}
    for call in record['callers']:
        address, target, pc = call['call_va'], call['entry'], call['return_pc']
        if any(type(v) is not int or not 0 <= v <= 0xffffffff for v in (address, target, pc)):
            raise ValueError('Invalid supplemental CALL identity')
        if target not in known_entries or pc != address + 5:
            raise ValueError('Supplemental CALL does not name an exact registered entry')
        row = {'address': address, 'bytes': call['bytes'], 'mnemonic': 'call', 'operand': hex(target)}
        if direct_call(row) != target:
            raise ValueError('Supplemental caller lacks exact direct CALL bytes')
        if pc in result:
            raise ValueError('Duplicate supplemental return PC')
        result[pc] = {'entry': target, 'call_va': address, 'discovered_owners': call['discovered_owners']}
    return result


def export(executable, catalog, inventory_path):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original executable; no export')
    inventory = json.loads(inventory_path.read_text())
    if inventory['source_sha256'] != BUILD_SHA256:
        raise ValueError('Discovery census build mismatch')
    assembly = subprocess.check_output(['objdump', '-d', '-Mintel', str(executable)], text=True)
    rows = instructions(assembly)
    entries = {int(s['identity'], 0) if isinstance(s['identity'], str) else s['identity']
               for route in catalog['routes'] for s in route['selectors'] if s['kind'] == 'entry'}
    calls = calls_for(rows, entries, inventory)
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Original input changed during export')
    return {'schema': 1, 'source_sha256': BUILD_SHA256, 'image_base': inventory['image_base'],
            'scope': 'Exact direct CALL sites to registered drawing entries across pinned objdump output. '
                     'Ownership uses containing discovered ranges only. No runtime module-base, indirect-target, '
                     'function-completeness or scenario-closure assertion.',
            'input_unchanged': True, 'entries': sorted(entries), 'callers': calls,
            'sources': {'tools/drawing_census.py': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                        str(inventory_path.relative_to(ROOT)): hashlib.sha256(inventory_path.read_bytes()).hexdigest()}}
