#!/usr/bin/env python3
"""Independent SFT metric/frame/RLE decoding and owned inspector comparison."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def reference(data):
    magic, size, version, count, rows, ascent, descent, opaque, palette, metrics = struct.unpack_from('<5I2i3I', data)
    assert magic == 0x00544653 and size == len(data) and version == 3
    at = 40 + (768 if palette else 0)
    pairs = [list(p) for p in struct.iter_unpack('<2i', data[at:at+rows*metrics*8])]
    table = at + rows*metrics*8
    base = table + count*4
    glyphs = []
    for i in range(count):
        start = base + struct.unpack_from('<I', data, table+i*4)[0]
        length, w, h, x, y, name, pal, a, b = struct.unpack_from('<3I2i8s3I', data, start)
        frame = data[start:start+length]
        assert len(frame) == length
        aux = [frame[o:min([v for v in (a,b,length) if v>o], default=length)].hex() if o else '' for o in (a,b)]
        stride = 1 if palette else 2
        pixels = bytearray(w*h*stride)
        mask = bytearray(w*h)
        if h:
            ranges = [struct.unpack_from('<2I', frame, 40+j*8) for j in range(h)]
            for row, (control, pixel) in enumerate(ranges):
                stop = ranges[row+1][0] if row+1<h else ranges[0][1]
                pixel_end = ranges[row+1][1] if row+1<h else (a or b or length)
                column, colour = 0, False
                for run in frame[control:stop]:
                    assert column+run <= w
                    if colour:
                        assert pixel+run*stride <= pixel_end
                        dst = (row*w+column)*stride
                        pixels[dst:dst+run*stride] = frame[pixel:pixel+run*stride]
                        mask[row*w+column:row*w+column+run] = b'\1'*run
                        pixel += run*stride
                    column += run
                    colour = not colour
                assert column == w
        glyphs.append(dict(byte=i+33 if i<223 else None, width=w, height=h, origin_x=x, origin_y=y,
                           name_hex=name.hex(), source_offset=start, encoded_size=length,
                           palette_index=pal if palette else None, auxiliary_offsets=[a,b],
                           auxiliary_hex=aux, pixels_hex=pixels.hex(), mask_hex=mask.hex()))
    return dict(source_bytes=len(data), version=version, row_count=rows, ascent=ascent, descent=descent,
                header_word_28=opaque, palette_flag=palette, metric_glyph_count=metrics,
                row_metrics=pairs, palettes_hex=[data[40:808].hex()] if palette else [],
                glyph_count=count, glyphs=glyphs)


def fixture(palette=1, alias=False, empty=False):
    count = 0 if empty else (2 if alias else 1)
    rgb = bytes(v for i in range(256) for v in (i,255-i,i//2)) if palette else b''
    profile = b'' if empty else struct.pack('<4i', -1, 0x7fffffff, -8, 12)
    pixels = bytes([0,127,255]) if palette else struct.pack('<3H', 0,0xf800,0xffff)
    # Transparent first pixel followed by three opaque pixels; two distinct auxiliary planes.
    size = 50+len(pixels)+4
    frame = struct.pack('<3I2i8s3I2I', size,4,1,-2,2,b'!\0\xffabcde',0 if palette else 0xffffffff,
                        50+len(pixels),52+len(pixels),48,50)+b'\1\3'+pixels+b'abcd'
    body = rgb+profile+bytes(count*4)+(frame if count else b'')
    return struct.pack('<5I2i3I',0x00544653,40+len(body),3,count,0 if empty else 2,-1,-3,
                       0x12345678,palette,0 if empty else 1)+body


def compare(inspector, root, path, atlas=None):
    data = (root/path).read_bytes()
    command = [inspector,str(root),path.lower()]
    if atlas:
        command.append(str(atlas))
    actual = json.loads(subprocess.check_output(command, text=True))
    expected = reference(data)
    assert actual == expected, path
    return dict(path=path, source_sha256=hashlib.sha256(data).hexdigest(), **actual)


def run(args):
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        for palette, alias, empty in [(1,False,False),(7,True,False),(0,False,False),(0,False,True)]:
            (root/'Font.SFT').write_bytes(fixture(palette,alias,empty))
            atlas = root/'atlas.png'
            compare(args.inspector,root,'Font.SFT',atlas)
            assert atlas.read_bytes().startswith(b'\x89PNG\r\n\x1a\n')
            try:
                from PIL import Image
            except ImportError:
                continue
            with Image.open(atlas) as image:
                assert image.getpixel((1,1))[3] == 0
                if not empty:
                    assert image.getpixel((2,1))[3] == 255  # opaque zero is not a colour key
                    assert image.getpixel((3,1)) == ((127,128,63,255) if palette else (255,0,0,255))
        for data in (b'bad',fixture()[:-1],fixture()+b'\0'):
            (root/'bad.sft').write_bytes(data)
            assert subprocess.run([args.inspector,str(root),'bad.sft'],capture_output=True).returncode == 2
        for path in ('../Font.SFT','missing.sft'):
            assert subprocess.run([args.inspector,str(root),path],capture_output=True).returncode == 2
    records = []
    if args.installation:
        root = args.installation.resolve()
        if args.atlas_dir:
            args.atlas_dir.mkdir(parents=True,exist_ok=True)
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower()=='.sft':
                atlas = args.atlas_dir/(path.stem+'.png') if args.atlas_dir else None
                records.append(compare(args.inspector,root,path.relative_to(root).as_posix(),atlas))
        assert records, 'No installed SFT inputs'
        for record in records:
            assert hashlib.sha256((root/record['path']).read_bytes()).hexdigest()==record['source_sha256']
    canonical = json.dumps(records,sort_keys=True,separators=(',',':')).encode()
    report = dict(files=len(records),glyphs=sum(r['glyph_count'] for r in records),
                  metric_rows=sum(len(r['row_metrics']) for r in records),
                  source_bytes=sum(r['source_bytes'] for r in records),
                  comparison_sha256=hashlib.sha256(canonical).hexdigest(),records=records)
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f"SFT comparisons passed: 4 synthetic fixtures, {len(records)} installed files, {report['glyphs']} glyphs, {report['metric_rows']} metric rows")


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('inspector')
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--report',type=Path)
    parser.add_argument('--atlas-dir',type=Path)
    args=parser.parse_args()
    manifest=Path(__file__).resolve().parents[1]/'tools/original-manifest.sh'
    if args.installation:
        subprocess.run([str(manifest),'verify'],check=True)
    try:
        run(args)
    finally:
        if args.installation:
            subprocess.run([str(manifest),'verify'],check=True)


if __name__=='__main__':
    main()
