#!/usr/bin/env python3
"""Independent TAG field comparison and installed SPR frame-name correlation."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    assert len(data) % 12 == 0
    entries = [dict(name=name.split(b'\0', 1)[0].decode('latin1'),
                    name_bytes_hex=name.hex(), occurrence=index)
               for name, index in struct.iter_unpack('<8sI', data)]
    return dict(source_bytes=len(data), record_count=len(entries), entries=entries)


def sprite_names(data):
    magic, size, version, count, palettes, flags = struct.unpack_from('<6I', data)
    assert magic == 0x00525053 and size == len(data) and version == 4 and palettes <= 4
    table = 24 + palettes * 768
    base = table + count * 4
    assert base <= len(data)
    names = []
    for i in range(count):
        start = base + struct.unpack_from('<I', data, table + i * 4)[0]
        assert start + 40 <= len(data)
        extent = struct.unpack_from('<I', data, start)[0]
        assert extent >= 40 and start + extent <= len(data)
        names.append(data[start + 20:start + 28])
    return names


def fixture(count):
    pairs = [(b'UA000S1\0', 0), (b'UC000S1\0', 0), (b'A\0\xff' + bytes(5), 0xffffffff),
             (b'\xe9' * 8, 0x80000000), (bytes(8), 19), (b'UA000S1\0', 71)]
    return b''.join(struct.pack('<8sI', *pairs[n % len(pairs)]) for n in range(count))


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=sha(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for count in (0, 1, 2, 6, 15):
            (root / 'Odd.TAG').write_bytes(fixture(count))
            compare(args.inspector, root, 'Odd.TAG', 'odd.tag')
        bad = root / 'bad.tag'
        for data in (b'bad', fixture(2)[:-1], fixture(2) + b'\0'):
            bad.write_bytes(data)
            assert subprocess.run([args.inspector, str(root), 'bad.tag'], capture_output=True).returncode == 2
        for path in ('../bad.tag', 'absent.tag'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if not path.is_file() or path.suffix.lower() != '.tag':
                continue
            result = compare(args.inspector, root, path.relative_to(root).as_posix())
            sprite = path.with_suffix('.spr')
            if not sprite.exists():
                sprite = path.parent / 'Terrain.spr'
            sprite_data = sprite.read_bytes()
            names = sprite_names(sprite_data)
            assert [e['name_bytes_hex'] for e in result['entries']] == [n.hex() for n in names], path
            counts = Counter()
            for entry in result['entries']:
                assert entry['occurrence'] == counts[entry['name_bytes_hex']], path
                counts[entry['name_bytes_hex']] += 1
            result.update(sprite_path=sprite.relative_to(root).as_posix(), sprite_sha256=sha(sprite_data))
            records.append(result)
        assert records, 'No installed TAG files found'
        for record in records:
            assert sha((root / record['path']).read_bytes()) == record['source_sha256']
            assert sha((root / record['sprite_path']).read_bytes()) == record['sprite_sha256']
    canonical = json.dumps(records, sort_keys=True, separators=(',', ':')).encode()
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  entries=sum(r['record_count'] for r in records), comparison_sha256=sha(canonical), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"TAG comparisons passed: 5 synthetic fixtures, {len(records)} installed files")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('inspector')
    parser.add_argument('--installation', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    manifest = Path(__file__).resolve().parents[1] / 'tools/original-manifest.sh'
    if args.installation:
        subprocess.run([str(manifest), 'verify'], check=True)
    try:
        run(args)
    finally:
        if args.installation:
            subprocess.run([str(manifest), 'verify'], check=True)


if __name__ == '__main__':
    main()
