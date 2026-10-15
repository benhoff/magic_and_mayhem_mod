#!/usr/bin/env python3
"""Build the isolated, configured PE32 controls adapter without game media."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from rts_controls_config import ROOT, DEFAULT, load


def build(output=None, config=DEFAULT):
    output = Path(output or ROOT/'working/build/rts-controls')
    output.mkdir(parents=True, exist_ok=True)
    settings, entries = load(config)
    (output/'settings.h').write_text(
        f'#define RTS_CTRL_ADDITIVE {int(settings["ctrl_additive_selection"])}\n'
        f'#define RTS_DOUBLE_TAP_MS {settings["double_tap_ms"]}u\n'
        'static const unsigned int bindings[][4] = {\n'+
        ''.join(' {'+','.join(str(n)+'u' for n in row)+'},\n' for row in entries)+'};\n')
    imports = {'GetModuleHandleA':4, 'VirtualProtect':16, 'VirtualQuery':12,
               'GetLastError':0, 'SetLastError':4, 'GetEnvironmentVariableA':12,
               'GetCurrentThreadId':0, 'GetTickCount':0, 'VirtualAlloc':16,
               'FlushInstructionCache':12, 'GetCurrentProcess':0}
    definition = output/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+
                          ''.join(f'{k}@{n}\n' for k,n in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),
                    '-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    flags = ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
             '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
             '-I',str(output)]
    sources = ['runtime/rts-controls/bridge.c','game/input/rts_controls.c']
    objects = []
    for source in sources:
        obj = output/(Path(source).name+'.obj')
        subprocess.run([*flags,'-c',str(ROOT/source),'-o',str(obj)],check=True)
        objects.append(obj)
    dll = output/'MnmRtsControls.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib',
                    '/timestamp:0',f'/out:{dll}','/export:RtsControlsAnchor',
                    *map(str,objects),str(output/'kernel32.lib')],check=True)
    sources += ['game/input/rts_controls.h','runtime/shadow/win32_min.h',
                'tools/build-rts-controls.py','tools/rts_controls_config.py']
    (output/'manifest.json').write_text(json.dumps({
        'sha256':hashlib.sha256(dll.read_bytes()).hexdigest(), 'settings':settings,
        'config_sha256':hashlib.sha256(Path(config).read_bytes()).hexdigest(),
        'sources':{s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in sources},
        'scope':'Intentional campaign controls policy; original rendering/simulation retained; live playtest pending.'
    },indent=2)+'\n')
    return dll


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path)
    p.add_argument('--config',type=Path,default=DEFAULT)
    a = p.parse_args()
    print(build(a.output,a.config))
