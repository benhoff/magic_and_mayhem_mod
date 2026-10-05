#!/usr/bin/env python3
"""Build the opt-in PE32 menu observer; no game artifacts consumed by this build."""
import hashlib
import json
from pathlib import Path
import subprocess

REPO=Path(__file__).resolve().parents[1]
IMPORTS={'GetModuleHandleA':4,'VirtualAlloc':16,'VirtualProtect':16,'VirtualQuery':12,
         'FlushInstructionCache':12,'GetCurrentProcess':0,'CreateFileA':28,'WriteFile':20,
         'CloseHandle':4,'GetLastError':0,'SetLastError':4,'GetEnvironmentVariableA':12,
         'GetCurrentThreadId':0,'ExitProcess':4,'CreateFileMappingA':24,'MapViewOfFile':20,
         'UnmapViewOfFile':4,'GetFileSize':8,'GetTickCount':0,'Sleep':4}
def build(root=None,selftest=False,experimental_mini=False,fixture=True):
    root=root or REPO/'working/build/menu-observer';root.mkdir(parents=True,exist_ok=True)
    source=REPO/'runtime/menu'
    definition=root/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in IMPORTS.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(root/'kernel32.lib'),'--kill-at'],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
           '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(root)]
    if experimental_mini:flags.append('-DMNM_MENU_MINI_EXPERIMENTAL=1')
    subprocess.run(flags+(['-DMNM_MENU_SELFTEST'] if selftest else [])+['-c',str(source/'observer.c'),'-o',str(root/'observer.obj')],check=True)
    user_definition=root/'user32.def'
    user_definition.write_text('LIBRARY USER32.dll\nEXPORTS\nGetClientRect@8\n')
    subprocess.run(['llvm-dlltool','-m','i386','-D','USER32.dll','-d',str(user_definition),'-l',str(root/'user32.lib'),'--kill-at'],check=True)
    exports=['/export:MenuAnchor']
    if selftest:exports+=['/export:MenuInstallForTest=_MenuInstallForTest@0','/export:MenuCampaignInstallForTest=_MenuCampaignInstallForTest@0','/export:MenuCampaignNavigationForTest=_MenuCampaignNavigationForTest@0','/export:MenuCampaignWorldForTest=_MenuCampaignWorldForTest@0']
    dll=root/'MnmMenu.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/timestamp:0',
                    f'/out:{dll}',*exports,str(root/'observer.obj'),str(root/'kernel32.lib'),str(root/'user32.lib')],check=True)
    (root/'manifest.json').write_text(json.dumps({'sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'architecture':'PE32 i386','scope':'Menu callback/state observation; optional V1 Main/Quick, V2 Single Player/Map, V3 pre-battle spells V5 Quick Battle results or V6 Main Preferences / V7 fresh Region Entry / V8 original campaign Enter; V4 Mini separately gated','selftest':selftest,'experimental_mini':experimental_mini,
        'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [*source.iterdir(),REPO/'protocols/include/mnm/menu_v1.h',REPO/'protocols/include/mnm/menu_v2.h',REPO/'protocols/include/mnm/menu_v3.h',REPO/'protocols/include/mnm/menu_v4.h',REPO/'protocols/include/mnm/menu_v5.h',REPO/'protocols/include/mnm/menu_v6.h',REPO/'protocols/include/mnm/menu_v7.h',REPO/'protocols/include/mnm/menu_v8.h'] if p.is_file()}},indent=2)+'\n')
    if selftest:
        definition=root/'menu.def';definition.write_text('LIBRARY MnmMenu.dll\nEXPORTS\nMenuInstallForTest@0\nMenuCampaignInstallForTest@0\nMenuCampaignNavigationForTest@0\nMenuCampaignWorldForTest@0\n')
        subprocess.run(['llvm-dlltool','-m','i386','-D','MnmMenu.dll','-d',str(definition),'-l',str(root/'menu.lib'),'--kill-at'],check=True)
        if not fixture:return dll
        for name in ('contract_selftest.c','contract_selftest.S'):
            subprocess.run(flags+['-c',str(source/name),'-o',str(root/(name+'.obj'))],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/nodefaultlib',
                        '/safeseh:no','/timestamp:0',f'/out:{root/"selftest.exe"}',str(root/'contract_selftest.c.obj'),
                        str(root/'contract_selftest.S.obj'),str(root/'kernel32.lib'),str(root/'menu.lib')],check=True)
    return dll
if __name__=='__main__':print(build())
