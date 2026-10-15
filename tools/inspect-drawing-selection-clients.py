#!/usr/bin/env python3
"""Pinned actor latch/count clients and bounded Region zero-mode click branch."""
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
        for entry in (0x4a5070, 0x5073d0, 0x5074f0):
            fn = entries.get(entry)
            if fn is None or len(fn['ranges']) != 1:
                raise ValueError('Missing/fragmented reviewed function')
            start, end = fn['ranges'][0]
            if start != entry or end - start > 4096:
                raise ValueError('Function boundary differs')
            windows.append((start, end, True))
        windows.append((0x4ff13a, 0x4ff1ca, False))
        result = []
        for start, end, whole in windows:
            output = subprocess.run(['objdump', '-d', '-Mintel', '--wide', f'--start-address={start}', f'--stop-address={end}', str(args.executable)], capture_output=True, text=True, check=True).stdout
            lines = [x for x in output.splitlines() if re.match(r'^\s*[0-9a-f]+:', x)]
            if not lines or int(lines[0].split(':')[0], 16) != start:
                raise ValueError('Entry alignment differs')
            at = pe.offset(data, start)
            result.append(dict(start=start, end=end, whole_inventory_function=whole, bytes_sha256=pe.sha(data[at:at + end - start]), instructions=lines))
        names = ['tools/inspect-drawing-selection-clients.py', 'tools/inspect-drawing-canvas-storage.py', 'research/runtime/coverage/binary-nocd.json']
        if pe.sha(args.executable.read_bytes()) != pe.BUILD:
            raise ValueError('Executable changed')
        report = dict(schema=1, success=True, sources={n: pe.sha((ROOT / n).read_bytes()) for n in names}, source_executable_sha256=pe.BUILD, windows=result, actor_field_offsets=dict(activity=4, creature_type=0xa8, owner=0x174, health=0xe4, raw_control_flag_68=0x68, raw_control_flag_70=0x70), raw_latch_count=0x6e984c, raw_mode=0x6903e8, original_game_executed=False, complete_selection_semantics_proved=False, scope='Exact inventory-bounded control dispatcher and actor latch setter/resetter, plus bounded Regionzero-mode maskbit1branch absent whole discovered function. Static instruction/offset evidence only; no running-game input, complete selection/highlight/lifetime semantics or native parity.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, functions=3, bounded_missing_function_branch=1)))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
