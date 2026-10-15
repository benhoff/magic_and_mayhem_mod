#!/usr/bin/env python3
"""Create the launcher's missing verified baseline from an already pinned input."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_corpus import ROOT, BUILD, sha


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-game', type=Path, required=True)
    args = parser.parse_args()
    source = args.reference_game.resolve()
    if not source.is_relative_to(ROOT / 'working'):
        parser.error('Reference input must be contained in this worktree')
    manifest_path = source.parent / (source.name + '-manifest.json')
    manifest = json.loads(manifest_path.read_text())
    target = ROOT / 'working/game-nocd'
    output = ROOT / 'working/manifests/game-nocd.tsv'
    if target.exists() or target.is_symlink() or output.exists():
        parser.error('Refusing to replace an existing launcher baseline')
    if manifest['executable_sha256'] != BUILD or manifest['original_manifest_verified_before_after'] is not True:
        raise ValueError('Reference input lacks supported original provenance')
    files = []
    for name, digest in sorted(manifest['staged_files'].items()):
        path = source / name
        if (Path(name).is_absolute() or '..' in Path(name).parts or path.is_symlink() or
                not path.resolve().is_relative_to(source) or sha(path.read_bytes()) != digest):
            raise ValueError('Pinned reference input changed or escapes source: ' + name)
        files.append((name, path.stat().st_size, digest))
    subprocess.run(['cp', '-a', '--reflink=auto', str(source), str(target)], check=True)
    for name, size, digest in files:
        path = target / name
        if path.stat().st_size != size or sha(path.read_bytes()) != digest:
            raise ValueError('Launcher baseline copy differs from pinned reference')
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open('x') as out:
        out.write('sha256\tsize\tpath\n')
        for name, size, digest in files:
            out.write(f'{digest}\t{size}\t{name}\n')
    print(json.dumps(dict(baseline=str(target), manifest=str(output), files=len(files),
                         source_manifest_sha256=sha(manifest_path.read_bytes()), game_launched=False)))


if __name__ == '__main__':
    main()
