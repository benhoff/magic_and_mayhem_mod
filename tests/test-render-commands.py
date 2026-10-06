#!/usr/bin/env python3
"""Independent native-pixel oracle for bounded ordered OpenGL sessions."""
import hashlib
import json
import random
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = b'MNMCMD01'+struct.pack('<II', 1, 16)


def pack(*values):
    return struct.pack('<'+'I'*len(values), *values)


def stream(records):
    return HEADER+b''.join(pack(op, i+1, len(payload))+payload for i, (op, payload) in enumerate(records))


def main():
    executable = Path(sys.argv[1]).resolve()
    parent = ROOT/'working/tests/render-commands';parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    results = []

    def run(name, records, expected=None, rgba=None, bad=False, raw=None):
        source = root/(name+'.bin');output = root/(name+'-native.bin');preview = root/(name+'.png')
        source.write_bytes(stream(records) if raw is None else raw)
        result = subprocess.run([str(executable), str(source), '--output', str(output), '--preview', str(preview)],
                                capture_output=True, text=True, timeout=20)
        report = json.loads(result.stdout)
        assert result.returncode == (2 if bad else 0), (name, report, result.stderr)
        if bad:
            assert not output.exists() and not preview.exists(), name
        else:
            assert output.read_bytes() == expected, name
            assert report['output_sha256'] == hashlib.sha256(expected).hexdigest()
            assert report['presentation_rgba_sha256'] == hashlib.sha256(rgba).hexdigest()
            assert report['surface_stats']['surfaces'] == 0
            results.append(report)
        return report

    rng = random.Random(497)
    for bits in (8, 16, 24, 32):
        masks = {8:(0,0,0), 16:(0xf800,0x7e0,0x1f), 24:(0xff0000,0xff00,0xff), 32:(0xff0000,0xff00,0xff)}[bits]
        def native(values):
            return b''.join(v.to_bytes(bits//8, 'little') for v in values)
        source = [rng.randrange(1<<bits) for _ in range(12)]
        target = [rng.randrange(1<<bits) for _ in range(30)]
        records = [(1,pack(11,4,3,bits,*masks)+native(source)), (1,pack(22,6,5,bits,*masks)+native(target))]
        palette = [[0,0,0] for _ in range(256)]
        if bits == 8:
            palette = [[i,255-i,(i*7)%256] for i in range(256)]
            records.append((4,pack(22,0,256)+bytes(sum(palette,[]))))
        for step in range(24):
            # Explicit CPU write to a source patch, then an opaque or keyed copy.
            x,y = rng.randrange(3),rng.randrange(3)
            patch = [rng.randrange(1<<bits),rng.randrange(1<<bits)]
            source[y*4+x:y*4+x+2] = patch
            records.append((2,pack(11,x,y,2,1)+native(patch)))
            dx,dy = rng.randrange(3),rng.randrange(3)
            keyed = step%2;key = source[rng.randrange(12)] if keyed else 0
            records.append((3,pack(11,22,0,0,4,3,dx,dy,keyed,key)))
            for sy in range(3):
                for sx in range(4):
                    value = source[sy*4+sx]
                    if not keyed or value != key:target[(dy+sy)*6+dx+sx] = value
            records.append((5,pack(22)+native(target)))
            if bits == 8 and step%4 == 0:
                first = rng.randrange(254);colors = [[rng.randrange(256) for _ in range(3)] for _ in range(2)]
                palette[first:first+2] = colors
                records.append((4,pack(22,first,2)+bytes(sum(colors,[]))))
        records.append((6,pack(22)))
        records.extend([(7,pack(11)), (7,pack(22))])
        # Creating another ID after destruction exercises the replay handle mapping.
        records.extend([(1,pack(33,6,5,bits,*masks)+native(target))])
        if bits == 8:records.append((4,pack(33,0,256)+bytes(sum(palette,[]))))
        records.extend([(5,pack(33)+native(target)), (6,pack(33)), (7,pack(33)), (8,b'')])
        rgba = bytearray()
        for value in target:
            if bits == 8:rgb = palette[value]
            else:
                rgb = []
                for mask in masks:
                    low = mask & -mask;rgb.append(((value & mask)//low)*255//(mask//low))
            if bits==16 and masks==(0xf800,0x7e0,0x1f):
                red,green,blue=(value>>11)&31,(value>>5)&63,value&31;rgb=[(red<<3)|(red>>2),(green<<2)|(green>>4),(blue<<3)|(blue>>2)]
            rgba.extend([*rgb,255])
        records.insert(-2,(10,pack(33)+bytes(rgba)))
        report = run(f'sequence-{bits}', records, native(target), bytes(rgba))
        assert report['color_checks'] == 1
        assert report['checks'] == 25 and report['presentations'] == 2
        assert report['surface_stats']['uploads'] == 27 and report['surface_stats']['copies'] == 24

    for bits in (8,16,24,32):
        masks={8:(0,0,0),16:(0xf800,0x7e0,0x1f),24:(0xff0000,0xff00,0xff),32:(0xff0000,0xff00,0xff)}[bits]
        def native(values):return b''.join(v.to_bytes(bits//8,'little') for v in values)
        a=[1,2];b=[3,4]
        records=[(1,pack(1,2,1,bits,*masks)+native(a)),(1,pack(2,2,1,bits,*masks)+native(b))]
        palettes={1:[[i,0,0] for i in range(256)],2:[[0,i,255-i] for i in range(256)]}
        if bits==8:
            for sid in (1,2):records.append((4,pack(sid,0,256)+bytes(sum(palettes[sid],[]))))
        for step in range(5):
            records.append((11,pack(1,2)));a,b=b,a
            for sid,values in ((1,a),(2,b)):
                records.append((5,pack(sid)+native(values)))
                rgba=bytearray()
                for value in values:
                    rgb=palettes[sid][value] if bits==8 else [((value&mask)//(mask&-mask))*255//(mask//(mask&-mask)) for mask in masks]
                    if bits==16 and masks==(0xf800,0x7e0,0x1f):
                        red,green,blue=(value>>11)&31,(value>>5)&63,value&31;rgb=[(red<<3)|(red>>2),(green<<2)|(green>>4),(blue<<3)|(blue>>2)]
                    rgba.extend([*rgb,255])
                records.append((10,pack(sid)+bytes(rgba)))
                records.append((6,pack(sid)))
        records.extend([(7,pack(1)),(7,pack(2)),(8,b'')])
        report=run(f'swap-{bits}',records,native(b),bytes(rgba))
        assert report['checks']==10 and report['color_checks']==10 and report['presentations']==10
        assert report['surface_stats']['uploads']==2 and report['surface_stats']['copies']==0

    valid = [(1,pack(1,1,1,8,0,0,0)+b'\x03'), (5,pack(1)+b'\x03'), (6,pack(1)), (7,pack(1)), (8,b'')]
    bad_cases = [[],valid[:-1],valid+[(8,b'')],[(9,b'')],valid[:1]+[(1,valid[0][1])]+valid[1:],
        valid[:3]+[(7,pack(2))]+valid[3:],valid[:4]+[(1,valid[0][1])]+valid[4:],
        valid[:1]+[(2,pack(1,1,0,1,1)+b'\x02')]+valid[1:],
        valid[:1]+[(3,pack(1,1,0,0,1,1,0,0,0,0))]+valid[1:],
        valid[:1]+[(4,pack(1,255,2)+bytes(6))]+valid[1:],
        [(1,pack(1,1,1,8,1,0,0)+b'\x03')]+valid[1:],
        valid[:1]+[(5,pack(1)+b'\x04')]+valid[2:],
        valid[:1]+[(5,pack(1))]+valid[2:],valid[:4]+[(8,b'x')],
        [(1,pack(0,1,1,8,0,0,0)+b'\x03')]+valid[1:],
        [(1,pack(1,2049,1,8,0,0,0)+bytes(2049))]+valid[1:],
        valid[:2]+valid[3:],valid[:3]+[(8,b'')],
        valid[:4]+[(2,pack(1,0,0,1,1)+b'\x02')]+valid[4:],
        [(1,pack(i,1,1,8,0,0,0)+b'\x03') for i in range(1,66)]+[(8,b'')],
        valid[:1]+[(5,pack(1)+b'\x03')]*4096+valid[2:],
        valid[:1]+[(10,pack(1)+b'\0\0')]+valid[1:],
        valid[:1]+[(10,pack(2)+bytes(4))]+valid[1:],
        valid[:1]+[(10,pack(1)+bytes(4))]+valid[1:]]
    pair=[(1,pack(2,1,1,8,0,0,0)+b'\x07')]
    for payload in (pack(1,1),pack(1,3),pack(1),pack(1,2,0)):
        bad_cases.append(valid[:1]+pair+[(11,payload)]+valid[1:])
    bad_cases.append(valid[:1]+[(1,pack(2,2,1,8,0,0,0)+b'\x07\x08'),(11,pack(1,2))]+valid[1:])
    bad_cases.append(valid[:1]+[(1,pack(2,1,1,16,0xf800,0x7e0,0x1f)+bytes(2)),(11,pack(1,2))]+valid[1:])
    bad_cases.append(valid[:1]+pair+[(7,pack(2)),(11,pack(1,2))]+valid[1:])
    for i, records in enumerate(bad_cases):run(f'reject-{i}', records, bad=True)
    for i, raw in enumerate((stream(valid)[:-1],stream(valid)+b'x',HEADER[:15],
                             HEADER+pack(1,2,len(valid[0][1]))+valid[0][1])):
        run(f'reject-raw-{i}', [], bad=True, raw=raw)
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_ordered_commands','sessions':results,
                                              'rejected':len(bad_cases)+4}, indent=2)+'\n')
    print(f'Ordered surface commands passed: {root}')


if __name__ == '__main__':main()
