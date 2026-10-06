#!/usr/bin/env python3
"""Exercise PE32 idle retry/shutdown with an independent delayed mapped reader."""
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import render_commands_v2 as wire, frame_v1

def load(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/'tools/test-render-command-idle.py',ROOT/'tools/build-render-bridge.py',ROOT/'tools/prepare-shadow-experiment.py',ROOT/'protocols/include/mnm/render_command_ring.h']
    sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    parent=ROOT/'working/tests/render-command-idle';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=load('idle_build','tools/build-render-bridge.py').build(True);stage=load('idle_stage','tools/prepare-shadow-experiment.py')
    report=dict(success=True,sources=sources,scope='Actual PE32 idle queue/thread and explicit shutdown; independent raw owned byte reader, no original game/GPU comparison.',cases=[])
    for mode in ['idle','shutdown','cancel','timeout','detach']:
        case=run/mode;case.mkdir();channel=case/'commands.bin';frame=case/'frame.bin'
        for path,header,size in [(channel,wire.initial_header(),wire.SIZE),(frame,frame_v1.initial_header(),frame_v1.SIZE)]:
            with path.open('wb') as f:
                b=bytearray(header)
                if path==channel:struct.pack_into('<I',b,16,123)
                f.write(b);f.truncate(size)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'),MNM_COMMAND_IDLE_SELFTEST=mode,MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'))
        with channel.open('r+b') as f,mmap.mmap(f.fileno(),wire.SIZE) as m,(case/'wine.log').open('w') as log:
            child=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=env,stdout=log,stderr=log)
            consumed=0;deadline=time.monotonic()+20
            try:
                while struct.unpack_from('<I',m,20)[0]==0:
                    assert child.poll() is None and time.monotonic()<deadline,'no publication';time.sleep(.005)
                # Reader intentionally absent until mapped storage is full.
                time.sleep(.25);assert struct.unpack_from('<I',m,20)[0]==wire.CAPACITY
                if mode=='cancel':struct.pack_into('<I',m,32,1)
                if mode in ['idle','shutdown']:
                    while consumed<2097275:
                        pub=struct.unpack_from('<I',m,20)[0];n=min(65536,pub-consumed)
                        if n:
                            offset=consumed%wire.CAPACITY;first=min(n,wire.CAPACITY-offset);b=bytes(m[64+offset:64+offset+first])+bytes(m[64:64+n-first])
                            assert b==bytes(((consumed+i)*13+31)&255 for i in range(n)), 'owned byte mismatch'
                            consumed+=n;struct.pack_into('<I',m,36,consumed)
                        else:assert time.monotonic()<deadline,'idle progress stalled';time.sleep(.001)
                assert child.wait(timeout=10)==0,mode
                state,reason=struct.unpack_from('<II',m,24);pub=struct.unpack_from('<I',m,20)[0]
                expected=(2,0) if mode in ['idle','shutdown'] else (3,3 if mode=='cancel' else 4)
                assert (state,reason)==expected,(mode,state,reason)
                report['cases'].append(dict(mode=mode,state=state,reason=reason,published=pub,acknowledged=consumed,owned_bytes_verified=consumed,success=True))
            finally:
                if child.poll() is None:child.terminate();child.wait(timeout=5)
        print(mode+': passed',flush=True)
    assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items()),'sources changed'
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
