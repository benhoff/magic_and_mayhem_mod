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
    env=os.environ.copy();env['WINEPREFIX']=str(REPO/'working/tests/render-wine');env['WINEDEBUG']='-all'
    env.pop('MNM_RENDER_CAPTURE_DIR',None)
    env['MNM_RENDER_STREAM']='Z:'+str(stream).replace('/','\\')
    with (root/'wine.log').open('w') as log:
        subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
    data=stream.read_bytes();sequence,width,height,pitch,format,status,count=struct.unpack_from('<7I',data,16)
    expected=bytes([255,0,0,255])*2+bytes([0,255,0,255])*2+bytes([0,0,255,255])*2+bytes([255,255,255,255])*2
    assert (sequence,width,height,pitch,format,status,count)==(4,4,2,16,1,1,2)
    assert data[64:96]==expected
    replay=load('render_replay','tools/replay-render-capture.py')
    renderer_build=REPO/'working/build/renderer'
    subprocess.run(['cmake','-S',str(REPO/'renderer'),'-B',str(renderer_build)],check=True)
    subprocess.run(['cmake','--build',str(renderer_build),'--target','mnm-render-replay','--parallel','4'],check=True)
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
        draw_reports.append({'operation':captured['operation'],'comparison':comparison,'inventory':inventory,
                             'opengl_replay_matches_capture_and_cpu':True})
    subprocess.run(['cmake','-S',str(REPO/'apps/qt-shell'),'-B',str(REPO/'working/build/qt-shell')],check=True)
    subprocess.run(['cmake','--build',str(REPO/'working/build/qt-shell'),'--parallel','4'],check=True)
    env['QT_QPA_PLATFORM']='xcb';env['LIBGL_ALWAYS_SOFTWARE']='1'
    subprocess.run(['xvfb-run','-a',str(REPO/'working/build/qt-shell/mnm-qt-shell'),'--stream-test',str(stream)],env=env,check=True,timeout=15)
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_wine_opengl','frames':count,'pixel_bytes_match':True,
        'qt_opengl_readback_match':True,'draw_captures':draw_reports},indent=2)+'\n')
    print(f'Wine-to-Qt/OpenGL frame bridge passed: {root}')
if __name__=='__main__':main()
