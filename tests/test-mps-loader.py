#!/usr/bin/env python3
"""Compare all MPS words/metadata with independent struct-based decoding."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

KINDS = ['undefined', 'friendly_wizard', 'enemy_wizard', 'multiplayer_wizard', 'creature', 'artifact']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    magic, size, version, count = struct.unpack_from('<4I', data)
    assert magic == 0x0053504d and version == 1 and len(data) == 16 + 40 * count
    placements = []
    for words in struct.iter_unpack('<10i', data[16:]):
        kind = words[3]
        placements.append(dict(position=list(words[:3]), kind=kind,
                               kind_name=KINDS[kind] if 0 <= kind < 6 else 'unknown', parameters=list(words[4:])))
    return dict(source_bytes=len(data), header_size_word=size, version=version,
                record_count=count, placements=placements)


def fixture(count, size=None):
    records = [struct.pack('<10i', n, -n, 3, n % 8 - 1, 0, -1, -2147483648, 2147483647, 19, 23) for n in range(count)]
    return struct.pack('<4I', 0x0053504d, 16 + 16 * count if size is None else size, 1, count) + b''.join(records)


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=sha(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for count, size in [(0, None), (1, None), (9, None), (12, 0xffffffff)]:
            (root / 'Odd.MPS').write_bytes(fixture(count, size))
            compare(args.inspector, root, 'Odd.MPS', 'odd.mps')
        bad = root / 'bad.mps'
        for data in (b'bad', fixture(2)[:-1], fixture(2) + b'\0'):
            bad.write_bytes(data)
            assert subprocess.run([args.inspector, str(root), 'bad.mps'], capture_output=True).returncode == 2
        for path in ('../bad.mps', 'absent.mps'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() == '.mps':
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed MPS files found'
    kinds = Counter(p['kind'] for f in records for p in f['placements'])
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  placements=sum(r['record_count'] for r in records), kind_counts=dict(kinds), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"MPS comparisons passed: 4 synthetic fixtures, {len(records)} installed files")


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
