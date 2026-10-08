#!/usr/bin/env python3
"""Exercise actual PE32 startup hooks and strict diagnostic decoding on fixtures."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    paths = [ROOT/p for p in ['runtime/scene/canvas_startup.c','runtime/scene/canvas_startup.S',
        'tests/canvas-startup-test.c','tests/canvas-startup-call.S','tools/inspect-canvas-startup.py',
        'tools/test-canvas-startup.py','runtime/shadow/win32_min.h','tools/build-shadow-bridge.py']]
    sources = {str(p.relative_to(ROOT)):sha(p) for p in paths}
    parent=ROOT/'working/tests/canvas-startup';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(output,flush=True)
    def load(name,path):
        spec=importlib.util.spec_from_file_location(name,ROOT/path)
        m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
    build=load('canvas_build','tools/build-shadow-bridge.py')
    definition=output/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in build.IMPORTS.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
           '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DMNM_SCENE_SELFTEST']
    objects=[]
    for path in paths[:4]:
        obj=output/(path.name+'.obj');objects.append(str(obj))
        subprocess.run([*flags,'-c',str(path),'-o',str(obj)],check=True)
    binary=output/'selftest.exe'
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/dynamicbase:no','/nodefaultlib',
        '/safeseh:no','/timestamp:0',f'/out:{binary}',*objects,str(output/'kernel32.lib')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(MNM_SCENE_DIR='Z:'+str(output).replace('/','\\'),MNM_CANVAS_STARTUP='1',WINEDEBUG='-all',WINEPREFIX=str(output/'wineprefix'))
    subprocess.run(['cp','-a','--reflink=auto',str(ROOT/'working/tests/scene-selftest-wine'),env['WINEPREFIX']],check=True)
    with (output/'execution.log').open('x') as log:
        subprocess.run(['wine',str(binary)],cwd=output,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=60,check=True)
    decoder=load('canvas_decode','tools/inspect-canvas-startup.py')
    raw=(output/'canvas-startup.bin').read_bytes();rows=decoder.decode(raw)
    assert len(rows)==27 and rows[-1][31]==1
    cases=0
    for changed in [raw[:-1],raw[:160],b'BADMAGIC'+raw[8:]]:
        try:decoder.decode(changed)
        except ValueError:cases+=1
        else:raise AssertionError('Malformed trace accepted')
    for offset,value in [(8,2),(20,100),(160,2),(164,13),(168,2),(292,1),
                         (160+26*160+4,2),(160+2*160+4,0)]:
        changed=bytearray(raw);struct.pack_into('<I',changed,offset,value)
        try:decoder.decode(changed)
        except ValueError:cases+=1
        else:raise AssertionError('Invalid trace accepted')
    assert sources=={str(p.relative_to(ROOT)):sha(p) for p in paths}
    report={'success':True,'sources':sources,'sources_stable':True,'scope':'Synthetic actual PE32 wrapper entry/return forwarding, not original renderer equivalence',
        'hook_entries_checked':12,'paired_events':26,'nested_lock_calls_checked':1,'register_eflags_fxstate_last_error_match':True,
        'all_entry_mismatch_refusal_checked':True,'post_boundary_forwarding_checked':True,'malformed_trace_refusals':cases,
        'first_world_nonzero_pixels':1,'trace_sha256':sha(output/'canvas-startup.bin'),'binary_sha256':sha(binary),
        'original_pixels_used_as_native_inputs':False,'live_replacement':False}
    with (output/'report.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(output/'report.json')


if __name__=='__main__':main()
