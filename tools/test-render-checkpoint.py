#!/usr/bin/env python3
"""Late Qt attachment and repeated retained checkpoints through real PE32 control."""
import argparse
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
spec=importlib.util.spec_from_file_location('checkpoint_mutations',ROOT/'tools/test-render-mutations.py');mutation=importlib.util.module_from_spec(spec);spec.loader.exec_module(mutation)
live=mutation.live

def wait(test,process,timeout=15):
    end=time.monotonic()+timeout
    while not test():
        assert process.poll() is None and time.monotonic()<end,('wait failed',process.poll())
        time.sleep(.005)

def progress(case):
    try:return json.loads((case/'progress.json').read_text())['frames']
    except (OSError,json.JSONDecodeError):return []

def mark(case,prefix,phase):
    (case/(prefix+f'{phase:08x}.bin')).write_bytes(b'go')

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--case',action='append',choices=['rgb','indexed','incomplete','partial','palette-incomplete','held','dc','capacity','budget','uncertain']);args=parser.parse_args()
    assert os.environ.get('DISPLAY'),'Use xvfb-run -a'
    paths=list((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['apps/qt-shell/render_control.cpp','apps/qt-shell/render_control.hpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_session.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','renderer/CMakeLists.txt','renderer/commands.cpp','renderer/commands.hpp','renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/blit.cpp','renderer/blit.hpp','tests/live-command-checkpoint-test.cpp','tools/test-render-checkpoint.py','tools/test-render-mutations.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','protocols/schemas/render_control-v1.json','protocols/include/mnm/render_control_v1.h','protocols/python/mnm_protocols/render_control_v1.py','protocols/include/mnm/render_stream_v2.h']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    parent=ROOT/'working/tests/render-checkpoint';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('checkpoint_build','tools/build-render-bridge.py').build(True);stage=live.load('checkpoint_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,sources=sources,cases=[],scope='Actual PE32 administrative checkpoint worker and deliberately delayed Qt consumer in one viewport/context. Complete independently owned original-fixture pixels/palettes, preserved metadata, partial/fill/copy/flip/shared palette continuation and three fresh sessions; strict incomplete/borrowed-state refusal. No original artifacts/game or driver equivalence.')
    for mode in args.case or ['rgb','indexed','incomplete','partial','palette-incomplete','held','dc','capacity','budget','uncertain']:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();frame=case/'frame.bin';live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name);(case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        child=dict(env,MNM_CHECKPOINT_SELFTEST=mode,MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_CONTINUOUS='1',MNM_RENDER_NO_READBACK='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(case/'commands.bin').replace('/','\\'),MNM_RENDER_CONTROL='Z:'+str(case/'commands.bin.control').replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        valid=mode in ['rgb','indexed'];wine=qt=None;frozen=[]
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            try:
                qt=subprocess.Popen([str(args.build.resolve()/'live-command-checkpoint-test'),str(case)],env=env,stdout=qlog,stderr=qlog);wait(lambda:(case/'ready.json').exists(),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog)
                for phase in range(3 if valid else 1):
                    wait(lambda:(case/f'checkpoint-ready-{phase:08x}.bin').exists(),wine)
                    time.sleep(.15)
                    if not phase:assert not progress(case),'initial consumer consumed stale incremental history'
                    (case/f'attach-{phase}').write_bytes(b'go')
                    if valid:
                        wait(lambda:sum(x['session']==phase+1 for x in progress(case))>=1,qt)
                        mark(case,'resume-',phase)
                        wait(lambda:(case/f'checkpoint-drawn-{phase:08x}.bin').exists(),wine)
                        wait(lambda:sum(x['session']==phase+1 for x in progress(case))==(8 if mode=='indexed' else 6),qt)
                    else:
                        assert qt.wait(timeout=10)==0,(mode,(case/'qt.log').read_text());mark(case,'resume-',phase)
                        wait(lambda:(case/f'checkpoint-drawn-{phase:08x}.bin').exists(),wine)
                    old=case/('commands.bin' if not phase else f'commands.bin.retry-{phase}')
                    old_header=old.read_bytes()[:64];assert struct.unpack_from('<I',old_header,24)[0]==3
                    frozen.append((old,live.sha(old)));mark(case,'advance-',phase)
                wait(lambda:(case/'checkpoint-done-00000000.bin').exists(),wine);mark(case,'exit-',0);assert wine.wait(timeout=10)==0,(mode,(case/'wine.log').read_text());assert qt.wait(timeout=10)==0,(mode,(case/'qt.log').read_text())
                observed=json.loads((case/'qt.json').read_text());expected=mutation.frames((case/'mutation-frames.bin').read_bytes())
                assert observed['ended']==valid and observed['same_context'] and observed['requests']==(3 if valid else 1),(mode,observed)
                assert [x['hash'] for x in observed['frames']]==(expected[1:] if valid else []),(mode,observed,expected)
                for path,digest in frozen:assert live.sha(path)==digest,'retired ring mutated'
                for phase in range(3 if valid else 1):
                    with (case/'commands.bin.control').open('rb') as f,mmap.mmap(f.fileno(),576,access=mmap.ACCESS_READ) as m:
                        assert struct.unpack_from('<I',m,24)[0]==2 and struct.unpack_from('<I',m,40)[0]==1
                records=[]
                if valid:
                    for phase in range(1,4):
                        rec=mutation.archive((capture/f'session-{phase+1:08x}.bin').read_bytes());ops=[x['op'] for x in rec];first_present=ops.index(6)
                        assert (phase!=3 or ops[-1]==8) and ops[:first_present].count(1)==2 and not any(op in [2,3,11] for op in ops[:first_present])
                        assert all(op in ops[first_present+1:] for op in [2,3,11])
                        if mode=='indexed':assert ops[:first_present].count(12)==1 and ops[:first_present].count(14)==2 and ops.count(13)==1
                        records.append(dict(session=123+phase,complete_resources_before_present=True,archive_complete=ops[-1]==8,opcodes={str(op):ops.count(op) for op in set(ops)}))
                counts=struct.unpack('<10I',(case/'engine-counts.bin').read_bytes());assert counts[0]==counts[1] and counts[2:6]==(2 if mode=='uncertain' else 1,1,1,1),(mode,counts)
                if valid:assert counts==(14,14,1,1,1,1,12,6,9 if mode=='indexed' else 0,0),(mode,counts)
                cs_counts=struct.unpack('<II',(case/'checkpoint-cs-counts.bin').read_bytes());assert cs_counts==((31,31) if mode=='capacity' else (2,2) if mode=='budget' else (0,0))
                report['cases'].append(dict(mode=mode,success=True,valid=valid,original_counts=list(counts),consumer=observed,checkpoints=records,immutable_old_rings=len(frozen)))
                print(mode+': passed',flush=True)
            finally:
                for process in [wine,qt]:
                    if process and process.poll() is None:process.terminate();process.wait(timeout=5)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Source changed during execution'
    report.update(success=True,full_frame_comparisons=sum(len(c['consumer']['frames']) for c in report['cases']));(run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
