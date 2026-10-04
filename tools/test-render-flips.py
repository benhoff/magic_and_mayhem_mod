#!/usr/bin/env python3
"""Synthetic x86 double-buffer flips checked against an independent CPU model."""
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


def records(data):
    assert data[:16]==b'MNMCMD01'+struct.pack('<II',1,16)
    at=16;out=[]
    while at<len(data):
        op,seq,size=struct.unpack_from('<III',data,at);at+=12
        assert seq==len(out)+1 and size<=len(data)-at
        out.append((op,data[at:at+size]));at+=size
    return out


def main():
    dll=load('build','tools/build-render-bridge.py').build(True)
    parent=REPO/'working/tests/render-flips';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));frame=root/'frame.bin'
    with frame.open('wb') as f:
        f.write(frame_protocol.initial_header());f.truncate(frame_protocol.SIZE)
    shutil.copy2(dll,root/dll.name)
    stage=load('stage','tools/prepare-shadow-experiment.py')
    (root/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    build=REPO/'working/build/renderer'
    subprocess.run(['cmake','-S',str(REPO/'renderer'),'-B',str(build)],check=True)
    subprocess.run(['cmake','--build',str(build),'--target','mnm-render-commands','--parallel','4'],check=True)
    env=os.environ.copy();env.pop("MNM_STARTUP_SELFTEST",None);env.pop("MNM_HISTORY_SELFTEST",None);env.pop('MNM_PALETTE_SELFTEST',None);env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',
        MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_HISTORY='1',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    qt_build=REPO/'working/build/qt-shell'
    subprocess.run(['cmake','-S',str(REPO/'apps/qt-shell'),'-B',str(qt_build)],check=True)
    subprocess.run(['cmake','--build',str(qt_build),'--target','mnm-qt-shell','--parallel','4'],check=True)
    reports=[]
    for mode in ('ok','old','target','failed','chain','stereo','masks','bad-rotation','post-lock'):
        run=root/mode;run.mkdir();env['MNM_RENDER_CAPTURE_DIR']='Z:'+str(run).replace('/','\\');env['MNM_FLIP_SELFTEST']=mode
        with (run/'wine.log').open('w') as log:
            subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
        commands=records((run/'history-0001.bin').read_bytes());valid=mode in ('ok','old','target','failed')
        result=subprocess.run(['xvfb-run','-a',str(build/'mnm-render-commands'),str(run/'history-0001.bin'),
            '--output',str(run/'native.bin'),'--preview',str(run/'preview.png')],env=env,capture_output=True,text=True,timeout=20)
        report=json.loads(result.stdout);assert result.returncode==(0 if valid else 2),(mode,report,result.stderr)
        if valid:
            # CPU oracle consumes only CREATE/UPDATE/COPY/SWAP inputs. CHECKs are
            # independent observations and must never become reconstruction inputs.
            surfaces={};last=None;swaps=0
            for op,p in commands:
                if op==1:
                    sid,w,h,bits,r,g,b=struct.unpack_from('<7I',p);assert bits==16
                    surfaces[sid]=(w,h,list(struct.unpack_from('<'+'H'*(w*h),p,28)))
                elif op==2:
                    sid,x,y,w,h=struct.unpack_from('<5I',p);dw,dh,pixels=surfaces[sid]
                    patch=struct.unpack_from('<'+'H'*(w*h),p,20)
                    for row in range(h):pixels[(y+row)*dw+x:(y+row)*dw+x+w]=patch[row*w:(row+1)*w]
                elif op==3:
                    src,dst,l,t,r,b,x,y,keyed,key=struct.unpack('<10I',p)
                    sw,sh,source=surfaces[src];dw,dh,dest=surfaces[dst]
                    for row in range(t,b):
                        for col in range(l,r):
                            value=source[row*sw+col]
                            if not keyed or value!=key:dest[(y+row-t)*dw+x+col-l]=value
                elif op==11:
                    a,b=struct.unpack('<2I',p);assert a!=b
                    aw,ah,ap=surfaces[a];bw,bh,bp=surfaces[b];assert (aw,ah)==(bw,bh)
                    surfaces[a]=(aw,ah,bp);surfaces[b]=(bw,bh,ap);swaps+=1
                elif op==5:
                    sid=struct.unpack_from('<I',p)[0];pixels=surfaces[sid][2]
                    assert p[4:]==struct.pack('<'+'H'*len(pixels),*pixels),(mode,op)
                elif op==6:
                    sid=struct.unpack('<I',p)[0];last=list(surfaces[sid][2])
                elif op==7:del surfaces[struct.unpack('<I',p)[0]]
                else:assert op==8
            assert swaps==3 and last and not surfaces and commands[-1]==(8,b'')
            expected=struct.pack('<12H',*last);assert (run/'native.bin').read_bytes()==expected
            assert last==[0xffe0,0x1f,0x1f,0x1f,0x1f,0xffff,0,0xf800,0x1f,0x7e0,0,0xffff]
            rgba=bytes(channel for value in last for channel in
                       (((value>>11)&31)*255//31,((value>>5)&63)*255//63,(value&31)*255//31,255))
            assert report['presentation_rgba_sha256']==hashlib.sha256(rgba).hexdigest()
            assert report['surface_stats']['uploads']==5 and report['surface_stats']['copies']==1
            assert report['presentations']==4 and report['surface_stats']['surfaces']==0
            qt=subprocess.run(['xvfb-run','-a',str(qt_build/'mnm-qt-shell'),'--commands',
                str(run/'history-0001.bin'),'--smoke-test'],env=env,capture_output=True,text=True,timeout=20)
            assert qt.returncode==0,(mode,qt.stderr)
        else:
            assert not (run/'native.bin').exists() and not (run/'preview.png').exists()
            if mode=='bad-rotation':assert 'disagree' in report['error']
            else:assert commands[-1]==(9,struct.pack('<I',6)) and 'gap' in report['error']
        reports.append({'mode':mode,'qt_readback':valid,'replay':report})
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_x86_double_buffer_flips','architecture':'PE32 i386',
        'dll_sha256':hashlib.sha256((root/dll.name).read_bytes()).hexdigest(),
        'executable_sha256':hashlib.sha256((root/'selftest.exe').read_bytes()).hexdigest(),'fixtures':reports},indent=2)+'\n')
    print(f'Double-buffer flip history passed: {root}')


if __name__=='__main__':main()
