#!/usr/bin/env python3
"""Synthetic PE32 main-image ExitProcess dispatch through owned native GPU cleanup."""
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
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('orchestrate_live',ROOT/'tools/test-live-render-channel.py')
live=importlib.util.module_from_spec(spec);spec.loader.exec_module(live)
from mnm_protocols import render_commands_v2 as ring


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);args=parser.parse_args()
    assert os.environ.get('DISPLAY'),'Run under xvfb-run -a'
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-render-orchestration.py','tools/build-render-bridge.py','tools/test-live-render-channel.py','tools/prepare-shadow-experiment.py','runtime/shadow/win32_min.h','tests/live-render-channel-test.cpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','renderer/commands.cpp','renderer/command_consumer.cpp','protocols/include/mnm/render_command_ring.h']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    parent=ROOT/'working/tests/render-orchestration';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('orchestration_build','tools/build-render-bridge.py').build(True);stage=live.load('orchestration_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,success=True,sources=sources,cases=[],scope='Actual synthetic PE32 resolved main-image ExitProcess IAT dispatch, explicit startup/idempotent shutdown and native GPU cleanup. Independent full frames. No original game/driver, crashes, forced termination, dynamic hook unloading or recovery.')
    for mode in ['auto','explicit','held','nopresent','empty','guards','invalid','no-capture','missing-channel','bad-size','long-path']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();channel,frame=case/'commands.bin',case/'frame.bin'
        protocol=live.protocol if mode=='invalid' else ring
        header=bytearray(protocol.initial_header());struct.pack_into('<I',header,16,123)
        live.create(channel,header,protocol.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        active,output=case/'producer.active',case/'qt.json'
        child=dict(env,MNM_ORCHESTRATION_SELFTEST='x' if mode=='explicit' else 'invalid' if mode in ['no-capture','missing-channel','bad-size','long-path'] else mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        if mode=='guards':child['MNM_RENDER_AUTO_SHUTDOWN']='0'
        if mode=='no-capture':child.pop('MNM_RENDER_LOCK_CAPTURE_DIR')
        if mode in ['missing-channel','bad-size']:
            path=case/'bad-channel.bin'
            if mode=='bad-size':path.write_bytes(b'bad')
            child['MNM_RENDER_COMMAND_CHANNEL']='Z:'+str(path).replace('/','\\')
        if mode=='long-path':child['MNM_RENDER_COMMAND_CHANNEL']='Z:'+600*'x'
        valid=mode in ['auto','explicit','guards']
        unbound=mode in ['missing-channel','bad-size','long-path']
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            qt=None if unbound else subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],env=env,stdout=qlog,stderr=qlog);wine=None
            try:
                if qt:live.wait_ready(Path(str(output)+'.ready'),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog);active.write_text(str(wine.pid))
                assert wine.wait(timeout=20)==0,(mode,(case/'wine.log').read_text())
                active.unlink()
                if qt:assert qt.wait(timeout=20)==(0 if valid else 8),(mode,output.read_text())
            finally:
                for process in [wine,qt]:
                    if process is not None and process.poll() is None:process.terminate();process.wait(timeout=5)
        observed=dict(success=False,frames=[],consumer_started=False) if unbound else json.loads(output.read_text());frames=2 if mode in ['auto','explicit','guards','held'] else 0
        # Refusal clears the consumer presentation; frames observed before the
        # ownership refusal still have independently verified original pixels.
        assert observed['frames']==([hashlib.sha256(bytes([(i*19)&255,(i*71)&255,(i*37)&255,255])*(512*512)).hexdigest() for i in range(frames)])
        assert observed['success']==valid
        if not unbound:assert observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==observed.get('live_surfaces',0)==0
        control=channel.read_bytes()[:64];published,state,reason=struct.unpack_from('<III',control,20)
        assert state==(0 if unbound else 2 if valid else 3),(mode,state,reason)
        assert reason==(0 if valid or unbound else 5 if mode=='invalid' else 2),(mode,state,reason)
        if mode!='no-capture':
            log=(capture/'lifecycle.log').read_text();assert 'command_lifecycle_shutdown' in log
        if valid:
            assert struct.unpack_from('<I',control,36)[0]==published
            data=(capture/'session-00000001.bin').read_bytes();at=16;ops=[]
            while at<len(data):
                op,seq,size=struct.unpack_from('<III',data,at);ops.append(op);assert seq==len(ops);at+=12+size
            assert at==len(data) and ops[-3:]==[7,7,8] and ops.count(6)==2
        report['cases'].append(dict(mode=mode,success=True,state=state,reason=reason,published=published,consumer=observed))
        print(mode+': passed',flush=True)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Sources changed during execution'
    report['full_frame_comparisons']=sum(len(c['consumer']['frames']) for c in report['cases'])
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
