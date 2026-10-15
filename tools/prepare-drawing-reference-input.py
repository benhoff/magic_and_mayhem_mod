#!/usr/bin/env python3
"""Pin installed assets with the supplied unchanged No-CD executable for capture."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from drawing_corpus import ROOT, BUILD


def fingerprints(root):
    result = {}
    for path in sorted(root.rglob('*')):
        if path.is_symlink():
            raise ValueError('Installed source symlink needs explicit review: ' + str(path))
        if path.is_file():
            result[path.relative_to(root).as_posix()] = hashlib.file_digest(path.open('rb'), 'sha256').hexdigest()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-game', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    source = args.source_game.resolve()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / 'working') or output.exists() or args.output.is_symlink():
        parser.error('Reference input must be a new contained working path')
    verify = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(verify, check=True)
    try:
        original = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        data = original.read_bytes()
        if hashlib.sha256(data).hexdigest() != BUILD:
            raise ValueError('Supplied original No-CD executable differs from the supported build')
        before = fingerprints(source)
        output.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(['cp', '-a', '--reflink=auto', str(source), str(output)], check=True)
        executable = output / 'Chaos.exe'
        if not executable.resolve().is_relative_to(output):
            raise ValueError('Executable destination escapes new reference input')
        executable.write_bytes(data)
        staged = fingerprints(output)
        if staged != dict(before, **{'Chaos.exe': BUILD}) or fingerprints(source) != before:
            raise ValueError('Reference preparation changed an asset or its installed source')
        manifest = dict(schema=1, source_game=str(source), source_files=before,
                        staged_files=staged, executable_sha256=BUILD,
                        original_executable=str(original), original_manifest_verified_before_after=True,
                        scope='Installed source assets/configuration retained byte-for-byte, with supplied unchanged pinned No-CD executable. No game execution, comparison, input normalization or gameplay balance change.')
        subprocess.run(verify, check=True)
        with (output.parent / (output.name + '-manifest.json')).open('x') as out:
            json.dump(manifest, out, indent=2)
            out.write('\n')
        print(output)
    finally:
        subprocess.run(verify, check=True)


if __name__ == '__main__':
    main()
