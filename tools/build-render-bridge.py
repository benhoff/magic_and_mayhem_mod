#!/usr/bin/env python3
"""Build the PE32 DirectDraw frame capture bridge (Clang/LLVM, no CRT)."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
REPO=Path(__file__).resolve().parent.parent

def build(selftest=False):
    subprocess.run([sys.executable, str(REPO/"protocols/generate.py"), "--check"], check=True)
    spec=importlib.util.spec_from_file_location('shadow_build',REPO/'tools/build-shadow-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    imports=dict(module.IMPORTS,CreateFileMappingA=24,MapViewOfFile=20,UnmapViewOfFile=4,GetFileSize=8,GetFileInformationByHandle=8,GetTickCount=0,GetCurrentThreadId=0,GetProcAddress=8,Sleep=4,CreateThread=24,WaitForSingleObject=8,QueryPerformanceCounter=4,QueryPerformanceFrequency=4)
    root=REPO/('working/build/render-selftest' if selftest else 'working/build/render');root.mkdir(parents=True,exist_ok=True)
    definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')],check=True)
    gdi_definition=root/'gdi32.def';gdi_definition.write_text('LIBRARY GDI32.dll\nEXPORTS\nGetCurrentObject@8\nGetObjectA@12\nGetBitmapBits@12\nGdiFlush@0\n')
    if selftest:gdi_definition.write_text(gdi_definition.read_text()+'CreateCompatibleDC@4\nCreateDIBSection@24\nSelectObject@8\nDeleteObject@4\nDeleteDC@4\n')
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(gdi_definition),'-l',str(root/'gdi32.lib')],check=True)
    source=REPO/'runtime/render'
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags+=['-DMNM_RENDER_SELFTEST']
    subprocess.run(flags+['-c',str(source/'bridge.c'),'-o',str(root/'bridge.obj')],check=True)
    exports=['/export:RenderInstallForTest=_RenderInstallForTest@8','/export:RenderCaptureGuardForTest=_RenderCaptureGuardForTest@4','/export:RenderCaptureWaitsForTest=_RenderCaptureWaitsForTest@0','/export:RenderCreateForTest=_RenderCreateForTest@16','/export:RenderInputForTest=_RenderInputForTest@12','/export:RenderMediaForTest=_RenderMediaForTest@12'] if selftest else []
    if selftest:exports+=['/export:RenderQueueForTest=_RenderQueueForTest@12']
    exports+=['/export:RenderShutdown=_RenderShutdown@4','/export:RenderStartup=_RenderStartup@0','/export:RenderRecover=_RenderRecover@4']
    if selftest:exports+=['/export:RenderExitInstallForTest=_RenderExitInstallForTest@4','/export:RenderRecoveryStateForTest=_RenderRecoveryStateForTest@4']
    dll=root/'MnmRender.dll'
    subprocess.run(['lld-link',*exports,'/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{dll}',str(root/'bridge.obj'),str(root/'kernel32.lib'),str(root/'gdi32.lib')],check=True)
    protocol_headers=sorted((REPO/'protocols/include/mnm').glob('*_v1.h'))+[REPO/'protocols/include/mnm/render_commands_v2.h',REPO/'protocols/include/mnm/render_command_ring.h']
    protocol_sources=protocol_headers+sorted((REPO/'protocols/schemas').glob('*-v1.json'))+[REPO/'protocols/schemas/render_commands-v2.json',REPO/'protocols/generate.py']
    (root/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in source.iterdir() if p.is_file()},
        'protocol_sources':{str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest() for p in protocol_sources}},indent=2)+'\n')
    if selftest:
        definition=root/'render.def';definition.write_text('LIBRARY MnmRender.dll\nEXPORTS\nRenderInstallForTest@8\nRenderCaptureGuardForTest@4\nRenderCaptureWaitsForTest@0\nRenderCreateForTest@16\nRenderInputForTest@12\nRenderMediaForTest@12\nRenderShutdown@4\nRenderStartup@0\nRenderRecover@4\nRenderRecoveryStateForTest@4\nRenderExitInstallForTest@4\nRenderQueueForTest@12\n')
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'render.lib')],check=True)
        subprocess.run(flags+['-c',str(source/'selftest.c'),'-o',str(root/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root / "selftest.exe"}',str(root/'selftest.obj'),str(root/'render.lib'),str(root/'kernel32.lib'),str(root/'gdi32.lib')],check=True)
    return dll
if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--selftest',action='store_true');print(build(parser.parse_args().selftest))
