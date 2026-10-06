#!/usr/bin/env python3
"""Bounded original-game recovery negotiation; pixel equivalence is unclaimed."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('live_observation',ROOT/'tools/test-live-render-game.py');helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)

def wait(test,process,timeout):
    deadline=time.monotonic()+timeout
    while not test():
        if process.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Bounded observation startup failed')
        time.sleep(.05)

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');parser.add_argument('--require-sustained',action='store_true',help='Require one healthy recovered session and at least 100 frames spanning 11 seconds');args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Run under Xvfb')
    parent=ROOT/'working/tests/live-render-recovery';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    paths=list((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-live-render-recovery.py','tools/test-live-render-game.py','tools/run-opengl-game.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','tests/live-render-recovery-probe.cpp','renderer/CMakeLists.txt','renderer/commands.cpp','renderer/commands.hpp','renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/blit.cpp','renderer/blit.hpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_session.hpp','apps/qt-shell/render_control.cpp','apps/qt-shell/render_control.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','protocols/include/mnm/render_control_v1.h','protocols/include/mnm/render_commands_v2.h','protocols/include/mnm/render_stream_v2.h','protocols/include/mnm/render_command_ring.h']]
    sources={str(p.relative_to(ROOT)):helper.sha(p) for p in paths};report=dict(schema=1,sources=sources,cases=[],scope='Three bounded original no-CD game startup observations with continuous rendering, guarded native recovery/checkpoint negotiation and retained original window. No gameplay route, independent pixel equivalence or live replacement claim.',pixel_equivalence=False,live_replacement=False,sustained_recovery_required=args.require_sustained)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(run/'wineprefix'));env.pop('WAYLAND_DISPLAY',None)
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        subprocess.run(['cp','-a','--reflink=auto',str(args.prefix_template.resolve()),env['WINEPREFIX']],check=True)
        with (run/'wineboot.log').open('w') as log:subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=log,check=True,timeout=90)
        for mode in ['early-checkpoint','checkpoint','recover']:
            case=run/mode;case.mkdir();qt=wine=None
            try:
                with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
                    qt=subprocess.Popen([str(args.build.resolve()/'live-render-recovery-probe'),str(case),'checkpoint' if mode.endswith('checkpoint') else 'recover'],env=env,stdout=qlog,stderr=qlog,start_new_session=True);wait(lambda:(case/'ready.json').exists(),qt,20)
                    frame=case/'frame.bin';helper.create(frame,helper.frame_v1.initial_header(),helper.frame_v1.SIZE)
                    child=dict(env,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_SESSION_ARCHIVE='1')
                    staged=subprocess.run(['python3',str(ROOT/'tools/run-opengl-game.py'),'--stream',str(frame),'--command-channel',str(case/'commands.bin'),'--render-control',str(case/'commands.bin.control'),'--capture-draws','--skip-movies','--stage-only'],env=child,capture_output=True,text=True,check=True,timeout=90)
                    (case/'stage.log').write_text(staged.stdout+staged.stderr);experiment=Path(next(line.removeprefix('Render experiment: ') for line in staged.stdout.splitlines() if line.startswith('Render experiment: ')));metadata=json.loads((experiment/'manifest.json').read_text());game=experiment/'game'
                    for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]:assert helper.sha(game/name)==metadata[key]
                    for key,path in [('MNM_RENDER_STREAM',frame),('MNM_RENDER_COMMAND_CHANNEL',case/'commands.bin'),('MNM_RENDER_CONTROL',case/'commands.bin.control'),('MNM_RENDER_LOCK_CAPTURE_DIR',experiment/'lock-capture'),('MNM_RENDER_CAPTURE_DIR',experiment/'draw-capture'),('MNM_RENDER_FAILURE_LOG',experiment/'surface-failures.log')]:child[key]='Z:'+str(path).replace('/','\\')
                    wine=subprocess.Popen(['wine','explorer','/desktop=RecoveryObservation,800x600',str(game/'Chaos.exe')],cwd=game,env=child,stdout=wlog,stderr=wlog,start_new_session=True)
                    started=time.monotonic();lifecycle=experiment/'lock-capture/lifecycle.log'
                    if mode=='early-checkpoint':
                        wait(lambda:lifecycle.exists() and 'blit_propagated ' in lifecycle.read_text(),wine,90)
                    else:
                        wait(lambda:struct.unpack_from('<I',frame.read_bytes(),helper.frame_v1.FRAME_COUNT_OFFSET)[0]>0,wine,90)
                        time.sleep(2)
                    # Simulate disappearance of the old reader. CHECKPOINT is strict;
                    # ordinary recovery is selected by the service's failed-ring path.
                    with (case/'commands.bin').open('r+b') as file:
                        file.seek(32);file.write(struct.pack('<I',1))
                    wait(lambda:struct.unpack_from('<I',(case/'commands.bin').read_bytes(),24)[0]==3,wine,10)
                    initial=(case/'commands.bin').read_bytes()[:64];(case/'attach').write_bytes(b'attach');assert qt.wait(timeout=20)==0,(case/'qt.log').read_text();observed=json.loads((case/'qt.json').read_text());assert observed['success'] and observed['recoveries']>=1
                    time.sleep(2);assert wine.poll() is None,'Original drawing process exited during fallback observation'
                    subprocess.run(['import','-window','root',str(case/'original-window.png')],env=env,timeout=10,check=True)
                    diagnostics=(experiment/'lock-capture/lifecycle.log').read_text();control=(case/'commands.bin.control').read_bytes();request,operation,target,response,status=struct.unpack_from('<5I',control,20)
                    first=next(n for n in observed['negotiations'] if n['request']==n['response']==1 and n['status'] in [1,2])
                    assert first['operation']==(2 if mode.endswith('checkpoint') else 1) and first['target']==124
                    assert observed['terminal_resources']==observed['ordinary_readbacks']==observed['viewport_uploads']==0
                    frames=observed['frames']
                    sustained=bool(observed['state']==1 and not observed['error'] and observed['recoveries']==1 and len(frames)>=100 and frames[-1]['ms']-frames[0]['ms']>=11000)
                    if args.require_sustained and mode!='early-checkpoint':assert sustained,observed
                    policies=[dict(checkpoint=int(line.split()[4],16),session=int(line.split()[5],16)) for line in diagnostics.splitlines() if line.startswith('command_recovery_policy ')]
                    initial_policy=next((p['checkpoint'] for p in policies if p['session']==first['target']),None)
                    outcome='checkpoint_ready' if first['status']==1 and (mode.endswith('checkpoint') or initial_policy==1) else 'fresh_observations_ready' if first['status']==1 else 'ownership_or_state_refused'
                    assert diagnostics and ('command_recovery_' in diagnostics)
                    report['source_sha256']=metadata['source_sha256'];report['cases'].append(dict(mode=mode,success=True,sustained_publication_observed=sustained,recovery_policies=policies,elapsed_seconds=time.monotonic()-started,outcome=outcome,operation=first['operation'],target_session=first['target'],producer_status=first['status'],final_control=dict(request=request,operation=operation,target=target,response=response,status=status),consumer=observed,pre_request_original_drawing_observed=True,pre_request_primary_frame_observed=mode!='early-checkpoint',original_process_alive=True,original_drawing_retained=True,initial_failure_reason=struct.unpack_from('<I',initial,28)[0],lifecycle=diagnostics.splitlines(),experiment=str(experiment.relative_to(ROOT)),artifacts={str(p.relative_to(ROOT)):helper.sha(p) for p in [case/'qt.json',case/'stage.log',case/'wine.log',case/'original-window.png',experiment/'manifest.json',experiment/'bridge-build.json',experiment/'lock-capture/lifecycle.log']}))
                    print(mode+': '+outcome+', frames='+str(len(observed['frames'])),flush=True)
            finally:helper.stop(qt);helper.stop(wine);subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        assert all(helper.sha(ROOT/p)==h for p,h in sources.items()),'Source changed during observation'
        report.update(success=True,original_manifest_verified_before_after=True)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
