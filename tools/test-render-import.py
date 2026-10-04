#!/usr/bin/env python3
"""Probe the pinned PE import hook with a replacement fixture entry point (no game loop)."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host-display',action='store_true',help='Use the inherited X11 display instead of isolated Xvfb')
    parser.add_argument('--hardware',action='store_true',help='Use default driver selection instead of forcing Mesa software rendering')
    args=parser.parse_args()
    if args.host_display and not os.environ.get('DISPLAY'):raise ValueError('--host-display requires DISPLAY')
    stage=load('stage','tools/prepare-shadow-experiment.py')
    source=REPO/'working/game-nocd/Chaos.exe';data=source.read_bytes()
    if hashlib.sha256(data).hexdigest()!=stage.HASH:raise ValueError("Unsupported executable hash; refusing fixture patch")
    pe=struct.unpack_from('<I',data,60)[0];opt=pe+24
    assert struct.unpack_from('<I',data,opt+28)[0]==0x400000
    patched=bytearray(stage.add_import(data,dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    count=struct.unpack_from('<H',patched,pe+6)[0];table=opt+struct.unpack_from('<H',patched,pe+20)[0]
    last=table+(count-1)*40;size,rva,rawsize,raw=struct.unpack_from('<4I',patched,last+8)
    code_offset=(size+3)//4*4;entry=rva+code_offset;address=0x400000+entry
    result=address+192;counter=address+196;callback=address+128;callback_ex=address+144
    code=bytearray()
    def push(value):code.extend(b'\x68'+struct.pack('<I',value))
    def call(target):code.extend(b'\xe8'+struct.pack('<i',target-(entry+len(code)+5)))
    push(counter);push(callback);call(0x197560) # imported EnumerateA
    push(address+200);code.extend(b'\xff\x15'+struct.pack('<I',0x5c50f4)) # GetModuleHandleA(ddraw.dll)
    push(address+212);code.extend(b'\x50\xff\x15'+struct.pack('<I',0x5c50d4)) # GetProcAddress(EnumerateExA)
    code.extend(b'\x85\xc0\x74\x11');push(1);push(counter);push(callback_ex);code.extend(b'\xff\xd0')
    push(0);push(result);push(0);call(0x19755a)
    code.extend(b'\x31\xc0\x83\x3d'+struct.pack('<I',counter)+b'\x02\x0f\x92\xc0\x50') # exit 1 unless both callback paths ran
    code.extend(b'\xff\x15'+struct.pack('<I',0x5c51ac))
    assert len(code)<128
    code.extend(bytes(128-len(code)))
    # Callback increments the supplied context counter, and continues enumeration.
    body=b'\x8b\x44\x24\x10\xff\x00\xb8\x01\x00\x00\x00\xc2'
    code.extend(body+b'\x10\x00');code.extend(bytes(144-len(code)));code.extend(body+b'\x14\x00')
    code.extend(bytes(200-len(code)));code.extend(b'ddraw.dll\0');code.extend(bytes(212-len(code)));code.extend(b'DirectDrawEnumerateExA\0')
    assert code_offset+len(code)<=rawsize
    patched[raw+code_offset:raw+code_offset+len(code)]=code
    struct.pack_into('<I',patched,last+8,code_offset+len(code))
    struct.pack_into('<I',patched,last+36,0xe0000040)
    struct.pack_into('<I',patched,opt+16,entry)
    dll=load('build','tools/build-render-bridge.py').build()
    parent=REPO/'working/tests/render-import';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(root/'probe.exe').write_bytes(patched)
    shutil.copy2(dll,root/dll.name)
    for dependency in source.parent.glob('*.dll'):shutil.copy2(dependency,root/dependency.name)
    frame=root/'frame.bin'
    with frame.open('wb') as f:f.write(frame_protocol.initial_header());f.truncate(frame_protocol.SIZE)
    capture=root/'capture';capture.mkdir()
    env=os.environ.copy();env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',
        LIBGL_ALWAYS_SOFTWARE='1',__GLX_VENDOR_LIBRARY_NAME='mesa',MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),
        MNM_RENDER_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
    if args.hardware:
        for name in ('LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES'):env.pop(name,None)
    else:
        env['__EGL_VENDOR_LIBRARY_FILENAMES']='/usr/share/glvnd/egl_vendor.d/50_mesa.json'
    env.pop('MNM_RENDER_HISTORY',None)
    with (root/'wine.log').open('w') as log:
        result=subprocess.run(([] if args.host_display else ['xvfb-run','-a'])+['wine',str(root/'probe.exe')],cwd=root,env=env,stdout=log,stderr=log,timeout=30)
    with frame.open('rb') as f:header=struct.unpack('<16I',f.read(64))
    report={'origin':'pinned_import_entry_fixture','source_sha256':stage.HASH,
        'fixture_sha256':hashlib.sha256(patched).hexdigest(),'entry_rva':hex(entry),
        'dll_sha256':hashlib.sha256((root/dll.name).read_bytes()).hexdigest(),'exit_status':result.returncode,
        'bridge_status':header[9],'create_hresult':hex(header[11]),'create_calls':header[12],'enumerate_hresult':hex(header[13]),'enumerate_calls':header[14],'last_enumerate_kind':header[15],
        'display':env.get('DISPLAY') if args.host_display else 'isolated Xvfb','hardware':args.hardware,'game_loop_executed':False,'source_unchanged':hashlib.sha256(source.read_bytes()).hexdigest()==stage.HASH}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(root,report)
    assert report["source_unchanged"]
    assert result.returncode==0 and header[14]==2 and header[13]==0 and header[15]==2 and header[12]==1 and header[9] in (7,8,10),report
if __name__=='__main__':main()
