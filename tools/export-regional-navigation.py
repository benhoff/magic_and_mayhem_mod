#!/usr/bin/env python3
"""Pinned, read-only regional navigation disassembly and optional pseudocode."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'neighbors': (0x53a380, 0x53a5ad),
          'heuristic': (0x53a5b0, 0x53a630),
          'lookup': (0x53d7d0, 0x53e515),
          'lower-bound': (0x53e520, 0x53e5d7),
          'search': (0x53e5e0, 0x53ea50),
          'edge-distance': (0x53ea50, 0x53ea9e),
          'candidate': (0x513cf0, 0x513e3a),
          'displacement-probe': (0x51b300, 0x51b705)}


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--project', type=Path)
    p.add_argument('--headless', type=Path)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    manifest = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(manifest, check=True)
    try:
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(exe) != HASH:
            raise ValueError('Unsupported original executable')
        for name, (start, end) in RANGES.items():
            result = subprocess.run(['objdump', '-d', '-M', 'intel',
                                     f'--start-address={start}',
                                     f'--stop-address={end}', str(exe)],
                                    check=True, capture_output=True)
            (a.output / (name + '.asm')).write_bytes(result.stdout)
        if a.project:
            if not a.headless:
                p.error('--project requires --headless')
            env = dict(os.environ, XDG_CONFIG_HOME=str(a.output / 'config'),
                       XDG_CACHE_HOME=str(a.output / 'cache'),
                       GHIDRA_HEADLESS_JAVA_OPTIONS=
                       f'-Dapplication.cachedir={a.output}/cache -Dapplication.tempdir={a.output}/tmp')
            command = [str(a.headless), str(a.project), 'MagicMayhem',
                       '-process', 'Chaos.exe', '-readOnly', '-noanalysis',
                       '-scriptPath', str(ROOT / 'tools/ghidra'), '-postScript',
                       'ExportPersistenceProgression.java', str(a.output / 'c'),
                       *[f'{v[0]:x}' for v in RANGES.values()], '-max-cpu', '2']
            with (a.output / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log,
                               stderr=subprocess.STDOUT, check=True, timeout=600)
        if sha(exe) != HASH:
            raise ValueError('Original executable changed')
        report = {'success': True, 'original_sha256': HASH,
                  'scope': 'Static discovery only; no execution equivalence.',
                  'ranges': RANGES,
                  'sources': {'tools/export-regional-navigation.py': sha(Path(__file__))},
                  'artifacts': {str(q.relative_to(a.output)): sha(q)
                                for q in a.output.rglob('*')
                                if q.is_file() and (q.suffix in ('.asm', '.c', '.refs'))}}
        (a.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    finally:
        subprocess.run(manifest, check=True)


if __name__ == '__main__':
    main()
