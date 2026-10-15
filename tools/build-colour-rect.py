#!/usr/bin/env python3
"""Build fixed-build native colour rectangles without Qt, CRT or capture I/O."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['runtime/canvas-writers/colour_rect_bridge.c', 'runtime/canvas-writers/colour_rect_entry.S',
           'reconstruction/rendering/colour_rect.c', 'reconstruction/rendering/pixel_kernel.c']


def build():
    out = ROOT/'working/build/colour-rect'
    out.mkdir(parents=True,exist_ok=True)
    spec = importlib.util.spec_from_file_location('map_imports',ROOT/'tools/build-shadow-bridge.py')
    imports = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(imports)
    definition = out/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in
        {**imports.IMPORTS,'GetCurrentThreadId':0,'GetProcAddress':8}.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',out/'kernel32.lib','--kill-at'],check=True)
    objects,closure = [],set(SOURCES+['tools/build-colour-rect.py','tools/build-shadow-bridge.py'])
    for name in SOURCES:
        obj = out/(Path(name).name+'.obj')
        dep = obj.with_suffix('.d')
        subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
            '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MD','-MF',dep,'-c',ROOT/name,'-o',obj],check=True)
        for token in dep.read_text().replace('\\\n',' ').split():
            p = Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                closure.add(p.relative_to(ROOT).as_posix())
        objects.append(obj)
    dll = out/'MnmColourRect.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',
                    '/out:'+str(dll),*objects,out/'kernel32.lib'],check=True)
    (out/'manifest.json').write_text(json.dumps(dict(architecture='PE32 i386',sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),
        sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(closure)}),indent=2)+'\n')
    return dll


if __name__=='__main__':
    print(build())
