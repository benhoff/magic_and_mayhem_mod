#!/usr/bin/env python3
"""Export a hash-pinned, whole-image discovery inventory, retaining unknowns."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = 'nocd-40209ca7'
EXPECTED = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def pe_sections(data):
    """Account for executable file-backed bytes, excluding virtual zero-fill."""
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    opt = pe + 24
    base = struct.unpack_from('<I', data, opt + 28)[0]
    table = opt + struct.unpack_from('<H', data, pe + 20)[0]
    sections = []
    for index in range(struct.unpack_from('<H', data, pe + 6)[0]):
        at = table + index * 40
        name = data[at:at+8].split(b'\0')[0].decode('ascii')
        virtual, rva, size, raw = struct.unpack_from('<4I', data, at + 8)
        flags = struct.unpack_from('<I', data, at + 36)[0]
        if raw + size > len(data):
            raise ValueError('Section exceeds input')
        sections.append({'name': name, 'start': base+rva, 'end': base+rva+size,
                         'virtual_size': virtual, 'file_offset': raw,
                         'executable': bool(flags & 0x20000000),
                         'sha256': hashlib.sha256(data[raw:raw+size]).hexdigest()})
    return base, sections


def uncovered(sections, functions):
    """Complement of inferred function bodies, not proof these bytes are code."""
    result = []
    for section in sections:
        if not section['executable']:
            continue
        cursor = section['start']
        spans = sorted((max(a, section['start']), min(b, section['end']))
                       for f in functions for a, b in f['ranges']
                       if a < section['end'] and b > section['start'])
        for start, end in spans:
            if start > cursor:
                result.append([cursor, start])
            cursor = max(cursor, end)
        if cursor < section['end']:
            result.append([cursor, section['end']])
    return result


def normalize(data, discovery):
    audit = module('file_api_inventory', ROOT/'tools/audit-file-apis.py')
    base, imports = audit.pe_imports(data)
    _, sections = pe_sections(data)
    def address(value):
        # Ghidra also emits external and entry-point pseudo-addresses. Preserve
        # those tokens without interpreting them as PE virtual addresses.
        return int(value, 16) if re.fullmatch(r'0x[0-9a-fA-F]+', value) else value

    functions = []
    for item in discovery['functions']:
        entry = int(item['entry'], 16)
        ranges = [[int(a, 16), int(b, 16)] for a, b in item['ranges']]
        if not ranges or not any(a <= entry < b for a, b in ranges):
            raise ValueError('Function entry outside inferred body')
        digest = hashlib.sha256()
        for a, b in ranges:
            if a >= b:
                raise ValueError('Empty/reversed function body')
            section = next((s for s in sections if s['start'] <= a < b <= s['end']), None)
            if section is None:
                raise ValueError(f'Function body not file-backed: {entry:x}')
            digest.update(data[section['file_offset']+a-section['start']:
                               section['file_offset']+b-section['start']])
        functions.append({'entry': entry, 'name': item['name'], 'ranges': ranges,
                          'sha256': digest.hexdigest(), 'thunk': item['thunk'],
                          'data_references': [address(x) for x in item['data_references']]})
    if len({f['entry'] for f in functions}) != len(functions) or not functions:
        raise ValueError('Empty or duplicate function inventory')
    flows = []
    function_entries = {f['entry'] for f in functions}
    for item in discovery['flows']:
        owner = address(item['owner']) if item['owner'] else None
        targets = [address(x) for x in item['targets']]
        # Ordinary local jumps remain in raw discovery; retain calls, computed
        # dispatch and direct jumps to other function entries in the baseline.
        if item['kind'] == 'jump' and not item['computed'] and not any(
                t in function_entries and t != owner for t in targets):
            continue
        flow = {'site': address(item['site']), 'owner': owner,
                'kind': item['kind'], 'computed': item['computed'], 'targets': targets}
        if item['computed']:
            flow['references'] = [{**r, 'target': address(r['target'])} for r in item['references']]
            flow['instruction'] = item['instruction']
        flows.append(flow)
    return {'schema': 1, 'build': BUILD, 'source_sha256': EXPECTED, 'image_base': base,
            'sections': sections, 'imports': imports, 'functions': functions, 'flows': flows,
            'unassigned_executable_ranges': uncovered(sections, functions),
            'limits': ['Ghidra discovery is inferred, not a complete function or behavior oracle.',
                       'Unassigned executable bytes include padding/data and undiscovered code.',
                       'Computed targets/data references are candidates, not complete dispatch/callback contracts.',
                       'External DLL, dynamically loaded, COM and OS behavior needs separate inventories.',
                       'No execution, equivalence or replacement coverage follows from discovery.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, default=ROOT/'working/game-nocd/Chaos.exe')
    parser.add_argument('--ghidra', type=Path)
    parser.add_argument('--output', type=Path, required=True, help='New inventory JSON; refuses overwrite')
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Output already exists; retain old inventory and compare a new export')
    verify = [str(ROOT/'tools/original-manifest.sh'), 'verify']
    subprocess.run(verify, check=True)
    try:
        data = args.executable.read_bytes()
        if hashlib.sha256(data).hexdigest() != EXPECTED:
            raise ValueError('Unsupported executable hash')
        support = module('decompile_inventory_support', ROOT/'tools/decompile-game.py')
        headless = support.find_headless(args.ghidra)
        if headless is None:
            raise ValueError('Ghidra headless not found')
        headless = support.prepare_native(headless)
        parent = ROOT/'working/tests/binary-inventory'
        parent.mkdir(parents=True, exist_ok=True)
        run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
        (run/'project').mkdir()
        print(f'Inventory run: {run}', flush=True)
        command = [str(headless), str(run/'project'), 'BinaryInventory', '-import', str(args.executable.resolve()),
                   '-scriptPath', str(ROOT/'tools/ghidra'), '-postScript', 'ExportBinaryInventory.java',
                   str(run/'discovery.json'), '-max-cpu', '2', '-analysisTimeoutPerFile', '600']
        env = os.environ.copy()
        env['XDG_CONFIG_HOME'] = str(run/'config')
        env['XDG_CACHE_HOME'] = str(run/'cache')
        env['GHIDRA_HEADLESS_JAVA_OPTIONS'] = env.get('GHIDRA_HEADLESS_JAVA_OPTIONS', '') + f' -Dapplication.cachedir={run / "cache"} -Dapplication.tempdir={run / "tmp"}'
        with (run/'ghidra.log').open('w') as log:
            subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        discovery = json.loads((run/'discovery.json').read_text())
        inventory = normalize(data, discovery)
        if args.executable.read_bytes() != data:
            raise ValueError('Executable changed during inventory')
        inventory['provenance'] = {'run': str(run.relative_to(ROOT)), 'command': command,
            'ghidra_version': headless.parent.parent.name,
            'discovery_sha256': hashlib.sha256((run/'discovery.json').read_bytes()).hexdigest(),
            'sources': {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in
                        ('tools/inventory-binary.py', 'tools/ghidra/ExportBinaryInventory.java', 'tools/audit-file-apis.py')}}
    finally:
        subprocess.run(verify, check=True)
    inventory['provenance']['original_manifest_verified_before_after'] = True
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as out:
        json.dump(inventory, out, separators=(',', ':'))
        out.write('\n')
    print(f'{len(inventory["functions"])} functions; {len(inventory["imports"])} imports; inventory: {args.output}')


if __name__ == '__main__':
    main()
