#!/usr/bin/env python3
"""Index current sources against explicitly registered implementation/test links."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = ('assets', 'audio', 'renderer', 'reconstruction', 'runtime', 'compat', 'apps', 'game', 'protocols', 'tools', 'tests')
SUFFIXES = {'.cpp', '.hpp', '.c', '.h', '.inc', '.S', '.py', '.java', '.sh', '.json', '.html', '.css', '.js'}
BUILD_NAMES = {'CMakeLists.txt', 'Makefile', 'GNUmakefile', 'meson.build', 'meson_options.txt',
               'pyproject.toml', 'requirements.txt', 'CMakePresets.json', 'CMakeUserPresets.json'}
BUILD_SUFFIXES = {'.cmake', '.mk', '.pro', '.pri'}


def in_scope(path):
    parts = Path(path).parts
    if not parts or '__pycache__' in parts or any(p in {'original', 'working', '.git'} for p in parts):
        return False
    return (parts[0] in SOURCE_ROOTS and (Path(path).suffix in SUFFIXES or Path(path).name in BUILD_NAMES
                                        or Path(path).suffix in BUILD_SUFFIXES)
            or len(parts) == 1 and (Path(path).name in BUILD_NAMES or Path(path).suffix in BUILD_SUFFIXES))


def source_paths(root):
    candidates = [p for name in SOURCE_ROOTS for p in (root/name).rglob('*')]
    candidates.extend(root.iterdir())
    return sorted({p.relative_to(root).as_posix() for p in candidates
                   if p.is_file() and in_scope(p.relative_to(root))})


def index(root, register):
    rows = []
    for path in source_paths(root):
        references = [{'behavior': b['id'], 'role': role} for b in register['behaviors']
                      for role in ('implementation', 'tests', 'validation_dependencies') if path in b.get(role, [])]
        prefix = '/'.join(Path(path).parts[:2]) if path.startswith(('apps/', 'runtime/', 'reconstruction/', 'compat/')) else Path(path).parts[0]
        role = 'build' if Path(path).name in BUILD_NAMES or Path(path).suffix in BUILD_SUFFIXES else ('test' if path.startswith('tests/') else
               'tool' if path.startswith('tools/') else 'protocol' if path.startswith('protocols/') else 'code')
        rows.append({'path': path, 'sha256': hashlib.sha256((root/path).read_bytes()).hexdigest(),
                     'component': prefix, 'role': role, 'references': references})
    return {'schema': 1, 'scope': 'Source census and explicit links only; file presence is not semantic or validation coverage.',
            'roots': list(SOURCE_ROOTS), 'suffixes': sorted(SUFFIXES),
            'build_names': sorted(BUILD_NAMES), 'build_suffixes': sorted(BUILD_SUFFIXES), 'files': rows}


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
