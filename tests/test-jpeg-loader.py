#!/usr/bin/env python3
"""Compare native JPEG RGB output with Pillow's separately invoked decoder."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, __version__ as pillow_version


def digest(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    with Image.open(io.BytesIO(data)) as source:
        assert source.format == 'JPEG'
        rgb = source.convert('RGB')  # No EXIF transpose or ICC transform.
        return dict(source_bytes=len(data), width=rgb.width, height=rgb.height,
                    rgb_sha256=digest(rgb.tobytes()))


def fixture(mode, progressive=False, orientation=1):
    image = Image.new(mode, (7, 5))
    if mode == 'RGB':
        image.putdata([(x * 31 % 256, y * 47 % 256, (x + y) * 53 % 256) for y in range(5) for x in range(7)])
    else:
        image.putdata([(x * 31 + y * 47) % 256 for y in range(5) for x in range(7)])
    exif = Image.Exif()
    exif[274] = orientation
    encoded = io.BytesIO()
    image.save(encoded, format='JPEG', quality=95, progressive=progressive, exif=exif)
    return encoded.getvalue()


def compare(inspector, root, path, request=None):
    data = (root / path).read_bytes()
    actual = json.loads(subprocess.check_output([inspector, str(root), request or path], text=True))
    expected = reference(data)
    assert actual == expected, f'{path}: native={actual}, Pillow={expected}'
    return dict(path=path, source_sha256=digest(data), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for mode in ('RGB', 'L'):
            for progressive in (False, True):
                for orientation in (1, 6):
                    (root / 'Odd.JPG').write_bytes(fixture(mode, progressive, orientation))
                    compare(args.inspector, root, 'Odd.JPG', 'odd.jpg')
        bad = root / 'bad.jpg'
        for data in (b'bad', b'\xff\xd8\xff\xd9', fixture('RGB')[:-2]):
            bad.write_bytes(data)
            assert subprocess.run([args.inspector, str(root), 'bad.jpg'], capture_output=True).returncode == 2
        for path in ('../bad.jpg', 'absent.jpg'):
            assert subprocess.run([args.inspector, str(root), path], capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower() in ('.jpg', '.jpeg'):
                records.append(compare(args.inspector, root, path.relative_to(root).as_posix()))
        assert records, 'No installed JPEG files found'
    report = dict(files=len(records), source_bytes=sum(r['source_bytes'] for r in records),
                  pixels=sum(r['width'] * r['height'] for r in records),
                  pillow_version=pillow_version, records=records)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f"JPEG comparisons passed: 8 synthetic fixtures, {len(records)} installed files")


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
