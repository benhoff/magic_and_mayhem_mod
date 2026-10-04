#!/usr/bin/env python3
"""Independent synthetic world fixtures; optionally splice original writer captures."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
spec=importlib.util.spec_from_file_location('persistence',Path(__file__).with_name('test-persistence-loaders.py'))
codec=importlib.util.module_from_spec(spec)
spec.loader.exec_module(codec)
def w(*values): return struct.pack('<'+'I'*len(values),*values)
def animation(full): return w(7,8,9,10) if full else w(0xffffffff)
def creature(rich):
    b=w(42,1)+bytes(140)+w(24 if rich else 31)
    if rich: b+=w(5)+b'Test\0'
    b+=animation(rich)+w(0,1 if rich else 0,0,0)
    if not rich: b+=bytes(12)
    b+=bytes(48)+animation(rich)+bytes(28+42*24+469)+bytes([rich])
    if rich: b+=animation(True)
    b+=w(1 if rich else 0,0xffffffff)
    if rich: b+=w(0xffffffff)+animation(True)
    b+=bytes(25)
    for _ in range(3):
        b+=w(int(rich))
        if rich: b+=animation(True)
    b+=bytes(56)+w(2 if rich else 0)
    if rich: b+=bytes(72)+w(1)
    b+=bytes(737)+w(0,0,2 if rich else 0)
    if rich: b+=w(10,20)
    b+=w(1 if rich else 0)
    if rich: b+=w(30)
    b+=w(0,0,int(rich))
    if rich: b+=bytes(204)
    return b+bytes(129)
def fixture(rich=False, capture=None):
    out=bytearray(); expected=[]
    def add(name,data):
        expected.append((name,len(out),len(data)));out.extend(data)
    add('map-path',b'Realms/Greek/test.M3D\0'.ljust(256,b'\0'))
    out.extend(w(1,2,3,0xffffffff))
    add('resource-registry',w(int(rich))+(bytes(264) if rich else b''));out.extend(w(123))
    header=bytearray(76);struct.pack_into('<I',header,20,int(rich))
    add('map',header+(bytes(12) if rich else b'')+w(0)+bytes(16)+w(int(rich))+(bytes(40) if rich else b'')+bytes(16)+w(0)+bytes(1864)+w(3 if rich else 0)+(b'abc' if rich else b'')+w(int(rich))+(w(0xffffffff)+animation(True) if rich else b'')+bytes(20)+bytes(16)+w(int(rich))+(bytes(72) if rich else b''))
    add('map-secondary-state',bytes(8))
    add('queued-cell-references',w(int(rich))+(w(0xffffffff) if rich else b''))
    add('records-40',w(int(rich))+(bytes(40) if rich else b''))
    add('eight-state-slots',(w(1)+bytes(336) if rich else w(0))+bytes(7*4+4))
    records=[w(41,0),creature(False),creature(True)] if rich else []
    if capture and capture[0]=="creature": records=[capture[1]]
    add('creatures',bytes(8)+w(int(rich))+(bytes(24) if rich else b'')+w(len(records))+b''.join(records)+bytes(112)+w(int(rich))+(w(42) if rich else b'')+bytes(64))
    out.extend(bytes(12))
    missile=w(2,1)+bytes(32+252+56)+w(int(rich))
    if rich: missile+=w(0xffffffff)+animation(True)
    missile+=bytes(154)
    missiles=[w(1,0),missile] if rich else []
    if capture and capture[0]=='missile': missiles=[capture[1]]
    add('missiles',w(len(missiles),0)+b''.join(missiles)+bytes(12)+w(0)*4)
    effects=[w(1,0)]
    for t in (0,50,61): effects.append(w(t+2,1)+bytes(30)+w(t)+(animation(True) if t==0 else b'')+bytes(28))
    effects=effects if rich else []
    if capture and capture[0]=='effect': effects=[capture[1]]
    add('effects',w(len(effects))+b''.join(effects)+bytes(8))
    add('104-path-slots',bytes(104*256))
    add('optional-records-660',w(int(rich))+(w(1)+bytes(660+0x6878) if rich else b''))
    add('optional-records-6',w(int(rich))+(w(2)+bytes(12+16) if rich else b''))
    add('100-conditional-slots',(w(1)+w(1,2,3) if rich else w(0))+bytes(99*4))
    add('records-1148',w(0,int(rich))+(bytes(1148) if rich else b'')+bytes(412))
    add('world-controls',bytes(20))
    add('three-resource-slots',w(3 if rich else 0)+(w(3)+b'abc\0'+bytes(9))*3 if rich else w(0))
    out.extend(w(0x12345678,0x87654321))
    return bytes(out),expected

def run(binary, captures=()):
    with tempfile.TemporaryDirectory(prefix='mnm-world-') as tmp:
        root=Path(tmp);cases=[fixture(),fixture(True)]+[fixture(capture=(Path(p).name.split("-")[0],Path(p).read_bytes())) for p in captures]
        for index,(raw,expected) in enumerate(cases):
            (root/'world.bin').write_bytes(raw)
            actual=codec.inspect(binary,root,'world','world.bin')['world']
            assert actual['source_bytes']==len(raw) and actual['map_path']=='Realms/Greek/test.M3D'
            assert actual['globals']==[1,2,3,0xffffffff] and actual['counter']==123
            for name,offset,size in expected:
                found=[b for b in actual['blocks'] if b['name']==name]
                assert len(found)==1 and (found[0]['offset'],found[0]['size'])==(offset,size),(name,found,offset,size)
            for b in actual['blocks']:
                assert 0<=b['offset']<=len(raw) and b['offset']+b['size']<=len(raw)
                if b['parent'] is not None:
                    p=actual['blocks'][b['parent']];assert p['offset']<=b['offset'] and b['offset']+b['size']<=p['offset']+p['size']
            wizard=bytes(0x14a)+w(0)+bytes(1000)+w(11,22,0)+b'\0'
            save=w(0x564153,20,123)+bytes(0x200)+wizard*80+bytes(0x13dc+0x16c+0x18+0x3268)+w(8,9,0x17)+bytes(24)+raw
            (root/'save.vas').write_bytes(save);(root/'save.sav').write_bytes(codec.container(save,save,0,0xdeadbeef,no_cd=True))
            for kind,path in [('vas-world','save.vas'),('sav-world','save.sav')]:
                wrapped=codec.inspect(binary,root,kind,path)
                assert wrapped['world']['blocks']==actual['blocks'] and wrapped['world_sha256']==hashlib.sha256(raw).hexdigest()
            for cut in sorted({0,1,len(raw)-1}|{offset+size-1 for _,offset,size in expected if size}):
                (root/'bad.bin').write_bytes(raw[:cut])
                rejected=subprocess.run([str(binary),str(root),'world','bad.bin'],capture_output=True)
                assert rejected.returncode==2,(index,cut,rejected.stderr)
        (root/'bad.bin').write_bytes(cases[0][0]+b'X')
        assert subprocess.run([str(binary),str(root),'world','bad.bin'],capture_output=True).returncode==2
    print(json.dumps({'synthetic_worlds':2,'original_record_captures':len(captures),'packed_unpacked_layers_match':True,'real_save_corpus':False}))
if __name__=='__main__': run(Path(sys.argv[1]).resolve(),sys.argv[2:])
