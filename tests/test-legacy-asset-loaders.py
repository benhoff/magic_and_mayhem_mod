#!/usr/bin/env python3
"""Independent old ANI expansion and SPR v2 indexed row decoding comparisons."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def sha(b):return hashlib.sha256(b).hexdigest()


def ani_reference(b):
    magic,size,count,version,opaque,n=struct.unpack_from('<6I',b)
    assert magic==0x00494e41 and size==len(b) and version in (3,4,5)
    starts=list(struct.unpack_from('<'+str(n)+'I',b,44));base=44+n*4
    stride={3:28,4:36,5:44}[version]
    assert len(b)==base+count*stride and starts[0]==0 and starts[-1]==count
    normalized=b''.join(b[at:at+stride]+bytes(44-stride) for at in range(base,len(b),stride))
    for begin,end in zip(starts,starts[1:]):assert begin<end and struct.unpack_from('<I',normalized,(end-1)*44)[0]==6
    return dict(version=version,opaque_header=opaque,starts=starts,records=count,sprite_name_hex=b[24:44].hex(),records_sha256=sha(normalized))


def sprite_reference(b):
    magic,size,version,count,palettes=struct.unpack_from('<5I',b)
    assert magic==0x00525053 and size==len(b) and version==2 and palettes==1
    table=20+768;base=table+count*4;offsets=struct.unpack_from('<'+str(count)+'I',b,table)
    frames=[]
    for i,off in enumerate(offsets):
        at=base+off;size,w,h,x,y=struct.unpack_from('<3I2i',b,at)
        cookie=struct.unpack_from('<I',b,at+28)[0]
        mask=bytearray(w*h);pixels=bytearray(w*h)
        for row in range(h):
            delta,pixel=struct.unpack_from('<2I',b,at+32+row*8)
            cursor=at+delta;source=at+pixel;pos=0;colour=False
            # Reference walks until width, unlike native adjacent-row extent checks.
            while pos<w:
                run=b[cursor];cursor+=1;assert pos+run<=w and cursor<=at+size
                if colour:
                    assert source+run<=at+size
                    pixels[row*w+pos:row*w+pos+run]=b[source:source+run]
                    mask[row*w+pos:row*w+pos+run]=b'\1'*run;source+=run
                pos+=run;colour=not colour
        frames.append(dict(index=i,width=w,height=h,origin_x=x,origin_y=y,name_hex=b[at+20:at+28].hex(),
                           source_offset=at,encoded_size=size,auxiliary_offsets=[0,0],palette_index=0,
                           legacy_palette_word=cookie,pixels_hex=pixels.hex(),mask_hex=mask.hex(),pixels_sha256=sha(pixels),mask_sha256=sha(mask)))
    return dict(version=2,header_flags=0,source_bytes=len(b),storage='indexed8',frame_count=count,
                empty_frames=sum(f['width']==0 and f['height']==0 for f in frames),
                pixels=sum(f['width']*f['height'] for f in frames),palettes_rgb_hex=[b[20:788].hex()],frames=frames)


def ani_fixture(version):
    stride={3:28,4:36,5:44}[version]
    body=struct.pack('<'+str(stride//4)+'I',0,0xffffffff,*range(2,stride//4))+struct.pack('<2I',6,0xffffffff)+bytes(stride-8)
    return b'ANI\0'+struct.pack('<5I',52+len(body),2,version,0x87654321,2)+bytes(range(20))+struct.pack('<2I',0,2)+body


def spr_fixture():
    palette=bytes(i%256 for i in range(768))
    frame=struct.pack('<3I2i8sI2I',48,3,1,-1,-2147483648,b'A\0\xffBCDEF',0xffffffff,40,43)+bytes([0,2,1,0,255,0,0,0])
    empty=struct.pack('<3I2i8sI',32,0,0,0,0,b'empty\0xx',0x12345678)
    return b'SPR\0'+struct.pack('<4I',796+80,2,2,1)+palette+struct.pack('<2I',0,48)+frame+empty


def compare_ani(inspector,root,paths,temp):
    manifest=temp/'ani-manifest.json';manifest.write_text(json.dumps(paths))
    rows=json.loads(subprocess.check_output([inspector,str(root),str(manifest)],text=True))['files']
    results=[]
    for path,row in zip(paths,rows):
        b=(root/path).read_bytes();expected=ani_reference(b)
        assert row['decoded'] and all(row[k]==v for k,v in expected.items()),path
        stride={3:28,4:36,5:44}[expected['version']];base=44+4*len(expected['starts'])
        records=[b[at:at+stride]+bytes(44-stride) for at in range(base,len(b),stride)]
        assert b''.join(r[:stride] for r in records)==b[base:]
        results.append(dict(path=path,source_sha256=sha(b),source_bytes=len(b),version=expected['version'],records=expected['records'],normalized_records_sha256=expected['records_sha256']))
    assert len(rows)==len(paths)
    return results


def compare_spr(inspector,root,paths,temp):
    entries=[dict(path=p,frames=list(range(struct.unpack_from('<I',(root/p).read_bytes(),12)[0]))) for p in paths]
    manifest=temp/'spr-manifest.json';manifest.write_text(json.dumps(entries))
    rows=json.loads(subprocess.check_output([inspector,'--root',str(root),'--manifest',str(manifest)],text=True))['files']
    results=[]
    for path,row in zip(paths,rows):
        b=(root/path).read_bytes();expected=sprite_reference(b)
        assert row['status']=='decoded' and all(row[k]==v for k,v in expected.items()),path
        results.append(dict(path=path,source_sha256=sha(b),source_bytes=len(b),frames=expected['frame_count'],pixels=expected['pixels'],
                            decoded_sha256=sha(json.dumps(expected,sort_keys=True,separators=(',',':')).encode())))
    assert len(rows)==len(paths)
    return results


def run(args):
    with tempfile.TemporaryDirectory() as td:
        temp=Path(td);root=temp/'inputs';root.mkdir()
        for v in (3,4,5):(root/f'v{v}.ani').write_bytes(ani_fixture(v))
        compare_ani(args.ani,root,[f'v{v}.ani' for v in (3,4,5)],temp)
        (root/'old.spr').write_bytes(spr_fixture());compare_spr(args.spr,root,['old.spr'],temp)
        report=dict(ani=[],spr=[])
        if args.installation:
            root=args.installation.resolve()
            paths=sorted(p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file() and p.suffix.lower()=='.ani')
            report['ani']=compare_ani(args.ani,root,paths,temp)
            paths=sorted(p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file() and p.suffix.lower()=='.spr' and struct.unpack_from('<I',p.read_bytes(),8)[0]==2)
            assert len(paths)==7
            report['spr']=compare_spr(args.spr,root,paths,temp)
            for r in report['ani']+report['spr']:assert sha((root/r['path']).read_bytes())==r['source_sha256']
        report['summary']=dict(ani_files=len(report['ani']),ani_legacy_files=sum(r['version']!=5 for r in report['ani']),
                               ani_records=sum(r['records'] for r in report['ani']),spr_files=len(report['spr']),
                               spr_frames=sum(r['frames'] for r in report['spr']),spr_pixels=sum(r['pixels'] for r in report['spr']))
        if args.report:args.report.write_text(json.dumps(report,indent=2)+'\n')
        print('Legacy asset comparisons passed:',json.dumps(report['summary']))


def main():
    p=argparse.ArgumentParser();p.add_argument('ani');p.add_argument('spr');p.add_argument('--installation',type=Path);p.add_argument('--report',type=Path);args=p.parse_args()
    manifest=Path(__file__).resolve().parents[1]/'tools/original-manifest.sh'
    if args.installation:subprocess.run([str(manifest),'verify'],check=True)
    try:run(args)
    finally:
        if args.installation:subprocess.run([str(manifest),'verify'],check=True)

if __name__=='__main__':main()
