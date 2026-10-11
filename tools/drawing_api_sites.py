"""Selected original CALL boundaries; decoding never supplies COM semantics."""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
from drawing_census import ROOT, BUILD_SHA256, direct_call, instructions, owners

REGISTERS = ('eax', 'ecx', 'edx', 'ebx', 'esp', 'ebp', 'esi', 'edi')
GRAPHICS_DLLS = {'gdi32.dll', 'ddraw.dll', 'msimg32.dll', 'amstream.dll', 'quartz.dll'}
GRAPHICS_USER_APIS = {'GetDC', 'ReleaseDC', 'BeginPaint', 'EndPaint'}


def graphics_import(item):
    return item['dll'].lower() in GRAPHICS_DLLS or (item['dll'].lower() == 'user32.dll'
        and item['name'] in GRAPHICS_USER_APIS)


def instruction_scope(entry, code, registered_entries, budget=4096):
    """Bounded normal-return CFG selection, not a recovered function boundary."""
    pending = [entry]; visited = {}; exits = []
    while pending:
        address = pending.pop()
        if address in visited: continue
        if address != entry and address in registered_entries:
            exits.append({'kind': 'registered_entry_boundary', 'target': address}); continue
        if len(visited) >= budget: raise ValueError('Reachable instruction budget exceeded')
        row = code.get(address)
        if row is None: raise ValueError('Missing selected instruction boundary: ' + hex(address))
        visited[address] = row
        mnemonic = row['mnemonic']; next_address = address + len(bytes.fromhex(row['bytes']))
        if mnemonic.startswith('ret'):
            exits.append({'kind': 'return', 'site': address}); continue
        if mnemonic.startswith(('j', 'loop')):
            match = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
            if match: pending.append(int(match[1], 16))
            else: exits.append({'kind': 'unresolved_branch', 'site': address, 'operand': row['operand']})
            if mnemonic != 'jmp': pending.append(next_address)
        else: pending.append(next_address)
    ranges = []
    for address, row in sorted(visited.items()):
        end = address + len(bytes.fromhex(row['bytes']))
        if ranges and ranges[-1][1] == address: ranges[-1][1] = end
        else: ranges.append([address, end])
    return {'ranges': ranges, 'exits': exits, 'instructions': len(visited),
            'scope': 'Reachable instruction selection assuming CALL normal return; no exceptions, indirect-target or function-wide boundary recovery.'}


def call_mode(row):
    """Check supported x86 encodings independently of objdump's operand text."""
    if row['mnemonic'] != 'call': raise ValueError('Not a CALL instruction')
    raw = bytes.fromhex(row['bytes'])
    if not raw: raise ValueError('Empty CALL bytes')
    if raw[0] == 0xe8:
        target = direct_call(row)
        if target is None: raise ValueError('Malformed rel32 CALL')
        return {'kind': 'direct_relative', 'target': target}
    unresolved = {'kind': 'unresolved_indirect', 'reason': 'Unsupported/prefixed/far CALL encoding'}
    if raw[0] != 0xff: return unresolved
    if len(raw) < 2: raise ValueError('Truncated FF CALL')
    modrm = raw[1]; mode = modrm >> 6; register = modrm & 7
    if (modrm >> 3) & 7 != 2: return unresolved
    operand = row['operand'].replace(' ', '').lower()
    if mode == 3:
        if len(raw) != 2 or operand != REGISTERS[register]:
            raise ValueError('Register CALL operand/extent mismatch')
        return {'kind': 'register', 'register': REGISTERS[register]}
    if register == 4:
        # Preserve this separately rather than treating ESP/SIB as a plain base.
        if len(raw) < 3: raise ValueError('Truncated SIB CALL')
        length = 3 + (4 if mode == 2 or mode == 0 and raw[2] & 7 == 5 else 1 if mode == 1 else 0)
        if len(raw) != length: raise ValueError('SIB CALL extent mismatch')
        return {'kind': 'unresolved_indirect', 'reason': 'SIB/indexed memory CALL requires separate target recovery'}
    if mode == 0 and register == 5:
        if len(raw) != 6: raise ValueError('Absolute CALL extent mismatch')
        address = struct.unpack_from('<I', raw, 2)[0]
        match = re.fullmatch(r'dwordptr(?:ds:)?(?:\[)?0x([0-9a-f]+)(?:\])?', operand)
        if not match or int(match[1], 16) != address:
            raise ValueError('Absolute CALL operand mismatch')
        return {'kind': 'absolute_memory', 'address': address}
    length = {0: 2, 1: 3, 2: 6}[mode]
    if len(raw) != length: raise ValueError('Memory CALL extent mismatch')
    displacement = 0 if mode == 0 else struct.unpack_from('<b' if mode == 1 else '<i', raw, 2)[0]
    match = re.fullmatch(r'dwordptr\[([a-z]+)(?:([+-])0x([0-9a-f]+))?\]', operand)
    shown = 0 if match and not match[2] else (int(match[3], 16) * (-1 if match[2] == '-' else 1) if match else None)
    if not match or match[1] != REGISTERS[register] or shown != displacement:
        raise ValueError('Memory CALL operand mismatch')
    return {'kind': 'register_memory', 'register': REGISTERS[register], 'displacement': displacement}


