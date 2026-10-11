#!/usr/bin/env python3
"""Build the independent opt-in PE32 native canvas writer splice DLL."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def build(selftest=False):
    spec=importlib.util.spec_from_file_location('writers_imports',ROOT/'tools/build-shadow-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    out=ROOT/('working/build/canvas-writers-selftest' if selftest else 'working/build/canvas-writers');out.mkdir(parents=True,exist_ok=True)
    definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in {**module.IMPORTS,'GetCurrentThreadId':0}.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(out/'kernel32.lib'),'--kill-at'],check=True)
    names=['runtime/canvas-writers/bridge.c','runtime/canvas-writers/fade_route.c','runtime/canvas-writers/fade_entry.S','runtime/canvas-writers/fade_finish.S','reconstruction/rendering/fade_kernel.c']
    objects=[]
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags.append('-DMNM_CANVAS_WRITER_SELFTEST')
    for name in names:
        obj=out/(Path(name).name+'.obj');subprocess.run([*flags,'-c',str(ROOT/name),'-o',str(obj)],check=True);objects.append(obj)
    dll=out/'MnmCanvasWriters.dll'
    exports=['/export:CanvasWritersInstallForTest=_CanvasWritersInstallForTest@0','/export:CanvasWritersRouteForTest=_CanvasWritersRouteForTest@4'] if selftest else []
    subprocess.run(['lld-link',*exports,'/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{dll}',*[str(o) for o in objects],str(out/'kernel32.lib')],check=True)
    paths=[*names,'runtime/canvas-writers/fade_route.h','runtime/shadow/win32_min.h','reconstruction/rendering/fade_kernel.h','tools/build-canvas-writers.py','tools/build-shadow-bridge.py']
    (out/'manifest.json').write_text(json.dumps(dict(architecture='PE32 i386',scope='Opt-in positive bounded original fade interior loop takeover under original Lock/Unlock',sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}),indent=2)+'\n')
    if selftest:
        definition=out/'writers.def';definition.write_text('LIBRARY MnmCanvasWriters.dll\nEXPORTS\nCanvasWritersInstallForTest@0\nCanvasWritersRouteForTest@4\n')
        subprocess.run(['llvm-dlltool','-m','i386','-D','MnmCanvasWriters.dll','-d',str(definition),'-l',str(out/'writers.lib'),'--kill-at'],check=True)
        subprocess.run([*flags,'-c',str(ROOT/'tests/canvas-writers-selftest.c'),'-o',str(out/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{out/"selftest.exe"}',str(out/'selftest.obj'),str(out/'writers.lib'),str(out/'kernel32.lib')],check=True)
    return dll
if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--selftest',action='store_true');args=parser.parse_args();print(build(args.selftest))
