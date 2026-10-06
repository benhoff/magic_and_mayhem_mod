#!/usr/bin/env python3
"""Qt same-context recovery driven by real asynchronous PE32 producer control."""
import argparse
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import shlex
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('host_live',ROOT/'tools/test-live-render-channel.py');live=importlib.util.module_from_spec(spec);spec.loader.exec_module(live)
from mnm_protocols import render_commands_v2 as ring, render_control_v1 as control

def wait(test,process,timeout=15):
    end=time.monotonic()+timeout
    while not test():
        assert process.poll() is None and time.monotonic()<end,('wait failed',process.poll())
        time.sleep(.005)

def progress(case):
    try:return json.loads((case/'progress.json').read_text())['frames']
    except (OSError,json.JSONDecodeError):return []

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--case',action='append');args=parser.parse_args()
    assert os.environ.get('DISPLAY'),'Use xvfb-run -a'
    paths=list((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['apps/qt-shell/render_control.cpp','apps/qt-shell/render_control.hpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_session.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/main.cpp','apps/qt-shell/CMakeLists.txt','renderer/CMakeLists.txt','renderer/commands.cpp','renderer/command_consumer.cpp','tests/live-command-recovery-test.cpp','tools/test-render-host-recovery.py','tools/run-opengl-game.py','tools/build-render-bridge.py','protocols/schemas/render_control-v1.json','protocols/include/mnm/render_control_v1.h','protocols/python/mnm_protocols/render_control_v1.py','protocols/tests/test_contracts.py','tests/render-control-test.cpp']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    parent=ROOT/'working/tests/render-host-recovery';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('host_build','tools/build-render-bridge.py').build(True);stage=live.load('host_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,success=True,sources=sources,cases=[],scope='Actual PE32 administrative recovery worker and Qt single-viewport/context native consumer handoffs with independent full original-fixture pixels. No original artifacts/game, original-driver equivalence, dynamic unloading or unobserved leases.')
    result=subprocess.run([str(args.build.resolve()/'render-control-test')],check=True,capture_output=True,text=True)
    report['control_native']=json.loads(result.stdout)
    binary=run/'control-sanitized'
    flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
    subprocess.run(['g++','-std=c++17','-fPIC','-O1','-g','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ROOT/'apps/qt-shell'),str(ROOT/'tests/render-control-test.cpp'),str(ROOT/'apps/qt-shell/render_control.cpp'),'-o',str(binary),*flags],check=True)
    result=subprocess.run([str(binary)],check=True,capture_output=True,text=True,env=dict(env,ASAN_OPTIONS='detect_leaks=1'),timeout=30)
    (run/'control-sanitized.log').write_text(result.stdout+result.stderr);report['control_sanitized']=json.loads(result.stdout)
    subprocess.run(['python3',str(ROOT/'protocols/tests/test_contracts.py')],check=True)
    for mode in args.case or ['healthy','repeat','exhausted','held','blocked','collision','invalid-reply','silent','exit','cancel','mutated-request','frame-timeout','startup']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();frame=case/'frame.bin';live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        child=dict(env,MNM_HOST_RECOVERY_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_NO_READBACK='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(case/'commands.bin').replace('/','\\'),MNM_RENDER_CONTROL='Z:'+str(case/'commands.bin.control').replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        if mode in ['silent','invalid-reply']:child.pop('MNM_RENDER_CONTROL')
        if mode in ['collision','cancel','mutated-request']:child['MNM_RENDER_CONTROL_DELAY_FOR_TEST']='1'
        wine=qt=None;frozen=[]
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            try:
                qt=subprocess.Popen([str(args.build.resolve()/'live-command-recovery-test'),str(case)],env=env,stdout=qlog,stderr=qlog)
                wait(lambda:(case/'ready.json').exists(),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog)
                wait(lambda:(case/'drawn-00000000.bin').exists(),wine)
                if mode!='startup':wait(lambda:len(progress(case))>=2,qt)
                (case/'go-00000000.bin').write_bytes(b'go')
                if mode=='held':time.sleep(.08)
                if mode=='blocked':wait(lambda:(case/'callback-entered.bin').exists(),wine)
                count=0 if mode=='startup' else 4 if mode=='exhausted' else 2 if mode=='repeat' else 1
                for phase in range(count):
                    channel=case/('commands.bin' if not phase else f'commands.bin.retry-{phase}')
                    with channel.open('r+b') as f,mmap.mmap(f.fileno(),ring.SIZE) as mapped:struct.pack_into('<I',mapped,32,1)
                    if mode=='blocked':(case/'request-00000000.bin').write_bytes(b'go')
                    if mode in ['invalid-reply','cancel','exit','mutated-request']:
                        cp=case/'commands.bin.control'
                        with cp.open('r+b') as f,mmap.mmap(f.fileno(),control.SIZE) as m:
                            wait(lambda:struct.unpack_from('<I',m,20)[0]>0,qt)
                            if mode=='mutated-request':struct.pack_into('<I',m,24,99)
                            if mode=='invalid-reply':struct.pack_into('<I',m,36,99)
                            if mode=='cancel':struct.pack_into('<I',m,40,1)
                        if mode=='exit':wine.terminate();wine.wait(timeout=10)
                    want=mode in ['healthy','repeat','startup'] or (mode=='exhausted' and phase<3)
                    if want:
                        wait(lambda:sum(x['session']==phase+1 for x in progress(case))>=2,qt)
                        with channel.open('r+b') as f,mmap.mmap(f.fileno(),ring.SIZE) as m:wait(lambda:struct.unpack_from('<I',m,24)[0]==3,qt)
                        frozen.append((channel,live.sha(channel)))
                    else:break
                if mode in ['healthy','repeat','startup']:
                    wait(lambda:(case/'done-00000000.bin').exists(),wine);(case/'exit-00000000.bin').write_bytes(b'go');assert wine.wait(timeout=10)==0
                assert qt.wait(timeout=15)==0,(mode,(case/'qt.log').read_text())
                observed=json.loads((case/'qt.json').read_text());assert observed['ended']==(mode in ['healthy','repeat','startup']),(mode,observed)
                assert observed['same_context'] and observed['viewport_uploads']==0 and observed['clears']>=1
                if mode=='exhausted':assert observed['recoveries']==3
                elif mode=='repeat':assert observed['recoveries']==2
                else:assert observed['recoveries']==1
                if mode not in ['healthy','repeat','startup']:assert observed['error']
                for p,h in frozen:assert live.sha(p)==h,'terminal mapping mutated after recovery'
                if mode!='exit':
                    wait(lambda:(case/'done-00000000.bin').exists(),wine);(case/'exit-00000000.bin').write_bytes(b'go');assert wine.wait(timeout=10)==0,(mode,(case/'wine.log').read_text())
                    locks,unlocks=struct.unpack('<II',(case/'engine-counts.bin').read_bytes());assert locks==unlocks==(3 if mode=='held' else 41 if mode=='blocked' else 2 if mode=='frame-timeout' else 40),(mode,locks,unlocks)
                with (case/'commands.bin.control').open('r+b') as f,mmap.mmap(f.fileno(),control.SIZE) as m:
                    assert struct.unpack_from('<I',m,40)[0]==1,'host did not cancel on finish/fallback'
                    if mode=='silent':
                        # A reply arriving after timeout cannot resume presentation.
                        struct.pack_into('<II',m,32,1,1)
                report['cases'].append(dict(mode=mode,success=True,consumer=observed,immutable_old_rings=len(frozen),original_counts_equal=mode!='exit'))
                print(mode+': passed',flush=True)
            finally:
                for process in [wine,qt]:
                    if process and process.poll() is None:process.terminate();process.wait(timeout=5)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Sources changed during execution'
    report['full_frame_comparisons']=sum(len(c['consumer']['frames']) for c in report['cases']);(run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
