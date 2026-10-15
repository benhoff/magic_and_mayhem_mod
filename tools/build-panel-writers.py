#!/usr/bin/env python3
"""Build the independent optional PE32 post-Lock native panel splices."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['runtime/canvas-writers/panel_bridge.c', 'runtime/canvas-writers/panel_route.c',
           'runtime/canvas-writers/panel_entry.S', 'reconstruction/rendering/panel_kernel.c']


def build(selftest=False):
    spec = importlib.util.spec_from_file_location('panel_imports', ROOT/'tools/build-shadow-bridge.py')
    imports = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(imports)
    out = ROOT/('working/build/panel-writers-selftest' if selftest else 'working/build/panel-writers')
    out.mkdir(parents=True, exist_ok=True)
    definition = out/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(
        f'{name}@{size}\n' for name, size in {**imports.IMPORTS, 'GetCurrentThreadId': 0}.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', str(definition),
                    '-l', str(out/'kernel32.lib'), '--kill-at'], check=True)
    flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
             '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror']
    if selftest:
        flags.append('-DMNM_PANEL_WRITER_SELFTEST')
    objects = []
    for name in SOURCES:
        obj = out/(Path(name).name+'.obj')
        subprocess.run([*flags, '-c', str(ROOT/name), '-o', str(obj)], check=True)
        objects.append(str(obj))
    exports = ['/export:PanelWritersInstallForTest=_PanelWritersInstallForTest@0'] if selftest else []
    dll = out/'MnmPanelWriters.dll'
    subprocess.run(['lld-link', *exports, '/dll', '/machine:x86', '/entry:DllMain@12',
                    '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:'+str(dll),
                    *objects, str(out/'kernel32.lib')], check=True)
    paths = SOURCES + ['runtime/canvas-writers/panel_route.h', 'reconstruction/rendering/panel_kernel.h',
                       'runtime/shadow/win32_min.h', 'tools/build-panel-writers.py', 'tools/build-shadow-bridge.py']
    (out/'manifest.json').write_text(json.dumps({
        'architecture': 'PE32 i386', 'scope': 'Optional admitted native bevel/halo/whitewash pixel loops; original Lock/Unlock retained',
        'sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources': {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}}, indent=2)+'\n')
    return dll


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--selftest', action='store_true')
    print(build(parser.parse_args().selftest))
