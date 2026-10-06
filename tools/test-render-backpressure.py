#!/usr/bin/env python3
"""Actual PE32 stalled/paused readers with independent owned raw-byte checks."""
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('pressure_idle',ROOT/'tools/test-render-command-idle.py')
idle=importlib.util.module_from_spec(spec);spec.loader.exec_module(idle)
wire,frame_v1=idle.wire,idle.frame_v1


def main():
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-render-backpressure.py','tools/test-render-command-idle.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','runtime/shadow/win32_min.h','protocols/include/mnm/render_command_ring.h','protocols/python/mnm_protocols/render_commands_v2.py']]
    sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    parent=ROOT/'working/tests/render-backpressure';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=idle.load('pressure_build','tools/build-render-bridge.py').build(True);stage=idle.load('pressure_stage','tools/prepare-shadow-experiment.py')
    report=dict(schema=1,success=True,sources=sources,cases=[],scope='Actual synthetic PE32 queue/retry scheduler and raw mapped reader. Pause/resume and slow advancing ACK beyond the deadline preserve every owned byte; idle clears demand, no-ACK stalls, cancellation, invalid ACK and hard overflow refuse with sticky reason. No original game, GPU completion, queue coalescing or recovery.')
    for mode in ['pause','progress','idle','stall','small-stall','cancel','invalid-ack','overflow']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();channel=case/'commands.bin';frame=case/'frame.bin'
        for path,header,size in [(channel,wire.initial_header(),wire.SIZE),(frame,frame_v1.initial_header(),frame_v1.SIZE)]:
            with path.open('wb') as f:
                b=bytearray(header)
                if path==channel:struct.pack_into('<I',b,16,123)
                f.write(b);f.truncate(size)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'),MNM_BACKPRESSURE_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_STALL_TIMEOUT_MS='200' if mode not in ['pause','overflow'] else '5000',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        valid=mode in ['pause','progress','idle'];length=64 if mode in ['idle','small-stall','cancel','invalid-ack'] else 2097275
        with channel.open('r+b') as f,mmap.mmap(f.fileno(),wire.SIZE) as m,(case/'wine.log').open('w') as log:
            child=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=env,stdout=log,stderr=log);consumed=0;deadline=time.monotonic()+20
            try:
                while struct.unpack_from('<I',m,20)[0]==0:
                    assert child.poll() is None and time.monotonic()<deadline,'no publication';time.sleep(.002)
                start=time.monotonic()
                if mode=='pause':time.sleep(.15);assert struct.unpack_from('<I',m,20)[0]==wire.CAPACITY
                if mode=='cancel':struct.pack_into('<I',m,32,1)
                if mode=='invalid-ack':struct.pack_into('<I',m,36,65)
                if valid:
                    while consumed<length:
                        assert struct.unpack_from('<I',m,24)[0]!=3,'valid reader refused'
                        pub=struct.unpack_from('<I',m,20)[0];n=min(65536,pub-consumed)
                        if n:
                            offset=consumed%wire.CAPACITY;first=min(n,wire.CAPACITY-offset)
                            data=bytes(m[64+offset:64+offset+first])+bytes(m[64:64+n-first])
                            assert data==bytes(((consumed+i)*13+31)&255 for i in range(n)),'owned byte mismatch'
                            consumed+=n;struct.pack_into('<I',m,36,consumed)
                            if mode=='progress':time.sleep(.04)
                        else:assert time.monotonic()<deadline;time.sleep(.001)
                if mode in ['stall','small-stall']:
                    while struct.unpack_from('<I',m,24)[0]!=3:
                        assert child.poll() is None and time.monotonic()<deadline;time.sleep(.002)
                    elapsed=time.monotonic()-start;assert elapsed<.55,(mode,elapsed)
                else:elapsed=time.monotonic()-start
                assert child.wait(timeout=10)==0,(mode,(case/'wine.log').read_text())
                state,reason=struct.unpack_from('<II',m,24);pub=struct.unpack_from('<I',m,20)[0]
                want=(2,0) if valid else (3,3 if mode=='cancel' else 5 if mode=='invalid-ack' else 1 if mode=='overflow' else 4)
                assert (state,reason)==want,(mode,state,reason)
                if mode in ['stall','small-stall']:assert 'command_queue_stalled' in (capture/'lifecycle.log').read_text()
                if mode=='progress':assert elapsed>.2
                report['cases'].append(dict(mode=mode,state=state,reason=reason,published=pub,acknowledged=consumed,owned_bytes_verified=consumed,seconds=elapsed,success=True))
            finally:
                if child.poll() is None:child.terminate();child.wait(timeout=5)
        print(mode+': passed',flush=True)
    assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items()),'Source changed during execution'
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)


if __name__=='__main__':main()
