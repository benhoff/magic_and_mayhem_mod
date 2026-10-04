#!/usr/bin/env python3
"""Independent indexed-color oracle for x86 palette hooks -> OpenGL -> Qt."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol



def load(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def oracle(records):
    images={};palettes={};last=None;checks=0
    for op,p in records:
        if op==1:
            sid,w,h,bits,*masks=struct.unpack_from('<7I',p);assert bits==8 and masks==[0,0,0]
            images[sid]=(w,h,bytearray(p[28:]));palettes[sid]=[bytes(3)]*256
        elif op==4:
            sid,first,count=struct.unpack_from('<3I',p)
            palettes[sid][first:first+count]=[p[12+i*3:15+i*3] for i in range(count)]
        elif op==2:
            sid,x,y,w,h=struct.unpack_from('<5I',p);width,height,pixels=images[sid]
            for row in range(h):pixels[(y+row)*width+x:(y+row)*width+x+w]=p[20+row*w:20+(row+1)*w]
        elif op==3:
            src,dst,l,t,r,b,x,y,keyed,key=struct.unpack('<10I',p);sw,sh,s=images[src];dw,dh,d=images[dst]
            for sy in range(t,b):
                for sx in range(l,r):
                    v=s[sy*sw+sx]
                    if not keyed or v!=key:d[(y+sy-t)*dw+x+sx-l]=v
        elif op in (5,10,6):
            sid=struct.unpack_from('<I',p)[0];w,h,pixels=images[sid]
            rgba=b''.join(palettes[sid][index]+b'\xff' for index in pixels)
            if op==5:assert p[4:]==pixels
            elif op==10:assert p[4:]==rgba;checks+=1
            else:last=(bytes(pixels),rgba)
        elif op==7:images.pop(struct.unpack('<I',p)[0])
        else:assert op==8 and not p
    assert not images and last is not None;return *last,checks


def main():
    history=load('history','tools/test-render-history.py');dll=history.load('build','tools/build-render-bridge.py').build(True)
    parent=REPO/'working/tests/render-palettes';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));frame=root/'frame.bin'
    with frame.open('wb') as f:f.write(frame_protocol.initial_header());f.truncate(frame_protocol.SIZE)
    shutil.copy2(dll,root/dll.name);stage=history.load('stage','tools/prepare-shadow-experiment.py')
    (root/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    build=REPO/'working/build/renderer';qt_build=REPO/'working/build/qt-shell'
    for source,target,executable in ((REPO/'renderer',build,'mnm-render-commands'),(REPO/'apps/qt-shell',qt_build,'mnm-qt-shell')):
        subprocess.run(['cmake','-S',str(source),'-B',str(target)],check=True)
        subprocess.run(['cmake','--build',str(target),'--target',executable,'--parallel','4'],check=True)
    env=os.environ.copy();env.pop("MNM_STARTUP_SELFTEST",None);env.pop("MNM_FLIP_SELFTEST",None);env.pop('MNM_HISTORY_SELFTEST',None)
    env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',MNM_RENDER_HISTORY='1',
        MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    reports=[]
    for mode in ('ok','failed-entries','assignment-failure','bad-flags','detach','unobserved','caps','entries-readback','palette-only','rebind','initialize'):
        run=root/mode;run.mkdir();env['MNM_RENDER_CAPTURE_DIR']='Z:'+str(run).replace('/','\\');env['MNM_PALETTE_SELFTEST']=mode
        with (run/'wine.log').open('w') as log:subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
        records=history.records((run/'history-0001.bin').read_bytes());valid=mode in ('ok','failed-entries','assignment-failure','palette-only','rebind')
        result=subprocess.run(['xvfb-run','-a',str(build/'mnm-render-commands'),str(run/'history-0001.bin'),
            '--output',str(run/'native.bin'),'--preview',str(run/'preview.png')],env=env,capture_output=True,text=True,timeout=20)
        report=json.loads(result.stdout);assert result.returncode==(0 if valid else 2),(mode,report,result.stderr)
        if valid:
            native,rgba,color_checks=oracle(records)
            assert native==bytes([2,1,2,3,3,1,2,3]) and (run/'native.bin').read_bytes()==native
            assert report['color_checks']==color_checks and color_checks>=7
            assert report['presentation_rgba_sha256']==hashlib.sha256(rgba).hexdigest()
            assert report['surface_stats']['uploads']==3 and report['surface_stats']['copies']==(6 if mode=='rebind' else 5)
            assert report['presentations']>report['surface_stats']['copies']
            if mode=='palette-only':
                last_copy=max(i for i,(op,p) in enumerate(records) if op==3)
                assert any(op==4 for op,p in records[last_copy+1:]) and any(op==6 for op,p in records[last_copy+1:])
            partial=[struct.unpack_from('<3I',p) for op,p in records if op==4 and struct.unpack_from('<I',p,4)[0]==1]
            if mode=='failed-entries':assert not partial
            elif mode=='ok':assert partial==[(1,1,2),(2,1,2),(1,1,2),(1,1,2),(2,1,2)],partial
            subprocess.run(['xvfb-run','-a',str(qt_build/'mnm-qt-shell'),'--commands',str(run/'history-0001.bin'),'--smoke-test'],env=env,check=True,timeout=20)
        else:
            assert records[-1]==(9,struct.pack('<I',5)) and 'gap' in report['error']
            assert not (run/'native.bin').exists()
        reports.append({'mode':mode,'valid':valid,'replay':report});print(f'Palette fixture {mode}: passed',flush=True)
    # RGBA comparison is evidence only; poisoning it must fail, never recolor GPU inputs.
    raw=bytearray((root/'ok/history-0001.bin').read_bytes());at=16
    while at<len(raw):
        op,seq,size=struct.unpack_from('<III',raw,at)
        if op==10:raw[at+16]^=0x80;break
        at+=12+size
    poison=root/'poison.bin';poison.write_bytes(raw)
    result=subprocess.run(['xvfb-run','-a',str(build/'mnm-render-commands'),str(poison),'--output',str(root/'poison-native.bin')],env=env,capture_output=True,text=True,timeout=20)
    assert result.returncode==2 and 'RGBA colors disagree' in json.loads(result.stdout)['error'] and not (root/'poison-native.bin').exists()
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_x86_indexed_palettes','fixtures':reports,'poisoned_rgba_rejected':True,
        'architecture':'PE32 i386','dll_sha256':hashlib.sha256((root/dll.name).read_bytes()).hexdigest(),
        'executable_sha256':hashlib.sha256((root/'selftest.exe').read_bytes()).hexdigest()},indent=2)+'\n')
    print(f'Palette histories passed: {root}')


if __name__=='__main__':main()
