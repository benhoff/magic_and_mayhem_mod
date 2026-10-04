#!/usr/bin/env python3
"""Independent indexed CUR plane/hotspot comparison, with optional installed inputs."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

REPO=Path(__file__).resolve().parents[1]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    reserved,kind,count=struct.unpack_from('<3H',data)
    assert (reserved,kind)==(0,2)
    images=[]
    for i in range(count):
        width,height,colors,unused,x,y,length,offset=struct.unpack_from('<4B2H2I',data,6+i*16)
        width=width or 256; height=height or 256
        header,w,h,planes,depth,compression,size,xppm,yppm,used,important=struct.unpack_from('<IiiHHIIiiII',data,offset)
        assert (header,w,h,planes,compression)==(40,width,height*2,1,0)
        palette_size=used or 2**depth
        palette=[data[offset+40+c*4:offset+44+c*4] for c in range(palette_size)]
        rgb_reserved=b''.join(bytes((p[2],p[1],p[0],p[3])) for p in palette)
        start=offset+40+palette_size*4
        xor_stride=4*((width*depth+31)//32);and_stride=4*((width+31)//32)
        xor_rows=[];and_rows=[]
        for row in range(height):
            raw=data[start+row*xor_stride:start+(row+1)*xor_stride]
            if depth==8: indices=list(raw[:width])
            else: indices=[int(bit) for bit in ''.join(f'{v:08b}' for v in raw)[:width]]
            xor_rows.append(indices)
            mask_start=start+xor_stride*height+row*and_stride
            mask=data[mask_start:mask_start+and_stride]
            and_rows.append([int(bit) for bit in ''.join(f'{v:08b}' for v in mask)[:width]])
        indices=bytes(v for row in reversed(xor_rows) for v in row)
        mask=bytes(v for row in reversed(and_rows) for v in row)
        images.append(dict(width=width,height=height,hotspot_x=x,hotspot_y=y,bit_depth=depth,
                           directory_color_count=colors,palette_entries=palette_size,source_offset=offset,encoded_bytes=length,
                           palette_sha256=digest(rgb_reserved),xor_indices_sha256=digest(indices),and_mask_sha256=digest(mask),
                           and_one_nonzero_xor_pixels=sum(a and any(palette[v][:3]) for a,v in zip(mask,indices))))
    return dict(source_bytes=len(data),images=images)


def compare(binary,root,path):
    data=(root/path).read_bytes()
    result=subprocess.run([str(binary),str(root),str(path)],capture_output=True,text=True)
    assert result.returncode==0, f'{path}: {result.stderr}'
    actual=json.loads(result.stdout);expected=reference(data)
    assert actual==expected, f'Native/reference plane/hotspot mismatch: {path}'
    return dict(path=str(path),source_sha256=digest(data),**actual)


def fixture(width,height,depth,copies=1):
    palette=[(0,0,0,0),(255,255,255,0)] if depth==1 else [(4,4,4,0),(64,128,192,31),(255,255,0,255)]
    xstride=4*((width*depth+31)//32);astride=4*((width+31)//32)
    xor_rows=[];and_rows=[]
    for y in reversed(range(height)):
        ids=[(x+y)%len(palette) for x in range(width)]
        if depth==8: row=bytes(ids)
        else:
            bits=''.join(str(v) for v in ids).ljust(xstride*8,'1')
            row=int(bits,2).to_bytes(xstride,'big')
        xor_rows.append(row.ljust(xstride,b'\xee'))
        bits=''.join(str((x+y)%2) for x in range(width)).ljust(astride*8,'1')
        and_rows.append(int(bits,2).to_bytes(astride,'big'))
    header=struct.pack('<IiiHHIIiiII',40,width,height*2,1,depth,0,xstride*height,0,0,len(palette),0)
    image=header+b''.join(bytes(p) for p in palette)+b''.join(xor_rows)+b''.join(and_rows)
    offset=6+copies*16+3
    entries=b''.join(struct.pack('<4B2H2I',width%256,height%256,0,0,i%width,i%height,len(image),offset) for i in range(copies))
    return struct.pack('<3H',0,2,copies)+entries+b'gap'+image


def run(binary,installation):
    with tempfile.TemporaryDirectory(prefix='mnm-cursor-') as temp:
        root=Path(temp)
        fixtures=[fixture(9,3,1),fixture(7,2,8),fixture(256,256,1),fixture(5,4,8,3)]
        for i,data in enumerate(fixtures):
            path=Path(f'{i}.cur');(root/path).write_bytes(data);compare(binary,root,path)
        bad=bytearray(fixtures[0]);struct.pack_into('<H',bad,10,9)
        (root/'bad.cur').write_bytes(bad)
        result=subprocess.run([str(binary),str(root),'bad.cur'],capture_output=True)
        assert result.returncode==2
    report={'scope':'Offline native CUR byte/plane comparison; no original GDI execution or cursor presentation',
            'synthetic_matches':len(fixtures)}
    if installation:
        paths=sorted(installation.rglob('*.cur'))
        assert paths, 'No installed CUR inputs'
        files=[compare(binary,installation,path.relative_to(installation)) for path in paths]
        report.update(installed_matches=len(files),image_matches=sum(len(f['images']) for f in files),files=files,
                      inventory_sha256=digest(json.dumps(files,sort_keys=True,separators=(',',':')).encode()))
    return report


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary',type=Path)
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--report',type=Path)
    args=parser.parse_args()
    if args.installation: subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        result=run(args.binary.resolve(),args.installation.resolve() if args.installation else None)
        if args.report:
            args.report.parent.mkdir(parents=True,exist_ok=True)
            args.report.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps({k:v for k,v in result.items() if k!='files'},indent=2))
    finally:
        if args.installation: subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)

if __name__=='__main__':main()
