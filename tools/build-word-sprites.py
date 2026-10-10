#!/usr/bin/env python3
"""Build the freestanding, scoped direct-word sprite backend adapter."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
SOURCES=['renderer/sprites/word_raster.h','renderer/sprites/word_raster.c',
         'runtime/scene/word_route.c','runtime/scene/word_entry.S',
         'runtime/scene/word_workspace.h',
         'runtime/shadow/win32_min.h','tools/build-word-sprites.py']

def build():
    spec=importlib.util.spec_from_file_location('word_imports',ROOT/'tools/build-shadow-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    out=ROOT/'working/build/word-sprites';out.mkdir(parents=True,exist_ok=True)
    definition=out/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in module.IMPORTS.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(out/'kernel32.lib'),'--kill-at'],check=True)
    objects=[]
    for name in ['renderer/sprites/word_raster.c','runtime/scene/word_route.c','runtime/scene/word_entry.S']:
        obj=out/(Path(name).name+'.obj');objects.append(str(obj))
        subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
            '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
            '-c',str(ROOT/name),'-o',str(obj)],check=True)
    dll=out/'MnmWord.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib',
        '/safeseh:no','/timestamp:0',f'/out:{dll}',*objects,str(out/'kernel32.lib')],check=True)
    (out/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','scope':'Selected direct-word backend entries only',
        'sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}},indent=2)+'\n')
    return dll

if __name__=='__main__':print(build())
