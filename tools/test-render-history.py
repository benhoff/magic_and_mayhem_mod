#!/usr/bin/env python3
"""Synthetic x86 surface aliases, writable locks, Release and fail-closed history."""
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
    parent=REPO/'working/tests/render-history';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));frame=root/'frame.bin'
    with frame.open('wb') as f:
        f.write(b'MNMGL001'+struct.pack('<II',1,64)+bytes(48));f.truncate(64+2048*2048*4)
    shutil.copy2(dll,root/dll.name)
    stage=load('stage','tools/prepare-shadow-experiment.py')
    (root/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    build=REPO/'working/build/renderer'
    subprocess.run(['cmake','-S',str(REPO/'renderer'),'-B',str(build)],check=True)
    subprocess.run(['cmake','--build',str(build),'--target','mnm-render-commands','--parallel','4'],check=True)
    env=os.environ.copy();env.pop('MNM_PALETTE_SELFTEST',None);env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',
        MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_HISTORY='1',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    qt_build=REPO/'working/build/qt-shell'
    subprocess.run(['cmake','-S',str(REPO/'apps/qt-shell'),'-B',str(qt_build)],check=True)
    subprocess.run(['cmake','--build',str(qt_build),'--target','mnm-qt-shell','--parallel','4'],check=True)
    reports=[]
    for mode in ('ok','unlock','lock','budget','partial','flip','reentrant','gap','surface-restore','change-palette','dc','terminated-lock','detach','external-alias'):
        run=root/mode;run.mkdir();env['MNM_RENDER_CAPTURE_DIR']='Z:'+str(run).replace('/','\\');env['MNM_HISTORY_SELFTEST']=mode
        with (run/'wine.log').open('w') as log:
            subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=60)
        commands=records((run/'history-0001.bin').read_bytes());valid=mode in ('ok','unlock','lock','budget','detach','external-alias')
        result=subprocess.run(['xvfb-run','-a',str(build/'mnm-render-commands'),str(run/'history-0001.bin'),
            '--output',str(run/'native.bin'),'--preview',str(run/'preview.png')],env=env,capture_output=True,text=True,timeout=20)
        report=json.loads(result.stdout);assert result.returncode==(0 if valid else 2),(mode,report,result.stderr)
        creates=[struct.unpack_from('<I',p)[0] for op,p in commands if op==1]
        destroys=[struct.unpack_from('<I',p)[0] for op,p in commands if op==7]
        if valid:
            assert creates==[1,2,3] and sorted(destroys)==[1,2,3],(mode,creates,destroys)
            assert commands[-1]==(8,b'')
            updates=sum(op==2 for op,p in commands);copies=sum(op==3 for op,p in commands)
            assert updates==(0 if mode=='lock' else 1 if mode=='external-alias' else 2) and copies==(14 if mode=='budget' else 3 if mode=='external-alias' else 4)
            first=0xffff if mode=='lock' else 0xf81f
            expected=struct.pack('<12H',0x1f,0x1f,0x1f,0x1f,0x1f,first,0,0xf800,0x1f,0x7e0,0,0xffff)
            assert (run/'native.bin').read_bytes()==expected,mode
            assert report['checks']==copies*3+updates and report['presentations']==copies
            assert report['surface_stats']['uploads']==3+updates and report['surface_stats']['surfaces']==0
        else:
            assert not (run/'native.bin').exists() and not (run/'preview.png').exists()
            if mode=='gap':assert 'disagree' in report['error'] and commands[-1]==(8,b'')
            else:
                reason={'partial':3,'flip':6,'reentrant':1,'surface-restore':6,'change-palette':6,'dc':6,'terminated-lock':3}[mode]
                assert commands[-1]==(9,struct.pack('<I',reason)) and 'gap' in report['error']
        if valid or mode=='partial':
            qt=subprocess.run(['xvfb-run','-a',str(qt_build/'mnm-qt-shell'),'--commands',
                str(run/'history-0001.bin'),'--smoke-test'],env=env,capture_output=True,text=True,timeout=20)
            assert qt.returncode==(0 if valid else 8),(mode,qt.stderr)
            if not valid:assert 'gap' in qt.stderr
        reports.append({'qt_readback':valid,'mode':mode,'valid':valid,'replay':report})
        print(f'History fixture {mode}: passed',flush=True)
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_x86_surface_history','architecture':'PE32 i386',
        'dll_sha256':hashlib.sha256((root/dll.name).read_bytes()).hexdigest(),
        'executable_sha256':hashlib.sha256((root/'selftest.exe').read_bytes()).hexdigest(),'fixtures':reports},indent=2)+'\n')
    print(f'Surface history passed: {root}')


if __name__=='__main__':main()
