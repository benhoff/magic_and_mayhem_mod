#!/usr/bin/env python3
"""Synthetic PE32/Wine owned hooks publish incrementally into a live Qt GPU consumer."""
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
import time
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1,render_commands_v1 as protocol

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def create(path,header,size):
    with path.open('xb') as f:f.write(header);f.truncate(size)

def wait_ready(path,process):
    deadline=time.monotonic()+15
    while not path.exists():
        if process.poll() is not None:raise AssertionError('Qt exited before ready')
        if time.monotonic()>deadline:raise AssertionError('Qt startup timeout')
        time.sleep(.02)

def original_frames(case,indexed):
    expected=[];previous=0
    if indexed:
        events=struct.iter_unpack('<II',(case/'events.bin').read_bytes())
        for step,count in events:
            if count>previous:
                native=(case/f'original-{step:08x}.bin').read_bytes();colors=(case/f'colors-{step:08x}.bin').read_bytes()
                expected.append([0xff000000|int.from_bytes(colors[i*4:i*4+3],'big') for i in native])
            previous=count
    else:
        events=struct.iter_unpack('<6I',(case/'session-events.bin').read_bytes())
        for step,op,subject,status,count,partial in events:
            if count>previous:
                native=(case/f'front-{step:08x}.bin').read_bytes();pixels=[]
                for (v,) in struct.iter_unpack('<H',native):pixels.append(0xff000000|((v>>11)*255//31)<<16|(((v>>5)&63)*255//63)<<8|(v&31)*255//31)
                expected.append(pixels)
            previous=count
    return [hashlib.sha256(b''.join(struct.pack('<I',v) for v in pixels)).hexdigest() for pixels in expected]

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--sanitized-build',type=Path);args=parser.parse_args()
    sources=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['runtime/shadow/win32_min.h','tools/build-render-bridge.py','tools/test-live-render-channel.py','tools/prepare-shadow-experiment.py','renderer/commands.cpp','renderer/commands.hpp','renderer/command_state.hpp','renderer/command_consumer.cpp','renderer/blit.cpp','renderer/blit.hpp','renderer/CMakeLists.txt','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','tests/live-render-channel-test.cpp','tests/render-command-writer-test.c','protocols/schemas/render_commands-v1.json','protocols/include/mnm/render_commands_v1.h','protocols/python/mnm_protocols/render_commands_v1.py','protocols/generate.py']]
    hashes={str(p.relative_to(ROOT)):sha(p) for p in sources};parent=ROOT/'working/tests/live-render-channel';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0')
    report=dict(schema=1,scope='Synthetic Wine PE32 owned hooks to live Qt native GPU consumer; original game/rendering replacement untested.',source_sha256=hashes,run_directory=str(run.relative_to(ROOT)),synthetic=[],wine=[])
    for label,build in [('normal',args.build),('sanitized',args.sanitized_build)]:
        if build is None:continue
        result=subprocess.run([str(build.resolve()/'live-render-channel-test')],env=env,capture_output=True,text=True,timeout=45);(run/f'{label}.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr;report['synthetic'].append(json.loads(result.stdout))
    writer=run/'writer-test';subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(ROOT/'tests/render-command-writer-test.c'),'-o',str(writer)],check=True)
    result=subprocess.run([str(writer)],env=env,capture_output=True,text=True,check=True);report['writer']=json.loads(result.stdout)
    dll=load('live_render_build','tools/build-render-bridge.py').build(True);stage=load('live_render_stage','tools/prepare-shadow-experiment.py')
    for mode in ['mixed','failed','alias','indexed','restore','held','cancelled']:
        case=run/mode;capture=case/'capture';capture.mkdir(parents=True);frame=case/'frame.bin';channel=case/'commands.bin';header=bytearray(protocol.initial_header());struct.pack_into('<I',header,16,100+len(report['wine']));create(channel,header,protocol.SIZE);create(frame,frame_v1.initial_header(),frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name);original=(dll.parent/'selftest.exe').read_bytes();(case/'selftest.exe').write_bytes(stage.add_import(original,dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        active=case/'producer.active';active.write_text('Wine producer has not returned');output=case/'qt.json';qt_env=env.copy();qt_env.update(WINEPREFIX=str(ROOT/'working/tests/render-wine'),WINEDEBUG='-all',MNM_RENDER_OWNED_SESSION='1',MNM_COMMAND_TEST_DELAY='1',MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        if mode=='indexed':qt_env['MNM_BOOTSTRAP_SELFTEST']='idxflip-rotate'
        else:qt_env['MNM_OWNED_SESSION_SELFTEST']='mixed' if mode=='cancelled' else mode
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            qt=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],env=env,stdout=qlog,stderr=qlog)
            try:
                wait_ready(Path(str(output)+'.ready'),qt)
                if mode=='cancelled':
                    with channel.open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=qt_env,stdout=wlog,stderr=wlog)
                active.write_text(str(wine.pid));wine_code=wine.wait(timeout=45);active.unlink();assert wine_code==0,f'Wine {mode}: {wine_code}'
                code=qt.wait(timeout=35)
            finally:
                if qt.poll() is None:qt.terminate();qt.wait(timeout=5)
        observed=json.loads(output.read_text());valid=mode not in ['restore','held','cancelled'];assert observed['success']==valid and code==(0 if valid else 8),(mode,observed)
        if valid:
            expected=original_frames(case,mode=='indexed');actual=observed['frames'];assert actual==expected,f'{mode}: live GPU frame hashes differ ({len(actual)} observed, {len(expected)} expected)'
            assert observed['before_producer_exit'] and observed['native_readbacks']==observed['rgba_readbacks']==observed['viewport_uploads']==observed['live_surfaces']==0
            with channel.open('rb') as f:control=f.read(64);length=struct.unpack_from('<I',control,20)[0];published=f.read(length)
            assert published==(capture/'session-00000001.bin').read_bytes()
        else:assert observed.get('live_surfaces',0)==0
        report['wine'].append(dict(mode=mode,valid_session=valid,**observed));print(f'Live command fixture {mode}: passed',flush=True)
    assert all(sha(ROOT/p)==h for p,h in hashes.items()),'Source changed during execution'
    report['success']=True;report['native_integration']=dict(all_match=True,wine_sessions=len(report['wine']),valid_wine_sessions=4,incomplete_sessions=3,before_producer_exit=True,ordinary_readbacks=0,viewport_uploads=0,fragmented_decode=True,normal_and_sanitized=args.sanitized_build is not None)
    path=run/'report.json';path.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,report=str(path.relative_to(ROOT)))))
if __name__=='__main__':main()
