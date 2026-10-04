#!/usr/bin/env python3
"""Compare all EVT words/metadata with independent struct-based decoding."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

def sha(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    magic, size, version, count = struct.unpack_from('<4I', data)
    assert magic == 0x00545645 and version == 1 and len(data) == 16 + 72 * count
    areas = []
    for words in struct.iter_unpack('<6i48s', data[16:]):
        areas.append(dict(first=list(words[:3]), second=list(words[3:6]),
                          name=words[6].split(b'\0', 1)[0].decode('latin1'), name_bytes_hex=words[6].hex()))
    return dict(source_bytes=len(data), header_size_word=size, version=version,
                record_count=count, areas=areas)


def fixture(count, size=None):
    names = [b'A\0\xff' + bytes(45), b'\xe9' * 48, bytes(48), b'duplicate'.ljust(48, b'\0')]
    records = [struct.pack('<6i48s', 23, -1, -2147483648, -n, 2147483647, 0, names[n % 4]) for n in range(count)]
    return struct.pack('<4I', 0x00545645, 16 + 16 * count if size is None else size, 1, count) + b''.join(records)


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=sha(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for count, size in [(0, None), (1, None), (9, None), (12, 0xffffffff)]:
            (root / 'Odd.EVT').write_bytes(fixture(count, size))
            compare(args.inspector, root, 'Odd.EVT', 'odd.evt')
        bad = root / 'bad.evt'
        for data in (b'bad', fixture(2)[:-1], fixture(2) + b'\0'):
            bad.write_bytes(data)
            assert subprocess.run([args.inspector, str(root), 'bad.evt'], capture_output=True).returncode == 2
        for path in ('../bad.evt', 'absent.evt'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() == '.evt':
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed EVT files found'
    canonical = json.dumps(records, sort_keys=True, separators=(',', ':')).encode()
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  areas=sum(r['record_count'] for r in records), comparison_sha256=sha(canonical), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"EVT comparisons passed: 4 synthetic fixtures, {len(records)} installed files")


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
