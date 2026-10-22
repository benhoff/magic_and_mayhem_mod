#!/usr/bin/env python3
"""Pin the mouse ring writer and bounded Region callback discovery, static only."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        spec = importlib.util.spec_from_file_location('canvas', ROOT / 'tools/inspect-drawing-canvas-storage.py')
        pe = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(pe)
        data = evidence_bytes(args.executable)
        if pe.sha(data) != pe.BUILD:
            raise ValueError('Unsupported executable')
        inventory = json.loads(evidence_text(ROOT / 'research/runtime/coverage/binary-nocd.json'))
        entries = {f['entry']: f for f in inventory['functions']}
        windows = {}
        for entry in (0x4a3720, 0x4a3800, 0x4a5000, 0x4bfda0, 0x469b30):
            fn = entries.get(entry)
            if fn is None or len(fn['ranges']) != 1:
                raise ValueError('Missing/fragmented reviewed function')
            start, end = fn['ranges'][0]
            if start != entry or end - start > 8192:
                raise ValueError('Function boundary differs')
            windows[hex(entry)] = (start, end, True)
        # These anchors have no discovered whole-function entry. Retain bounded
        # instruction windows and all existing unknown inventory ranges.
        windows['0x4fdad0'] = (0x4fdad0, 0x4fdcc6, False)
        windows['0x4ea048'] = (0x4ea048, 0x4ea057, False)
        windows['0x4ea119'] = (0x4ea119, 0x4ea124, False)
        for key, (start, end, whole) in list(windows.items()):
            output = subprocess.run(['objdump', '-d', '-Mintel', '--wide', f'--start-address={start}', f'--stop-address={end}', str(args.executable)], capture_output=True, text=True, check=True).stdout
            lines = [x for x in output.splitlines() if re.match(r'^\s*[0-9a-f]+:', x)]
            if not lines or int(lines[0].split(':')[0], 16) != start:
                raise ValueError('Entry alignment differs')
            at = pe.offset(data, start)
            windows[key] = dict(start=start, end=end, whole_inventory_function=whole, bytes_sha256=pe.sha(data[at:at + end - start]), instructions=lines)
        table = struct.unpack_from('<3I', data, pe.offset(data, 0x5c6a54))
        if table != (0x4fdad0, 0x464320, 0x4a4e20):
            raise ValueError('Region virtual anchors differ')
        anchors = {0x4bdd55: ('e846200000', 0x4bfda0), 0x4ea052: ('e8a997fbff', 0x4a3800), 0x4ea11f: ('e8dc96fbff', 0x4a3800)}
        calls = []
        for address, (expected, target) in anchors.items():
            raw = data[pe.offset(data, address):pe.offset(data, address) + 5]
            if raw.hex() != expected or address + 5 + int.from_bytes(raw[1:], 'little', signed=True) != target:
                raise ValueError('Exact call anchor differs')
            calls.append(dict(call=address, target=target, bytes=expected, whole_inventory_function=False))
        imports = [x for x in inventory['imports'] if x['iat_va'] == 0x5c5204]
        if len(imports) != 1 or imports[0]['name'] != 'GetAsyncKeyState':
            raise ValueError('Mouse writer import differs')
        names = ['tools/inspect-drawing-mouse-dispatch.py', 'tools/inspect-drawing-canvas-storage.py', 'research/runtime/coverage/binary-nocd.json']
        if pe.sha(evidence_bytes(args.executable)) != pe.BUILD:
            raise ValueError('Executable changed')
        report = dict(schema=1, success=True, sources={n: pe.sha(evidence_bytes(ROOT / n)) for n in names}, source_executable_sha256=pe.BUILD, windows=windows, calls=calls, region_virtual_table=dict(address=0x5c6a54, first_three_entries=list(table)), imports=imports, original_game_executed=False, persistent_selection_proved=False, tooltip_proved=False, scope='Exact static mouse initialization/ring writer/control deactivation/Region constructor/camera input functions, and bounded missing-function call/Region callback anchors. 4a5000 copies the current40byte record to a virtual slot0 event0x40; it is not established queue consumption. Region slot0 starts4fdad0, whose bounded prefix reads hit class. No complete callback, window-procedure recovery, runtime picking, selection, tooltip, queue lifetime or native equivalence claim.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, ring_writer='0x4a3800', region_callback='0x4fdad0')))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
