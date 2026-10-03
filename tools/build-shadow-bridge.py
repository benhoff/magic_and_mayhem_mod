#!/usr/bin/env python3
"""Build a freestanding Windows i386 logging bridge using Clang/LLVM."""
import hashlib
import json
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parent.parent
IMPORTS={"GetModuleHandleA":4,"VirtualAlloc":16,"VirtualProtect":16,"VirtualQuery":12,
         "FlushInstructionCache":12,"GetCurrentProcess":0,"GetProcessHeap":0,"HeapAlloc":12,
         "HeapFree":12,"CreateFileA":28,"WriteFile":20,"CloseHandle":4,"GetLastError":0,
         "SetLastError":4,"GetEnvironmentVariableA":12,"ExitProcess":4}
def build(selftest=False):
    root=REPO/('working/build/shadow-selftest' if selftest else 'working/build/shadow');root.mkdir(parents=True,exist_ok=True)
    source=REPO/'runtime/shadow'
    definition=root/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in IMPORTS.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(root/'kernel32.lib'),'--kill-at'],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
           '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(source)]
    if selftest: flags+=['-DMNM_SHADOW_SELFTEST']
    subprocess.run(flags+['-c',str(source/'bridge.c'),'-o',str(root/'bridge.obj')],check=True)
    subprocess.run(flags+['-c',str(source/'bridge.S'),'-o',str(root/'stub.obj')],check=True)
    exports=['/export:ShadowInstallForTest=_ShadowInstallForTest@4'] if selftest else []
    subprocess.run(['lld-link',*exports,'/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib',
                    '/safeseh:no','/timestamp:0',f'/out:{root / "MnmShadow.dll"}',str(root/'bridge.obj'),
                    str(root/'stub.obj'),str(root/'kernel32.lib')],check=True)
    dll=root/'MnmShadow.dll'
    (root/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','scope':'logging-only neighbor expansion',
        'sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in source.iterdir() if p.is_file()}},indent=2)+'\n')
    if selftest:
        definition=root/'shadow.def'
        definition.write_text('LIBRARY MnmShadow.dll\nEXPORTS\nShadowInstallForTest@4\n')
        subprocess.run(['llvm-dlltool','-m','i386','-D','MnmShadow.dll','-d',str(definition),
                        '-l',str(root/'shadow.lib'),'--kill-at'],check=True)
        for name in ('selftest.c','selftest.S'):
            subprocess.run(flags+['-c',str(source/name),'-o',str(root/(name+'.obj'))],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000',
                        '/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root / "selftest.exe"}',
                        str(root/'selftest.c.obj'),str(root/'selftest.S.obj'),str(root/'shadow.lib'),
                        str(root/'kernel32.lib')],check=True)
    return dll
if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--selftest',action='store_true')
    print(build(parser.parse_args().selftest))