def selected_ranges(catalog, inventory, code=None):
    functions = {f['entry']: f for f in inventory['functions']}
    selected = {}
    for route in catalog['routes']:
        entry = route.get('entry')
        if entry in (0x4696e0, 0x469830, 0x469a00) or entry is not None and 0x58a9b0 <= entry < 0x58d3b0:
            if route.get('mode') == 'site': continue
            if entry in functions:
                selected[entry] = {'anchor': entry, 'ranges': functions[entry]['ranges'],
                                   'route': route['id'], 'kind': 'discovered_function'}
            else:
                if code is None: raise ValueError('Selected graphics entry has no discovered range')
                graph = instruction_scope(entry, code, set(functions))
                selected[entry] = {'anchor': None, 'selected_entry': entry, 'ranges': graph['ranges'],
                    'route': route['id'], 'kind': 'unassigned_reachable_instructions', 'normal_return_graph': graph}
    # This exact historical window is not promoted to a discovered function.
    if not any(r.get('entry') == 0x58d3b0 for r in catalog['routes']):
        raise ValueError('Missing separately retained WORD-copy window')
    selected[0x58d3b0] = {'anchor': None, 'ranges': [[0x58d3b0, 0x58d4a7]],
        'route': 'DRAW.entry.0058d3b0', 'kind': 'unassigned_selected_window'}
    return list(selected.values())


def boundary_rows(code, selections, inventory):
    imports = {item['iat_va']: item for item in inventory['imports']}
    result = []
    for address, row in sorted(code.items()):
        if row['mnemonic'] != 'call': continue
        containing = [selection for selection in selections
                      if any(start <= address < end for start, end in selection['ranges'])]
        exact_slot = None
        if row['bytes'].startswith('ff15') and len(bytes.fromhex(row['bytes'])) == 6:
            exact_slot = imports.get(struct.unpack_from('<I', bytes.fromhex(row['bytes']), 2)[0])
        if not containing and not (exact_slot and graphics_import(exact_slot)): continue
        if not containing:
            containing = [{'anchor': None, 'ranges': [[address, address + 6]],
                'kind': 'exact_graphics_import_call', 'import_iat': exact_slot['iat_va'],
                'scope': 'One exact imported graphics CALL across the whole image; no surrounding function recovery.'}]
        if any(not any(start <= address and address + len(bytes.fromhex(row['bytes'])) <= end
                       for start, end in item['ranges']) for item in containing):
            raise ValueError('Selected CALL crosses recorded range boundary')
        mode = call_mode(row)
        imported = imports.get(mode.get('address')) if mode['kind'] == 'absolute_memory' else None
        result.append({'site': address, 'return_pc': address + len(bytes.fromhex(row['bytes'])),
            'bytes': row['bytes'], 'operand': row['operand'], 'target_mode': mode,
            'selections': containing, 'discovered_owners': owners(address, inventory),
            'exact_import': imported, 'semantic_contract': 'unreviewed',
            'remaining': 'Stack cleanup, interface/type, actual target, output/side effects and lifetime require separate evidence.'})
    return result


def export(executable, inventory_path, catalog_path):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256: raise ValueError('Unsupported original executable')
    inventory = json.loads(inventory_path.read_text()); catalog = json.loads(catalog_path.read_text())
    if inventory['source_sha256'] != BUILD_SHA256 or catalog['source_sha256'] != BUILD_SHA256:
        raise ValueError('Selected graphics export build mismatch')
    # Wide output keeps all bytes of prefixed/long CALLs on their instruction row.
    assembly = subprocess.check_output(['objdump', '-w', '-d', '-Mintel', str(executable)], text=True)
    code = instructions(assembly)
    selections = selected_ranges(catalog, inventory, code)
    rows = boundary_rows(code, selections, inventory)
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256: raise ValueError('Original input changed')
    paths = ['tools/drawing_api_sites.py', 'tools/export-drawing-api-sites.py', 'tools/drawing_census.py',
             str(inventory_path.relative_to(ROOT)), str(catalog_path.relative_to(ROOT))]
    return {'schema': 1, 'source_sha256': BUILD_SHA256, 'image_base': inventory['image_base'],
        'input_unchanged': True, 'scope': 'Selected exact outgoing CALL boundaries and unresolved semantics; no runtime API/lifetime or complete drawing census.',
        'sources': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
        'assembly_sha256': hashlib.sha256(assembly.encode()).hexdigest(), 'selections': selections, 'calls': rows,
        'counts': {'selections': len(selections), 'calls': len(rows),
            'unresolved_encodings': sum(r['target_mode']['kind'] == 'unresolved_indirect' for r in rows),
            'exact_imports': sum(r['exact_import'] is not None for r in rows),
            'unassigned_registered_entries': sum(r['kind'] == 'unassigned_reachable_instructions' for r in selections),
            'graphics_import_calls_outside_selected_wrappers': sum(any(s['kind'] == 'exact_graphics_import_call' for s in r['selections']) for r in rows)}}
