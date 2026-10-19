#!/usr/bin/env python3
"""Build the PE32 DirectDraw frame capture bridge (Clang/LLVM, no CRT)."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
REPO=Path(__file__).resolve().parent.parent

def build(selftest=False,output=None):
    subprocess.run([sys.executable, str(REPO/"protocols/generate.py"), "--check"], check=True)
    spec=importlib.util.spec_from_file_location('shadow_build',REPO/'tools/build-shadow-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    imports=dict(module.IMPORTS,ReadFile=20,FreeLibrary=4,CreateFileMappingA=24,MapViewOfFile=20,UnmapViewOfFile=4,GetFileSize=8,GetFileInformationByHandle=8,GetTickCount=0,GetCurrentThreadId=0,GetProcAddress=8,Sleep=4,CreateThread=24,WaitForSingleObject=8,QueryPerformanceCounter=4,QueryPerformanceFrequency=4)
    root=Path(output).resolve() if output else REPO/('working/build/render-selftest' if selftest else 'working/build/render');root.mkdir(parents=True,exist_ok=True)
    definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')],check=True)
    gdi_definition=root/'gdi32.def';gdi_definition.write_text('LIBRARY GDI32.dll\nEXPORTS\nGetCurrentObject@8\nGetObjectA@12\nGetBitmapBits@12\nGdiFlush@0\n')
    if selftest:gdi_definition.write_text(gdi_definition.read_text()+'CreateCompatibleDC@4\nCreateDIBSection@24\nSelectObject@8\nDeleteObject@4\nDeleteDC@4\n')
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(gdi_definition),'-l',str(root/'gdi32.lib')],check=True)
    address_definition=root/'kernelbase.def';address_definition.write_text('LIBRARY KERNELBASE.dll\nEXPORTS\nWaitOnAddress@16\nWakeByAddressAll@4\n')
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(address_definition),'-l',str(root/'kernelbase.lib')],check=True)
    source=REPO/'runtime/render'
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags+=['-DMNM_RENDER_SELFTEST']
    subprocess.run(flags+['-MD','-MF',str(root/'bridge.d'),'-c',str(source/'bridge.c'),'-o',str(root/'bridge.obj')],check=True)
    exports=['/export:RenderInstallForTest=_RenderInstallForTest@8','/export:RenderCaptureGuardForTest=_RenderCaptureGuardForTest@4','/export:RenderCaptureWaitsForTest=_RenderCaptureWaitsForTest@0','/export:RenderCreateForTest=_RenderCreateForTest@16','/export:RenderInputForTest=_RenderInputForTest@12','/export:RenderMediaForTest=_RenderMediaForTest@12'] if selftest else []
    if selftest:exports+=['/export:RenderLifecyclePauseForTest=_RenderLifecyclePauseForTest@4','/export:RenderLifecycleGuardForTest=_RenderLifecycleGuardForTest@4','/export:RenderAliasSameForTest=_RenderAliasSameForTest@8','/export:RenderQueueForTest=_RenderQueueForTest@12','/export:RenderCommandAdmissionForTest=_RenderCommandAdmissionForTest@0','/export:RenderResidentHistoryForTest=_RenderResidentHistoryForTest@4','/export:RenderResidentBindHoldForTest=_RenderResidentBindHoldForTest@0','/export:RenderCallbackPublicationProbeForTest=_RenderCallbackPublicationProbeForTest@4']
    if selftest:exports+=['/export:RenderResidentReentryForTest=_RenderResidentReentryForTest@4','/export:RenderPublicationTraceForTest=_RenderPublicationTraceForTest@4']
    exports+=['/export:RenderRequireCpuHistoryV1=_RenderRequireCpuHistoryV1@20','/export:RenderOwnedInitialZeroV1=_RenderOwnedInitialZeroV1@12','/export:RenderPrepareStorageV1=_RenderPrepareStorageV1@4','/export:RenderResidentWorldV1=_RenderResidentWorldV1@32','/export:RenderStop=_RenderStop@4','/export:RenderShutdown=_RenderShutdown@4','/export:RenderStartup=_RenderStartup@0','/export:RenderRecover=_RenderRecover@4']
    if selftest:exports+=['/export:RenderStopStateForTest=_RenderStopStateForTest@4','/export:RenderRecoveryStorageForTest=_RenderRecoveryStorageForTest@4','/export:RenderExitInstallForTest=_RenderExitInstallForTest@4','/export:RenderRecoveryStateForTest=_RenderRecoveryStateForTest@4']
    if selftest:exports+=['/export:RenderPaletteStateForTest=_RenderPaletteStateForTest@12']
    if selftest:exports+=['/export:RenderDescriptionDomainForTest=_RenderDescriptionDomainForTest@8']
    if selftest:exports+=['/export:RenderNativeStatusProbeForTest=_RenderNativeStatusProbeForTest@4']
    dll=root/'MnmRender.dll'
    subprocess.run(['lld-link',*exports,'/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{dll}',str(root/'bridge.obj'),str(root/'kernel32.lib'),str(root/'kernelbase.lib'),str(root/'gdi32.lib')],check=True)
    closure={'tools/build-render-bridge.py','tools/build-shadow-bridge.py','protocols/generate.py'}
    for token in (root/'bridge.d').read_text().replace('\\\n',' ').split():
        path=Path(token).resolve()
        if path.is_file() and path.is_relative_to(REPO) and not path.is_relative_to(REPO/'working'):
            closure.add(path.relative_to(REPO).as_posix())
    protocol_headers=sorted((REPO/'protocols/include/mnm').glob('*_v1.h'))+sorted((REPO/'protocols/include/mnm').glob('*_v2.h'))+[REPO/'protocols/include/mnm/render_commands_v3.h',REPO/'protocols/include/mnm/render_stream_v3.h',REPO/'protocols/include/mnm/render_stream_v4.h',REPO/'protocols/include/mnm/render_stream_v5.h',REPO/'protocols/include/mnm/native_cpu_read_v1.h',REPO/'protocols/include/mnm/render_command_ring.h']
    protocol_sources=protocol_headers+sorted((REPO/'protocols/schemas').glob('*-v1.json'))+sorted((REPO/'protocols/schemas').glob('*-v2.json'))+[REPO/'protocols/schemas/render_commands-v3.json',REPO/'protocols/generate.py']
    (root/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in source.iterdir() if p.is_file()},
        'compiled_sources':{name:hashlib.sha256((REPO/name).read_bytes()).hexdigest() for name in sorted(closure)},
        'protocol_sources':{str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest() for p in protocol_sources}},indent=2)+'\n')
    if selftest:
        definition=root/'render.def';definition.write_text('LIBRARY MnmRender.dll\nEXPORTS\nRenderInstallForTest@8\nRenderCaptureGuardForTest@4\nRenderCaptureWaitsForTest@0\nRenderCreateForTest@16\nRenderInputForTest@12\nRenderMediaForTest@12\nRenderStop@4\nRenderStopStateForTest@4\nRenderShutdown@4\nRenderStartup@0\nRenderRecover@4\nRenderRecoveryStateForTest@4\nRenderRecoveryStorageForTest@4\nRenderExitInstallForTest@4\nRenderQueueForTest@12\nRenderCommandAdmissionForTest@0\nRenderResidentHistoryForTest@4\nRenderAliasSameForTest@8\nRenderLifecycleGuardForTest@4\nRenderLifecyclePauseForTest@4\n')
        with definition.open('a') as exports:exports.write('RenderDescriptionDomainForTest@8\nRenderNativeStatusProbeForTest@4\n')
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'render.lib')],check=True)
        subprocess.run(flags+['-c',str(source/'selftest.c'),'-o',str(root/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root / "selftest.exe"}',str(root/'selftest.obj'),str(root/'render.lib'),str(root/'kernel32.lib'),str(root/'kernelbase.lib'),str(root/'gdi32.lib')],check=True)
    return dll
if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--selftest',action='store_true');parser.add_argument('--output',type=Path);args=parser.parse_args();print(build(args.selftest,args.output))
