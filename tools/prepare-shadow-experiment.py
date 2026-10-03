#!/usr/bin/env python3
"""Stage a pinned disposable game with one additional logging-DLL import."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def add_import(data,dll='MnmShadow.dll',symbol_name='ShadowAnchor',section_name=b'.mnmcap'):
    """Preserve old sections/IATs; add one RW data section and import descriptor."""
    if len(section_name)>8 or not symbol_name.isidentifier():raise ValueError('Invalid import names')
    section_name=section_name.ljust(8,b'\0')
    if data[:2]!=b'MZ':raise ValueError('Not a PE image')
    pe=struct.unpack_from('<I',data,60)[0];opt=pe+24
    if data[pe:pe+4]!=b'PE\0\0' or struct.unpack_from('<H',data,pe+4)[0]!=0x14c or struct.unpack_from('<H',data,opt)[0]!=0x10b:
        raise ValueError('Expected PE32 i386')
    count=struct.unpack_from('<H',data,pe+6)[0];table=opt+struct.unpack_from('<H',data,pe+20)[0]
    section_alignment,file_alignment=struct.unpack_from('<II',data,opt+32)
    if not section_alignment or not file_alignment:raise ValueError('Invalid PE alignment')
    headers=struct.unpack_from('<I',data,opt+60)[0]
    if table+(count+1)*40>headers or any(data[table+count*40:table+(count+1)*40]):
        raise ValueError('No unused section header slot')
    if any(data[table+i*40:table+i*40+8]==section_name for i in range(count)):
        raise ValueError('Shadow import already staged')
    sections=[struct.unpack_from('<8sIIII',data,table+i*40) for i in range(count)]
    def offset(rva,length):
        for _,_,base,size,raw in sections:
            if base<=rva and rva+length<=base+size:return raw+rva-base
        raise ValueError('Import data not file-backed')
    import_rva,_=struct.unpack_from('<II',data,opt+104)
    descriptors=[]
    for index in range(256):
        at=offset(import_rva+index*20,20)
        descriptor=data[at:at+20]
        if descriptor==bytes(20):break
        descriptors.append(descriptor)
    else:raise ValueError('Unbounded import table')
    if not descriptors:raise ValueError('Missing original imports')
    align=lambda n,a:(n+a-1)//a*a
    rva=align(max(base+max(virtual,size) for _,virtual,base,size,_ in sections),section_alignment)
    raw=align(len(data),file_alignment)
    blob=bytearray((len(descriptors)+2)*20)
    for index,descriptor in enumerate(descriptors):blob[index*20:index*20+20]=descriptor
    name=rva+len(blob);blob.extend(dll.encode('ascii')+b'\0')
    if len(blob)%2:blob.append(0)
    symbol=rva+len(blob);blob.extend(b'\0\0'+symbol_name.encode('ascii')+b'\0')
    while len(blob)%4:blob.append(0)
    lookup=rva+len(blob);blob.extend(struct.pack('<2I',symbol,0))
    iat=rva+len(blob);blob.extend(struct.pack('<2I',symbol,0))
    struct.pack_into('<5I',blob,len(descriptors)*20,lookup,0,0,name,iat)
    result=bytearray(data);result.extend(bytes(raw-len(result)))
    result.extend(blob);result.extend(bytes(align(len(blob),file_alignment)-len(blob)))
    struct.pack_into('<8sIIIIIIHHI',result,table+count*40,section_name,len(blob),rva,
                     align(len(blob),file_alignment),raw,0,0,0,0,0xc0000040)
    struct.pack_into('<H',result,pe+6,count+1)
    struct.pack_into('<I',result,opt+56,align(rva+len(blob),section_alignment))
    struct.pack_into('<I',result,opt+8,struct.unpack_from('<I',result,opt+8)[0]+align(len(blob),file_alignment))
    struct.pack_into('<II',result,opt+104,rva,(len(descriptors)+2)*20)
    struct.pack_into('<I',result,opt+64,0) # invalidate old PE checksum
    struct.pack_into('<II',result,opt+96+11*8,0,0) # no stale bound imports
    return bytes(result)

def prepare():
    source=REPO/'working/game-nocd';data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported executable hash; refusing staging patch')
    spec=importlib.util.spec_from_file_location('bridge_build',REPO/'tools/build-shadow-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    dll=module.build()
    parent=REPO/'working/experiments/neighbor-shadow';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Game copy hash changed')
    patched=add_import(data)
    (game/'Chaos.exe').write_bytes(patched);shutil.copy2(dll,game/'MnmShadow.dll');(game/'shadow').mkdir()
    manifest={'origin':'runtime_shadow','source_sha256':HASH,'staged_sha256':hashlib.sha256(patched).hexdigest(),
              'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'game_copy':str(game),
              'scope':'Standard creature neighbor expansion; original algorithm remains in control',
              'default_samples':1,'patch':'Added DLL import in new data section; original code bytes unchanged'}
    shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    (root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    if hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Source executable changed')
    return root

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.parse_args()
    root=prepare();print(f'Evidence directory: {root}')
    print(f'Launch: ./tools/run-neighbor-shadow.py {root}')
