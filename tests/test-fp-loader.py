#!/usr/bin/env python3
"""Independent FP header and point-array comparison."""
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
    magic, version = struct.unpack_from('<2I', data)
    header_point = list(struct.unpack_from('<2i', data, 8))
    paths, *ranges = struct.unpack_from('<17I', data, 16)
    counts, offsets = ranges[:8], ranges[8:]
    count = sum(counts[:paths])
    assert magic == 0x0050462e and version == 2 and paths <= 8 and len(data) == 116 + count * 8
    assert all(offsets[i] + counts[i] <= count for i in range(paths))
    flags = [list(p) for p in struct.iter_unpack('<2i', data[84:116])]
    points = [list(p) for p in struct.iter_unpack('<2i', data[116:])]
    return dict(source_bytes=len(data), version=version, path_count=paths, header_point=header_point,
                point_counts=counts, point_offsets=offsets, flag_positions=flags,
                point_count=count, points=points)


def fixture(paths, alias=False):
    counts = [i % 3 + 1 for i in range(paths)] + [0xffffffff] * (8-paths)
    offsets = [sum(counts[:i]) for i in range(paths)] + [0xfffffffe] * (8-paths)
    if alias and paths > 1:
        offsets[1] = 0
    count = sum(counts[:paths])
    return (struct.pack('<2I2i17I', 0x0050462e, 2, -2147483648, 2147483647, paths, *counts, *offsets)
            + struct.pack('<8i', -1, 2, 3, 4, 5, 6, 7, 8)
            + b''.join(struct.pack('<2i', n, -n) for n in range(count)))


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=sha(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for paths, alias in [(0, False), (1, False), (3, False), (8, False), (3, True)]:
            (root / 'Odd.FP').write_bytes(fixture(paths, alias))
            compare(args.inspector, root, 'Odd.FP', 'odd.fp')
        bad = root / 'bad.fp'
        for data in (b'bad', fixture(2)[:-1], fixture(2) + b'\0'):
            bad.write_bytes(data)
            assert subprocess.run([args.inspector, str(root), 'bad.fp'], capture_output=True).returncode == 2
        for path in ('../bad.fp', 'absent.fp'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() == '.fp':
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed FP inputs found'
        for record in records:
            assert sha((root / record['path']).read_bytes()) == record['source_sha256']
    canonical = json.dumps(records, sort_keys=True, separators=(',', ':')).encode()
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  points=sum(r['point_count'] for r in records), paths=sum(r['path_count'] for r in records),
                  comparison_sha256=sha(canonical), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"FP comparisons passed: 5 synthetic fixtures, {len(records)} installed files")


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
