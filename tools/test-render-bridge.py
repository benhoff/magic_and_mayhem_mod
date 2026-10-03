#!/usr/bin/env python3
"""Synthetic Wine DirectDraw hook -> shared RGBA stream -> Qt/OpenGL readback."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent

def load(name,file):
    spec=importlib.util.spec_from_file_location(name,REPO/file)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    dll=load('render_build','tools/build-render-bridge.py').build(True)
    parent=REPO/'working/tests/render';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));stream=root/'frame.bin'
    with stream.open('wb') as file:file.write(b'MNMGL001'+struct.pack('<2I',1,64)+bytes(48));file.truncate(64+2048*2048*4)
    shutil.copy2(dll,root/dll.name)
    stage=load('stage','tools/prepare-shadow-experiment.py')
    (root/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    env=os.environ.copy();env.pop('MNM_RENDER_LOCK_CAPTURE_DIR',None);env.pop('MNM_LOCK_LIFECYCLE_SELFTEST',None);env.pop('MNM_RENDER_NO_READBACK',None);env.pop('MNM_PRIMARY_LOCK_FAILURE_SELFTEST',None);env.pop('MNM_RENDER_FAILURE_LOG',None);env.pop("MNM_FLIP_SELFTEST",None);env['WINEPREFIX']=str(REPO/'working/tests/render-wine');env['WINEDEBUG']='-all'
    env.pop("MNM_STARTUP_SELFTEST",None);env.pop('MNM_RENDER_CAPTURE_DIR',None);env.pop('MNM_RENDER_HISTORY',None);env.pop('MNM_HISTORY_SELFTEST',None);env.pop('MNM_PALETTE_SELFTEST',None)
    env.pop('MNM_RENDER_MEDIA',None);env.pop('MNM_MEDIA_SELFTEST',None)
    env['MNM_RENDER_STREAM']='Z:'+str(stream).replace('/','\\')
    with (root/'wine.log').open('w') as log:
        subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
    data=stream.read_bytes();sequence,width,height,pitch,format,status,count=struct.unpack_from('<7I',data,16)
    expected=bytes([255,0,0,255])*2+bytes([0,255,0,255])*2+bytes([0,0,255,255])*2+bytes([255,255,255,255])*2
    assert (sequence,width,height,pitch,format,status,count)==(4,4,2,16,1,1,2)
    assert data[64:96]==expected
    failure_stream=root/'failure-frame.bin'
    with failure_stream.open('wb') as file:
        file.write(b'MNMGL001'+struct.pack('<2I',1,64)+bytes(48));file.truncate(64+2048*2048*4)
    failure_env=env.copy();failure_env['MNM_PRIMARY_LOCK_FAILURE_SELFTEST']='1'
    failure_env['MNM_RENDER_STREAM']='Z:'+str(failure_stream).replace('/','\\')
    failure_log=root/'surface-failures.log'
    failure_env['MNM_RENDER_FAILURE_LOG']='Z:'+str(failure_log).replace('/','\\')
    with (root/'failure-wine.log').open('w') as log:
        subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=failure_env,stdout=log,stderr=log,check=True,timeout=30)
    failure_lines=[line.split() for line in failure_log.read_text().splitlines()]
    assert len(failure_lines)==1 # Repeated identical failures are deduplicated.
    assert all(line[0]=='primary_lock' and line[1]=='887601ae' and int(line[2],16)>0 and
               line[4:] == ['0000000e','00004810'] for line in failure_lines)
    with failure_stream.open('rb') as file:failure_header=struct.unpack('<16I',file.read(64))
    assert failure_header[9]==2 and failure_header[10]==0
    passive_stream=root/'no-readback-frame.bin'
    with passive_stream.open('wb') as file:
        file.write(b'MNMGL001'+struct.pack('<2I',1,64)+bytes(48));file.truncate(64+2048*2048*4)
    passive_env=env.copy();passive_env['MNM_RENDER_NO_READBACK']='1'
    passive_env['MNM_RENDER_STREAM']='Z:'+str(passive_stream).replace('/','\\')
    with (root/'no-readback-wine.log').open('w') as log:
        subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=passive_env,stdout=log,stderr=log,check=True,timeout=30)
    with passive_stream.open('rb') as file:passive_header=struct.unpack('<16I',file.read(64))
    assert passive_header[10]==0
    startup_reports=[]
    for mode,state,result in (('ok',7,0),('failed',8,0x887600ff),('null-result',10,0)):
        diagnostic=root/('startup-'+mode+'.bin')
        with diagnostic.open('wb') as file:
            file.write(b'MNMGL001'+struct.pack('<2I',1,64)+bytes(48));file.truncate(64+2048*2048*4)
        probe_env=env.copy();probe_env['MNM_STARTUP_SELFTEST']=mode;probe_env['MNM_RENDER_STREAM']='Z:'+str(diagnostic).replace('/','\\')
        with (root/('startup-'+mode+'.log')).open('w') as log:
            subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=probe_env,stdout=log,stderr=log,check=True,timeout=60)
        header=struct.unpack('<16I',diagnostic.read_bytes()[:64])
        assert header[9:13]==(state,0,result,1),(mode,header)
        startup_reports.append({'mode':mode,'status':state,'hresult':result,'calls':1})
    replay=load('render_replay','tools/replay-render-capture.py')
    renderer_build=REPO/'working/build/renderer'
    subprocess.run(['cmake','-S',str(REPO/'renderer'),'-B',str(renderer_build)],check=True)
    subprocess.run(['cmake','--build',str(renderer_build),'--target','mnm-render-replay','mnm-render-commands','--parallel','4'],check=True)
    draw_reports=[]
    for mode in ('fast','blt'):
        draw=root/mode;draw.mkdir();env['MNM_RENDER_CAPTURE_DIR']='Z:'+str(draw).replace('/','\\')
        env['MNM_DRAW_SELFTEST_MODE']=mode
        # Reset the shared presentation stream for each new producer process.
        with stream.open('r+b') as file:file.seek(16);file.write(bytes(48))
        with (draw/'wine.log').open('w') as log:
            subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
        captured=replay.decode((draw/'blit-0001.bin').read_bytes())
        assert captured['operation']==('BltFast' if mode=='fast' else 'Blt')
        assert captured['key']==((0,0) if mode=='fast' else None)
        assert captured['source']==struct.pack('<6H',0xffff,0,0xf800,0x07e0,0,0xffff)
        # Independent expected bytes, including the untouched destination border.
        key_pixel=0x001f if mode=='fast' else 0
        assert captured['after']==struct.pack('<12H',0x001f,0x001f,0x001f,0x001f,
                                              0x001f,0xffff,key_pixel,0xf800,0x001f,0x07e0,key_pixel,0xffff)
        comparison=replay.compare(captured,replay.replay(captured));assert comparison['matching']
        inventory=replay.summarize_events((draw/'events.bin').read_bytes())
        assert inventory['events']==8 and inventory['counts']['Lock']==1 and inventory['counts']['Unlock']==1
        subprocess.run(['python3',str(REPO/'tools/replay-render-capture.py'),str(draw),'--backend','opengl','--headless',
                        '--gl-executable',str(renderer_build/'mnm-render-replay')],check=True)
        command_env=env.copy();command_env['QT_QPA_PLATFORM']='xcb';command_env['LIBGL_ALWAYS_SOFTWARE']='1'
        result=subprocess.run(['xvfb-run','-a',str(renderer_build/'mnm-render-commands'),str(draw/'commands-0001.bin'),
                               '--output',str(draw/'commands-native.bin'),'--preview',str(draw/'commands.png')],
                              env=command_env,capture_output=True,text=True,check=True,timeout=20)
        command_report=json.loads(result.stdout)
        assert (draw/'commands-native.bin').read_bytes()==captured['after']
        assert command_report['checks']==1 and command_report['commands']==8
        assert command_report['surface_stats']['uploads']==2 and command_report['surface_stats']['copies']==1
        assert command_report['surface_stats']['surfaces']==0
        draw_reports.append({'commands':command_report,'operation':captured['operation'],'comparison':comparison,'inventory':inventory,
                             'opengl_replay_matches_capture_and_cpu':True})
    subprocess.run(['cmake','-S',str(REPO/'apps/qt-shell'),'-B',str(REPO/'working/build/qt-shell')],check=True)
    subprocess.run(['cmake','--build',str(REPO/'working/build/qt-shell'),'--parallel','4'],check=True)
    env['QT_QPA_PLATFORM']='xcb';env['LIBGL_ALWAYS_SOFTWARE']='1'
    subprocess.run(['xvfb-run','-a',str(REPO/'working/build/qt-shell/mnm-qt-shell'),'--stream-test',str(stream)],env=env,check=True,timeout=15)
    for mode in ('fast','blt'):
        subprocess.run(['xvfb-run','-a',str(REPO/'working/build/qt-shell/mnm-qt-shell'),
                        '--commands',str(root/mode/'commands-0001.bin'),'--smoke-test'],env=env,check=True,timeout=15)
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_wine_opengl','frames':count,'startup_diagnostics':startup_reports,'primary_lock_failure_log':failure_lines,'no_readback_forwarding_passed':True,'pixel_bytes_match':True,
        'qt_opengl_readback_match':True,'qt_command_replay_readback_match':True,'draw_captures':draw_reports},indent=2)+'\n')
    print(f'Wine-to-Qt/OpenGL frame bridge passed: {root}')
if __name__=='__main__':main()
