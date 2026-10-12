#!/usr/bin/env python3
"""Check V13 Save/Load admission, transactions and original callbacks in private PE32."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
    parent=ROOT/'working/tests/storage-menu-bridge';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        exporter=module('storage_export','tools/export-menu-support.py');exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=exporter.pinned_image(exe);header='/* Whole-file hash-pinned original control bytes; private fixture only. */\n'
        for name,start,end in [('load_button',0x4aa800,0x4aa84d),('save_button',0x4ab750,0x4ab865),('edit_set',0x4cd0e0,0x4cd10f),('list_select',0x4d1060,0x4d113d),('list_get',0x4d1df0,0x4d1e4d)]:
            at=exporter.image_offset(data,start,end-start);header+='static const unsigned char '+name+'[]={'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
        (out/'storage_bridge_bytes.h').write_text(header);module('storage_build','tools/build-menu-observer.py').build(out,selftest=True,fixture=False)
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(out)]
        subprocess.run(flags+['-c',str(ROOT/'tests/storage-menu-bridge-reference.c'),'-o',str(out/'reference.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/dynamicbase:no','/nodefaultlib','/timestamp:0','/out:'+str(out/'reference.exe'),str(out/'reference.obj'),str(out/'kernel32.lib'),str(out/'user32.lib')],check=True)
        channel=bytearray(131072);channel[:16]=b'MNMMCM13'+struct.pack('<II',13,len(channel));struct.pack_into('<III',channel,16,2,1,1);(out/'channel.bin').write_bytes(channel)
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(ROOT/'working/tests/save-load-contract-wine'),WINEDEBUG='-all',MNM_MENU_CHANNEL='Z:'+str(out/'channel.bin').replace('/','\\'))
        with (out/'wine.log').open('w') as log:subprocess.run(['wine',str(out/'reference.exe')],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Input changed')
        sources=['tools/test-storage-menu-bridge.py','tests/storage-menu-bridge-reference.c','tools/build-menu-observer.py','tools/export-menu-support.py','runtime/shadow/win32_min.h','runtime/menu/storage.h','runtime/menu/channel.h','runtime/menu/observer.c','runtime/menu/mini.h','protocols/include/mnm/menu_v13.h']
        report=dict(success=True,source_sha256=exporter.HASH,real_game_launched=False,serializer_and_delete_io_stubbed=True,scope='V13 fixed Main/campaign parent guards, original control identity, row/name/stale/modal/retired/once-only guards, original selection/edit/Load/Save/Delete callback mapping; serializer, disk space, fade and file deletion dependencies stubbed; no live integration or replacement',sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Storage V13 private adapter fixture passed')
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
