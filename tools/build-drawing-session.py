#!/usr/bin/env python3
"""Build the original-active PE32 drawing/input session observer without game media."""
import argparse,hashlib,importlib.util,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def build(selftest=False):
    s=importlib.util.spec_from_file_location('shadow',ROOT/'tools/build-shadow-bridge.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
    root=ROOT/'working/build'/('drawing-session-selftest' if selftest else 'drawing-session');root.mkdir(parents=True,exist_ok=True)
    imports=dict(m.IMPORTS,GetProcAddress=8,GetCurrentThreadId=0,GetTickCount=0,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,VirtualFree=12,CreateThread=24,WaitForSingleObject=8,Sleep=4,ReadFile=20,GetFileSize=8,LoadLibraryA=4)
    definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags+=['-DMNM_DRAWING_SELFTEST','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'runtime/drawing/session.c'),'-o',str(root/'session.obj')],check=True)
    subprocess.run(flags+['-c',str(ROOT/'runtime/drawing/wine_blit_packet.S'),'-o',str(root/'packet.obj')],check=True)
    exports=['/export:DrawingAnchor=_DrawingAnchor@0','/export:DrawingActionV1=_DrawingActionV1@28']
    if selftest:exports+=['/export:DrawingInstallForTest=_DrawingInstallForTest@8']
    dll=root/'MnmDrawing.dll';subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',*exports,'/out:'+str(dll),str(root/'session.obj'),str(root/'packet.obj'),str(root/'kernel32.lib')],check=True)
    if selftest:
        definition=root/'drawing.def';definition.write_text('LIBRARY MnmDrawing.dll\nEXPORTS\nDrawingInstallForTest@8\nDrawingActionV1@28\n')
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'drawing.lib')],check=True)
        subprocess.run(flags+['-c',str(ROOT/'runtime/drawing/selftest.c'),'-o',str(root/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(root/'selftest.exe'),str(root/'selftest.obj'),str(root/'drawing.lib'),str(root/'kernel32.lib')],check=True)
    sources=['runtime/drawing/wine_blit_packet.h','runtime/drawing/wine_blit_packet.S','runtime/drawing/wine_blit_worker.h','runtime/drawing/wine_texture_entry.h','runtime/drawing/wine_buffer.h','runtime/drawing/wine_lifetime.h','runtime/drawing/wine_submission.h','runtime/drawing/source_points.h','runtime/drawing/call_intervals.h','runtime/drawing/physical_fence.h','runtime/drawing/session.c' ,'runtime/shadow/win32_min.h','tools/build-drawing-session.py','tools/build-shadow-bridge.py']
    (root/'manifest.json').write_text(json.dumps({'schema':1,'sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'sources':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources}},indent=2)+'\n');return dll
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--selftest',action='store_true');print(build(p.parse_args().selftest))
