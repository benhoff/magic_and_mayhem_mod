#!/usr/bin/env python3
"""Terminal PE32/Qt application stop, two-worker joins and borrowed-state refusal."""
import argparse,hashlib,importlib.util,json,mmap,os,shlex,shutil,struct,subprocess,tempfile,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('stop_live',ROOT/'tools/test-live-render-channel.py');live=importlib.util.module_from_spec(spec);spec.loader.exec_module(live)
from mnm_protocols import render_commands_v2 as ring

def wait_file(path,p):
    until=time.monotonic()+20
    while not path.exists():
        assert p.poll() is None and time.monotonic()<until,(path,p.poll());time.sleep(.005)

def state(path):return list(struct.unpack('<10I',path.read_bytes()))

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('build',type=Path);ap.add_argument('--case',action='append',choices=['explicit','host','auto','held','cancel','disappear','timeout','control-join','publish-join','held-dc','recovery-stop']);args=ap.parse_args();assert os.environ.get('DISPLAY')
    paths=list((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-render-application-stop.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','tests/live-command-stop-test.cpp','apps/qt-shell/render_control.cpp','apps/qt-shell/render_control.hpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_session.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','renderer/commands.cpp','renderer/command_consumer.cpp','protocols/schemas/render_control-v1.json','protocols/include/mnm/render_control_v1.h']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths};parent=ROOT/'working/tests/render-application-stop';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('stop_build','tools/build-render-bridge.py').build(True);stage=live.load('stop_stage','tools/prepare-shadow-experiment.py')
    flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Widgets','Qt6OpenGLWidgets','Qt6OpenGL'],text=True));binary=run/'live-command-stop-test'
    native=['tests/live-command-stop-test.cpp','apps/qt-shell/render_control.cpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/gl_viewport.cpp']
    subprocess.run(['g++','-std=c++17','-fPIC','-O2','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'apps/qt-shell'),'-I'+str(ROOT/'renderer'),*[str(ROOT/p) for p in native],str(args.build.resolve()/'libmnm-renderer.a'),'-o',str(binary),*flags],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,success=True,sources=sources,cases=[],scope='Real PE32 final application stop separate from recoverable session shutdown, actual persistent control/publication worker ownership, Qt semantic stop and independent native frames. Borrowed Lock/DC refusal/resolution including failed original DC return; stop while real checkpoint recovery owns admission; cancellation, reader loss, unacknowledged END, failed joins with retained storage/retry; module pin and original-only post-stop callbacks. No original artifacts/game, hardware driver, arbitrary abandoned owners or dynamic unloading support.')
    expected=[hashlib.sha256(bytes([c&255,(c>>8)&255,(c>>16)&255,255])*(512*512)).hexdigest() for c in [0,0x254713]]
    for mode in args.case or ['explicit','host','auto','held','cancel','disappear','timeout','control-join','publish-join','held-dc','recovery-stop']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();frame=case/'frame.bin';live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE);spare=case/'spare.bin';header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,124);live.create(spare,header,ring.SIZE);spare_hash=live.sha(spare)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        child=dict(env,MNM_APPLICATION_STOP_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_ORDERED_COPIES='1',MNM_RENDER_NO_READBACK='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'),MNM_RENDER_COMMAND_CHANNEL='Z:'+str(case/'commands.bin').replace('/','\\'),MNM_RENDER_CONTROL='Z:'+str(case/'commands.bin.control').replace('/','\\'))
        if mode in ['control-join','publish-join']:child['MNM_RENDER_STOP_JOIN_FOR_TEST']='c' if mode=='control-join' else 'p'
        qt=wine=None
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            try:
                qt=subprocess.Popen([str(binary),str(case),mode],env=env,stdout=qlog,stderr=qlog);wait_file(case/'ready.json',qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog);wait_file(case/'drawn-00000000.bin',wine)
                until=time.monotonic()+10
                while not (case/'progress.json').exists() or len(json.loads((case/'progress.json').read_text())['frames'])<2:
                    assert qt.poll() is None and time.monotonic()<until;time.sleep(.005)
                assert json.loads((case/'progress.json').read_text())['frames']==expected
                if mode=='recovery-stop':
                    with (case/'commands.bin').open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                    with (case/'commands.bin.control').open('r+b') as f:
                        p=mmap.mmap(f.fileno(),0);path=('Z:'+str(spare).replace('/','\\')).encode();p[64:64+len(path)]=path;struct.pack_into('<I',p,24,2);struct.pack_into('<I',p,28,124);struct.pack_into('<I',p,44,len(path));struct.pack_into('<I',p,20,1);p.close()
                    wait_file(case/'transition-enter.bin',wine)
                if mode=='host':(case/'host-request').write_bytes(b'go')
                if mode=='timeout':(case/'pause').write_bytes(b'pause');time.sleep(.03)
                if mode=='disappear':qt.terminate();qt.wait(timeout=5)
                if mode=='cancel':
                    with (case/'commands.bin').open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                (case/'go-00000000.bin').write_bytes(b'go')
                refused=None
                if mode in ['held','control-join','publish-join','held-dc','recovery-stop']:
                    wait_file(case/'refused-00000000.bin',wine);refused=state(case/'refused-state.bin');assert refused[0]==1 and refused[2]==0 and refused[8]>0
                    if mode in ['held','held-dc']:assert refused[1]==0
                    if mode=='held':assert refused[9]>0
                    if mode=='recovery-stop':assert refused[3:8]==[1,1,0,0,0]
                    if mode=='control-join':assert refused[3:8]==[1,1,1,1,1]
                    if mode=='publish-join':assert refused[1]==1 and refused[5:8]==[1,1,1]
                    if mode=='recovery-stop':(case/'transition-release.bin').write_bytes(b'go')
                    elif mode not in ['held','held-dc']:(case/('control-join-go.bin' if mode=='control-join' else 'publish-join-go.bin')).write_bytes(b'go')
                    (case/'retry-00000000.bin').write_bytes(b'go')
                closed=None
                if mode!='auto':
                    wait_file(case/'closed-00000000.bin',wine);closed=state(case/'closed-state.bin');assert closed==[1,1,1,0,0,0,0,0,0,0],(mode,closed)
                    if mode=='timeout':(case/'pause').unlink()
                    if mode!='disappear':wait_file(case/'qt.json',qt)
                    terminal=live.sha(case/'commands.bin');archived=live.sha(capture/'session-00000001.bin')
                    (case/'after-00000000.bin').write_bytes(b'go');wait_file(case/'done-00000000.bin',wine);assert state(case/'after-state.bin')==closed
                    assert live.sha(case/'commands.bin')==terminal and live.sha(capture/'session-00000001.bin')==archived,'Post-stop drawing changed retired stream'
                    (case/'exit-00000000.bin').write_bytes(b'go')
                assert wine.wait(timeout=15)==0,(mode,(case/'wine.log').read_text())
                if mode!='disappear':
                    wait_file(case/'qt.json',qt);observed=json.loads((case/'qt.json').read_text());assert observed['frames']==expected and observed['live_surfaces']==0
                    assert observed['success']==(mode not in ['cancel','recovery-stop']),observed
                    (case/'qt-exit').write_bytes(b'go');assert qt.wait(timeout=5)==0
                else:observed=dict(frames=expected,consumer_process_disappeared=True)
                if mode=='auto':
                    lines=(capture/'lifecycle.log').read_text().splitlines();stopped=[line.split() for line in lines if line.startswith('command_application_stopped ')];assert stopped and [int(stopped[-1][i],16) for i in [4,5,6]]==[1,0,0]
                if mode=='recovery-stop':
                    data=(case/'commands.bin.control').read_bytes();assert struct.unpack_from('<2I',data,32)==(1,2),'Late recovery replied READY after stop'
                assert live.sha(spare)==spare_hash,'Terminal stop revived a session'
                locks,unlocks=struct.unpack('<2I',(case/'engine-counts.bin').read_bytes());assert locks==unlocks==(4 if mode=='held' else 23 if mode=='disappear' else 3),(mode,locks,unlocks)
                if mode!='auto':assert list(struct.unpack('<2I',(case/'after-counts.bin').read_bytes()))==[locks+1,unlocks+1]
                report['cases'].append(dict(mode=mode,success=True,refused_state=refused,closed_state=closed,original_locks=locks,original_unlocks=unlocks,consumer=observed,unused_candidate_unchanged=True));print(mode+': passed',flush=True)
            finally:
                for p in [wine,qt]:
                    if p is not None and p.poll() is None:p.terminate();p.wait(timeout=5)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Sources changed during execution';report['full_frame_comparisons']=sum(len(c['consumer']['frames']) for c in report['cases']);(run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
