#!/usr/bin/env python3
"""Compare native PCX metadata/pixels/palette with a separate Python decoder."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def digest(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    xmin, ymin, xmax, ymax = struct.unpack_from('<4H', data, 4)
    width, height = xmax - xmin + 1, ymax - ymin + 1
    stride = struct.unpack_from('<H', data, 66)[0]
    assert data[:4] == bytes([10, 5, 1, 8]) and data[65] == 1
    assert data[-769] == 12
    # Expand the entire encoded stream first, then independently strip padding.
    expanded = bytearray()
    stream = iter(data[128:-769])
    for token in stream:
        if token >= 192:
            count = token & 63
            assert count
            expanded.extend(bytes([next(stream)]) * count)
        else:
            expanded.append(token)
    assert len(expanded) == stride * height
    indices = b''.join(expanded[row * stride:row * stride + width] for row in range(height))
    return dict(source_bytes=len(data), width=width, height=height,
                origin_x=xmin, origin_y=ymin,
                horizontal_dpi=struct.unpack_from('<H', data, 12)[0],
                vertical_dpi=struct.unpack_from('<H', data, 14)[0],
                bytes_per_line=stride, palette_info=struct.unpack_from('<H', data, 68)[0],
                palette_sha256=digest(data[-768:]), indices_sha256=digest(indices))


def fixture(width, height):
    stride = (width + 1) & ~1
    header = bytearray(128)
    header[:4] = bytes([10, 5, 1, 8])
    struct.pack_into('<4H', header, 4, 7, 9, width + 6, height + 8)
    header[65] = 1
    struct.pack_into('<H', header, 66, stride)
    encoded = bytearray()
    for y in range(height):
        row = bytes((x * 37 + y * 19) % 256 for x in range(width)) + bytes([255]) * (stride - width)
        for value in row:
            encoded.extend([193, value] if value >= 192 else [value])
    return bytes(header + encoded + bytes([12]) + bytes(i % 256 for i in range(768)))


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=digest(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for w, h in [(1, 1), (3, 2), (64, 3), (127, 5)]:
            (root / 'Odd.PCX').write_bytes(fixture(w, h))
            compare(args.inspector, root, 'Odd.PCX', 'odd.pcx')  # Case-insensitive asset access.
        (root / 'bad.pcx').write_bytes(b'bad')
        assert subprocess.run([args.inspector, str(root), 'bad.pcx'], capture_output=True).returncode == 2
        assert subprocess.run([args.inspector, str(root), '../bad.pcx'], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() == '.pcx':
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed PCX files found'
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  pixels=sum(r['width'] * r['height'] for r in records), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"PCX comparisons passed: 4 synthetic fixtures, {len(records)} installed files")


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
