#!/usr/bin/env python3
"""Export pinned selected graphics/movie CALL sites; never patch an executable."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_api_sites import ROOT, export


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--virtual-candidates',action='store_true',help='Observe exact whole-image slot128 candidates without assigning API semantics')
    args = parser.parse_args(); output = args.output.resolve()
    if not output.is_relative_to(ROOT) or output.relative_to(ROOT).parts[0] in ('original', '.git'):
        raise ValueError('Export output must be a new repository-owned research/working artifact')
    if output.exists(): raise ValueError('Refusing to overwrite export history')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        record = export(args.executable, ROOT / 'research/runtime/coverage/binary-nocd.json',
                        ROOT / 'research/runtime/drawing-inventory.json',args.virtual_candidates)
        output.parent.mkdir(parents=True, exist_ok=True)
        with output.open('x') as out: json.dump(record, out, indent=2); out.write('\n')
        print(json.dumps(record['counts']))
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__': main()
