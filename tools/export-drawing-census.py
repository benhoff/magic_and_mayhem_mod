#!/usr/bin/env python3
"""Export exact drawing caller PCs from a verified original build."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT, export


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() or any(output.is_relative_to((ROOT / d).resolve()) for d in ('original', '.git')):
        parser.error('Output must be new and outside immutable original/Git roots')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        catalog = json.loads(evidence_text(ROOT / 'research/runtime/drawing-inventory.json'))
        report = export(args.executable, catalog, ROOT / 'research/runtime/coverage/binary-nocd.json')
        report['sources']['tools/export-drawing-census.py'] = hashlib.sha256(evidence_bytes(Path(__file__))).hexdigest()
        output.parent.mkdir(parents=True, exist_ok=True)
        with output.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps({'entries': len(report['entries']), 'direct_callers': len(report['callers'])}))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
