#!/usr/bin/env python3
"""Pinned cursor mutex creation, nested wait clients and bounded disposal."""
import argparse
import importlib.util
import json
from pathlib import Path
import re
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
        data = args.executable.read_bytes()
        if pe.sha(data) != pe.BUILD:
            raise ValueError('Unsupported executable')
        inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text())
        entries = {f['entry']: f for f in inventory['functions']}
        windows = []
        for entry in (0x4a30b0, 0x4a37d0, 0x4a39a0, 0x4a45b0, 0x4a4910, 0x4e8d80):
            fn = entries.get(entry)
            if fn is None or len(fn['ranges']) != 1:
                raise ValueError('Missing/fragmented reviewed function')
            start, end = fn['ranges'][0]
            if start != entry or end - start > 8192:
                raise ValueError('Function boundary differs')
            windows.append((start, end, True))
        windows.append((0x4e9df6, 0x4e9e10, False))
        result = []
        for start, end, whole in windows:
            output = subprocess.run(['objdump', '-d', '-Mintel', '--wide', f'--start-address={start}', f'--stop-address={end}', str(args.executable)], capture_output=True, text=True, check=True).stdout
            lines = [x for x in output.splitlines() if re.match(r'^\s*[0-9a-f]+:', x)]
            if not lines or int(lines[0].split(':')[0], 16) != start:
                raise ValueError('Entry alignment differs')
            at = pe.offset(data, start)
            result.append(dict(start=start, end=end, whole_inventory_function=whole, bytes_sha256=pe.sha(data[at:at + end - start]), instructions=lines))
        imports = [x for x in inventory['imports'] if x['iat_va'] in (0x5c50a0, 0x5c50b8, 0x5c513c, 0x5c5140)]
        if {x['name'] for x in imports} != {'CloseHandle', 'CreateMutexA', 'ReleaseMutex', 'WaitForSingleObject'}:
            raise ValueError('Mutex imports differ')
        names = ['tools/inspect-drawing-cursor-mutexes.py', 'tools/inspect-drawing-canvas-storage.py', 'research/runtime/coverage/binary-nocd.json']
        if pe.sha(args.executable.read_bytes()) != pe.BUILD:
            raise ValueError('Executable changed')
        report = dict(schema=1, success=True, sources={n: pe.sha((ROOT / n).read_bytes()) for n in names}, source_executable_sha256=pe.BUILD, windows=result, imports=imports, original_game_executed=False, mutex_ownership_proved=False, safe_source_synchronization_proved=False, scope='Exact static cursor mutex clients: both creations pass three zero arguments; foreground uses642c58 with10ms wait; timer first waits6e1ec0with10ms then642c58withFFFFFFFF when repainting, releases642c58before6e1ec0. Nested second wait result is not branched on. First nonzero result skips release. Disposal clears globals after optional CloseHandle;4e9df6is bounded absent whole function. No runtime handle lifetime, complete aliases/lock order, abandoned/timeout ownership or safe diagnostic source protection proof.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, inventoried_functions=6, bounded_disposal_window=1)))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
