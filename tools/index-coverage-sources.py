#!/usr/bin/env python3
"""Index current sources against explicitly registered implementation/test links."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = ('assets', 'audio', 'renderer', 'reconstruction', 'runtime', 'compat', 'apps', 'game', 'protocols', 'tools', 'tests')
SUFFIXES = {'.cpp', '.hpp', '.c', '.h', '.inc', '.S', '.py', '.java', '.sh', '.json', '.html', '.css', '.js'}


def source_paths(root):
    return sorted(p.relative_to(root).as_posix() for name in SOURCE_ROOTS for p in (root/name).rglob('*')
                  if p.is_file() and (p.suffix in SUFFIXES or p.name == 'CMakeLists.txt')
                  and '__pycache__' not in p.parts)


def index(root, register):
    rows = []
    for path in source_paths(root):
        references = [{'behavior': b['id'], 'role': role} for b in register['behaviors']
                      for role in ('implementation', 'tests') if path in b[role]]
        prefix = '/'.join(Path(path).parts[:2]) if path.startswith(('apps/', 'runtime/', 'reconstruction/', 'compat/')) else Path(path).parts[0]
        role = 'build' if Path(path).name == 'CMakeLists.txt' else ('test' if path.startswith('tests/') else
               'tool' if path.startswith('tools/') else 'protocol' if path.startswith('protocols/') else 'code')
        rows.append({'path': path, 'sha256': hashlib.sha256((root/path).read_bytes()).hexdigest(),
                     'component': prefix, 'role': role, 'references': references})
    return {'schema': 1, 'scope': 'Source census and explicit links only; file presence is not semantic or validation coverage.',
            'roots': list(SOURCE_ROOTS), 'suffixes': sorted(SUFFIXES), 'files': rows}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--register', type=Path, default=ROOT/'research/runtime/coverage/register.json')
    parser.add_argument('--output', type=Path, required=True, help='New source-index snapshot; refuses overwrite')
    args = parser.parse_args()
    result = index(ROOT, json.loads(args.register.read_text()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as out:
        json.dump(result, out, indent=2)
        out.write('\n')
    print(f'{len(result["files"])} source/build files; {sum(bool(r["references"]) for r in result["files"])} explicitly linked')


if __name__ == '__main__':
    main()
