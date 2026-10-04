#!/usr/bin/env python3
"""Build isolated x86 DirectSound voice adapter and COM fixture; no game patching."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
REPO = Path(__file__).resolve().parents[1]

def build(selftest=False):
    spec = importlib.util.spec_from_file_location('shadow_build', REPO/'tools/build-shadow-bridge.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    imports = dict(module.IMPORTS, CreateFileMappingA=24, MapViewOfFile=20, GetFileSize=8, GetTickCount=0, Sleep=4)
    root = REPO/('working/build/audio-bridge-selftest' if selftest else 'working/build/audio-bridge')
    root.mkdir(parents=True, exist_ok=True)
    definition = root/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')],check=True)
    flags = ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest: flags += ['-DMNM_AUDIO_SELFTEST']
    source = REPO/'runtime/audio'
    subprocess.run(flags+['-c',str(source/'bridge.c'),'-o',str(root/'bridge.obj')],check=True)
    dll = root/'MnmAudio.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/export:AudioCreateForTest=_AudioCreateForTest@4',f'/out:{dll}',str(root/'bridge.obj'),str(root/'kernel32.lib')],check=True)
    (root/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in source.iterdir() if p.is_file()}},indent=2)+'\n')
    if selftest:
        definition = root/'audio.def'
        definition.write_text('LIBRARY MnmAudio.dll\nEXPORTS\nAudioCreateForTest@4\n')
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'audio.lib')],check=True)
        subprocess.run(flags+['-c',str(source/'selftest.c'),'-o',str(root/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root/"selftest.exe"}',str(root/'selftest.obj'),str(root/'audio.lib'),str(root/'kernel32.lib')],check=True)
    return dll

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--selftest',action='store_true')
    print(build(parser.parse_args().selftest))
