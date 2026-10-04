#!/usr/bin/env python3
"""Independent struct-based RGB reconstruction versus the native BMP reader."""
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
    size, reserved1, reserved2, offset = struct.unpack_from('<IHHI', data, 2)
    header, width, signed_height, planes, bits, compression, image_bytes, xppm, yppm, used, important = struct.unpack_from('<IiiHHIIiiII', data, 14)
    assert data[:2] == b'BM' and header == 40 and planes == 1 and bits == 24 and compression == 0
    assert reserved1 == reserved2 == 0 and width > 0 and signed_height != 0
    height = abs(signed_height)
    stride = ((width * 3 + 3) // 4) * 4
    assert offset + height * stride <= size <= len(data)
    rows = []
    for y in range(height):
        raw = data[offset + y * stride:offset + y * stride + width * 3]
        rows.append(b''.join(bytes((red, green, blue)) for blue, green, red in struct.iter_unpack('BBB', raw)))
    if signed_height > 0:
        rows.reverse()
    return dict(source_bytes=len(data), width=width, height=height,
                source_top_down=signed_height < 0, pixel_offset=offset, row_stride=stride,
                declared_image_bytes=image_bytes, horizontal_pixels_per_meter=xppm,
                vertical_pixels_per_meter=yppm, colors_used=used, colors_important=important,
                rgb_sha256=digest(b''.join(rows)))


def fixture(width, height, top_down):
    stride = (width * 3 + 3) // 4 * 4
    # Gap/optional optimization color table, plus trailing data outside bfSize.
    offset = 62
    header = bytearray(offset)
    header[:2] = b'BM'
    struct.pack_into('<IHHI', header, 2, offset + stride * height, 0, 0, offset)
    struct.pack_into('<IiiHHIIiiII', header, 14, 40, width, -height if top_down else height,
                     1, 24, 0, 0, -72, 123, 2, 1)
    rows = []
    for y in range(height):
        rgb = [(x * 31 % 256, y * 73 % 256, (x + y) * 47 % 256) for x in range(width)]
        row = b''.join(bytes((b, g, r)) for r, g, b in rgb)
        rows.append(row + bytes([219]) * (stride - len(row)))
    if not top_down:
        rows.reverse()
    return bytes(header) + b''.join(rows) + b'trailing'


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    assert actual == reference(data), path
    return dict(path=path, source_sha256=digest(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for width in (1, 2, 3, 4, 7):
            for top_down in (False, True):
                (root / 'Odd.BMP').write_bytes(fixture(width, 3, top_down))
                compare(args.inspector, root, 'Odd.BMP', 'odd.bmp')
        (root / 'bad.bmp').write_bytes(b'bad')
        for path in ('bad.bmp', '../bad.bmp', 'absent.bmp'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() == '.bmp':
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed BMP files found'
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  pixels=sum(r['width'] * r['height'] for r in records), records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"BMP comparisons passed: 10 synthetic fixtures, {len(records)} installed files")


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
