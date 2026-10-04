#!/usr/bin/env python3
"""Read-only TAG inventory, companion SPR correlation and limited literal audit."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--installation', type=Path, default=REPO / 'working/game-clean')
    args = parser.parse_args()
    parent = REPO / 'working/decompiled'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='tag-support-', dir=parent))
    print(f'TAG evidence: {out}', flush=True)

    def verify(phase):
        run = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True, timeout=120)
        (out / f'original-{phase}.log').write_text(run.stdout + run.stderr)
        print(run.stdout, end='', flush=True)
        run.check_returncode()

    verify('before')
    try:
        exe = REPO / 'working/game-nocd/Chaos.exe'
        binary = exe.read_bytes()
        if sha(binary) != HASH:
            raise ValueError('Unsupported executable hash')
        root = args.installation.resolve()
        files = []
        sprites = {}
        all_names = Counter()
        for path in sorted(root.rglob('*')):
            if not path.is_file() or path.suffix.lower() != '.tag':
                continue
            data = path.read_bytes()
            assert len(data) % 12 == 0
            records = list(struct.iter_unpack('<8sI', data))
            sprite = path.with_suffix('.spr')
            if not sprite.exists():
                sprite = path.parent / 'Terrain.spr'
            relative = sprite.relative_to(root).as_posix()
            spr = sprite.read_bytes()
            magic, size, version, count, palettes, flags = struct.unpack_from('<6I', spr)
            assert magic == 0x00525053 and size == len(spr) and version == 4 and palettes <= 4
            table = 24 + palettes * 768
            base = table + 4 * count
            assert base <= len(spr) and count == len(records)
            occurrence = Counter()
            for i, (name, index) in enumerate(records):
                start = base + struct.unpack_from('<I', spr, table + 4 * i)[0]
                assert start + 40 <= len(spr)
                extent = struct.unpack_from('<I', spr, start)[0]
                assert extent >= 40 and start + extent <= len(spr)
                assert spr[start + 20:start + 28] == name
                assert index == occurrence[name]
                occurrence[name] += 1
                all_names[name] += 1
            sprites[relative] = sha(spr)
            files.append(dict(path=path.relative_to(root).as_posix(), source_sha256=sha(data),
                              source_bytes=len(data), record_count=len(records),
                              first_name_hex=data[:8].hex(), unique_names=len(occurrence),
                              max_occurrence=max((r[1] for r in records), default=None),
                              sprite_path=relative, sprite_sha256=sha(spr),
                              sprite_names_match=True, occurrence_sequence_match=True))
        if not files:
            raise ValueError('No installed TAG inputs')
        if sha(exe.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        for item in files:
            if sha((root / item['path']).read_bytes()) != item['source_sha256']:
                raise ValueError('TAG input changed')
        for path, digest in sprites.items():
            if sha((root / path).read_bytes()) != digest:
                raise ValueError('Companion SPR changed')
        report = dict(executable_sha256=HASH, script_sha256=sha(Path(__file__).read_bytes()),
                      scope='Installed TAG/SPR structural correlation; limited executable literal scan, no identified original TAG reader or live observation',
                      literal_counts={needle.decode(): binary.count(needle) for needle in (b'.tag', b'.TAG', b'UA000', b'UC000')},
                      input_unchanged=True,
                      summary=dict(files=len(files), source_bytes=sum(f['source_bytes'] for f in files),
                                   records=sum(f['record_count'] for f in files), companion_sprites=len(sprites),
                                   unique_names=len(all_names), name_bytes_nonzero_after_first_nul=sum(
                                       count for name, count in all_names.items() if b'\0' in name and any(name.split(b'\0', 1)[1]))),
                      files=files)
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary']), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
