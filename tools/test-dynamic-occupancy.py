#!/usr/bin/env python3
"""Preserve a fresh bounded original occupancy/footprint comparison."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    parent = ROOT / 'working/tests/dynamic-occupancy'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='original-', dir=parent))
    executable = ROOT / 'working/game-nocd/Chaos.exe'
    manifest = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(manifest, check=True, cwd=ROOT)
    report = None
    try:
        if digest(executable) != EXPECTED:
            raise ValueError('Unsupported original build')
        snapshot = output / 'reference.exe'
        snapshot.write_bytes(executable.read_bytes())
        sources = sorted((ROOT / 'reconstruction/pathfinding').glob('*.cpp'))
        sources = [p for p in sources if p.name not in ('route-world-main.cpp', 'route-dump-main.cpp', 'main.cpp')]
        # Use the known recovered predicate dependencies, not unrelated CLI mains.
        names = ('route_request', 'route_search', 'route_neighbors', 'route_creature_acceptance',
                 'route_movement', 'route_cell', 'route_occupancy', 'route_validity',
                 'route_cell_validity', 'route_record_query', 'route_cell_support',
                 'route_boundary', 'route_clearance', 'route_scalar')
        sources = [ROOT / f'reconstruction/pathfinding/{name}.cpp' for name in names]
        binary = output / 'compare'
        command = [os.environ.get('CXX', 'g++'), '-m32', '-std=c++17', '-Wall', '-Wextra',
                   '-Werror', '-g', '-DMNM_NATIVE_REFERENCE', '-I', str(ROOT / 'reconstruction/pathfinding'),
                   *map(str, sources), str(ROOT / 'tests/route-occupancy-test.cpp'), '-o', str(binary)]
        subprocess.run(command, check=True, timeout=120)
        result = subprocess.run([str(binary), str(snapshot)], check=True, timeout=60,
                                capture_output=True, text=True)
        (output / 'comparison.stdout').write_text(result.stdout)
        (output / 'comparison.stderr').write_text(result.stderr)
        match = re.search(r'(\d+) cell cases, (\d+) integrated footprint cases', result.stdout)
        if not match or digest(executable) != EXPECTED or digest(snapshot) != EXPECTED:
            raise AssertionError('Comparison count or input identity missing')
        fingerprints = sources + list((ROOT / 'reconstruction/pathfinding').glob('*.hpp'))
        fingerprints += [ROOT / f'tests/{name}' for name in
                         ('route-occupancy-test.cpp', 'occupancy-native-reference.hpp', 'movement-native-reference.hpp')]
        fingerprints += [Path(__file__).resolve()]
        report = {'all_match': True, 'original_sha256': EXPECTED,
                  'cell_comparisons': int(match[1]), 'footprint_comparisons': int(match[2]),
                  'input_unchanged': True, 'live_validated': False,
                  'original_manifest_verified_before_after': True,
                  'scope': 'Original 004f3550 occupancy and 004f47d0 footprint; supplied valid owned cells/tables, exempt/self/foreign tokens, full-DWORD owner, extents/clipping/overflow and wrapped footprints. Five unrelated movement dependencies redirected privately by existing loader; neither compared routine calls them. No original occupancy production, pointer lifetime, moving reservations or live scheduling claim.',
                  'source_sha256': {str(p.relative_to(ROOT)): digest(p) for p in sorted(set(fingerprints))},
                  'command': command,
                  'output_sha256': {str(p.relative_to(ROOT)): digest(p) for p in
                                    (output / 'comparison.stdout', output / 'comparison.stderr')}}
    finally:
        subprocess.run(manifest, check=True, cwd=ROOT)
    (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'report': str((output / 'report.json').relative_to(ROOT)),
                      'cells': report['cell_comparisons'], 'footprints': report['footprint_comparisons']}))


if __name__ == '__main__':
    main()
