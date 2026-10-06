#!/usr/bin/env python3
"""Actual same-process PE32 fresh-session recovery with independent GPU frames."""
import argparse
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import signal
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('recovery_live',ROOT/'tools/test-live-render-channel.py')
live=importlib.util.module_from_spec(spec);spec.loader.exec_module(live)
from mnm_protocols import render_commands_v2 as ring


def create(path,session=123,protocol=ring):
    header=bytearray(protocol.initial_header());struct.pack_into('<I',header,16,session);live.create(path,header,protocol.SIZE)


def wait_file(path,process):
    deadline=time.monotonic()+15
    while not path.exists():
        assert process.poll() is None and time.monotonic()<deadline,(path,process.poll())
        time.sleep(.005)


def archive(path):
    data=path.read_bytes();assert data[:16]==b'MNMCMD01'+struct.pack('<II',1,16)
    ops=[];ids=[];at=16
    while at<len(data):
        op,seq,size=struct.unpack_from('<III',data,at);assert seq==len(ops)+1 and at+12+size<=len(data)
        ops.append(op)
        if op==1:ids.append(struct.unpack_from('<I',data,at+12)[0])
        at+=12+size
    return dict(sha256=live.sha(path),opcodes=ops,create_ids=ids)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--case',action='append',choices=['orderly','cancel','stall','invalid','held','early','guards','repeat','budget']);args=parser.parse_args()
    assert os.environ.get('DISPLAY'),'Run under xvfb-run -a'
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-render-recovery.py','tools/build-render-bridge.py','tools/test-live-render-channel.py','tools/prepare-shadow-experiment.py','runtime/shadow/win32_min.h','tests/live-render-channel-test.cpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','renderer/commands.cpp','renderer/command_consumer.cpp','protocols/include/mnm/render_command_ring.h']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    parent=ROOT/'working/tests/render-recovery';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('recovery_build','tools/build-render-bridge.py').build(True);stage=live.load('recovery_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,success=True,sources=sources,cases=[],scope='Quiescent actual same-process PE32 RenderShutdown/RenderRecover with distinct v2 file identities and strictly increasing session IDs; poisoned borrowed rows and independent complete GPU frames. Fresh consumer per session, sticky immutable old rings/archives, lifecycle and ownership guards. No original game, automatic IPC negotiation, in-flight callback stress, partial checkpoint/palette reconstruction or real-driver equivalence.')
    for mode in args.case or ['orderly','cancel','stall','invalid','held','early','guards','repeat','budget']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();phases=16 if mode=='budget' else 3 if mode=='repeat' else 2
        channels=[case/f'commands-{i:08x}.bin' for i in range(phases+1)]
        for i,path in enumerate(channels):create(path,123+i)
        frame=case/'frame.bin';live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        guards=[]
        if mode=='guards':
            os.link(channels[0],case/'alias-old.bin')
            for name,session in [('reused-id',123),('cancelled',124),('claimed',124),('reserved',124)]:
                path=case/(name+'.bin');create(path,session);guards.append(path)
                if name!='reused-id':
                    with path.open('r+b') as f:f.seek(32 if name=='cancelled' else 24 if name=='claimed' else 40);f.write(struct.pack('<I',1))
            create(case/'v1.bin',124,live.protocol);guards.append(case/'v1.bin')
            (case/'bad-size.bin').write_bytes(b'bad');guards.append(case/'bad-size.bin')
        guard_hashes={str(p):live.sha(p) for p in guards}
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        child=dict(env,MNM_RECOVERY_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_STALL_TIMEOUT_MS='200' if mode=='stall' else '5000',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channels[0]).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        active=case/'producer.active';sessions=[];frozen={};wine=None;qt=None;stopped=False;logs=[]
        def start_reader(phase):
            output=case/f'qt-{phase:08x}.json';log=(case/f'qt-{phase:08x}.log').open('w');logs.append(log)
            process=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channels[phase]),str(active),str(output)],env=env,stdout=log,stderr=log)
            live.wait_ready(Path(str(output)+'.ready'),process);return process,output
        with (case/'wine.log').open('w') as wlog:
            try:
                qt,output=start_reader(0)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog);active.write_text(str(wine.pid))
                for phase in range(phases):
                    wait_file(case/f'drawn-{phase:08x}.bin',wine)
                    with channels[phase].open('r+b') as f,mmap.mmap(f.fileno(),ring.SIZE) as control:
                        deadline=time.monotonic()+10
                        while struct.unpack_from('<I',control,36)[0]!=struct.unpack_from('<I',control,20)[0]:
                            assert time.monotonic()<deadline;time.sleep(.005)
                        time.sleep(.05)
                        if phase==0 and mode in ['stall','invalid']:
                            os.kill(qt.pid,signal.SIGSTOP);os.waitpid(qt.pid,os.WUNTRACED);stopped=True
                        if phase==0 and mode=='cancel':struct.pack_into('<I',control,32,1)
                        if phase==0 and mode=='invalid':struct.pack_into('<I',control,36,struct.unpack_from('<I',control,20)[0]+1)
                        (case/f'go-{phase:08x}.bin').write_bytes(b'go')
                        wait_file(case/f'closed-{phase:08x}.bin',wine)
                        if stopped:os.kill(qt.pid,signal.SIGCONT);stopped=False
                        valid=not(phase==0 and mode in ['cancel','stall','invalid','held'])
                        assert qt.wait(timeout=15)==(0 if valid else 8),(mode,phase,output.read_text())
                        observed=json.loads(output.read_text());expected=[]
                        for i in range(phase*2,phase*2+2):
                            color=i*0x254713&0xffffff;expected.append(hashlib.sha256(bytes([color&255,(color>>8)&255,(color>>16)&255,255])*(512*512)).hexdigest())
                        assert observed['frames']==expected and observed['success']==valid,(mode,phase,observed)
                        assert observed.get('live_surfaces',0)==observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==0
                        pub,state,reason=struct.unpack_from('<III',control,20)
                        want=0 if valid else {'cancel':3,'stall':4,'invalid':5,'held':2}[mode]
                        assert (state,reason)==(2 if valid else 3,want),(mode,phase,state,reason)
                        if valid:assert struct.unpack_from('<I',control,36)[0]==pub
                        archived=archive(capture/f'session-{phase+1:08x}.bin')
                        if valid:assert archived['opcodes'][-3:]==[7,7,8] and archived['create_ids']==[1,2]
                        sessions.append(dict(phase=phase,valid=valid,session=123+phase,state=state,reason=reason,published=pub,archive=archived,consumer=observed))
                    frozen[str(channels[phase])]=live.sha(channels[phase]);frozen[str(capture/f'session-{phase+1:08x}.bin')]=archived['sha256']
                    for path,h in frozen.items():assert live.sha(Path(path))==h,'old session changed after handoff'
                    if phase+1<phases:qt,output=start_reader(phase+1)
                    (case/f'next-{phase:08x}.bin').write_bytes(b'next')
                assert wine.wait(timeout=15)==0,(mode,(case/'wine.log').read_text())
                active.unlink()
            finally:
                if stopped and qt and qt.poll() is None:os.kill(qt.pid,signal.SIGCONT)
                for process in [wine,qt]:
                    if process is not None and process.poll() is None:process.terminate();process.wait(timeout=5)
                for log in logs:log.close()
        for path,h in frozen.items():assert live.sha(Path(path))==h,'old terminal session mutated'
        for path,h in guard_hashes.items():assert live.sha(Path(path))==h,'rejected candidate mutated'
        assert struct.unpack_from('<I',channels[-1].read_bytes(),24)[0]==0,'unused candidate claimed'
        locks,unlocks,count=struct.unpack('<III',(case/'engine-counts.bin').read_bytes());assert locks==unlocks and count==phases
        assert locks==phases*3+(mode in ['held','stall'])
        report['cases'].append(dict(mode=mode,success=True,sessions=sessions,original_locks=locks,original_unlocks=unlocks,immutable_terminal_files=len(frozen),rejected_candidates_unchanged=len(guards)))
        print(mode+': passed',flush=True)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Sources changed during execution'
    report['full_frame_comparisons']=sum(len(s['consumer']['frames']) for c in report['cases'] for s in c['sessions'])
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
