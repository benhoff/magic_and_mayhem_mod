#!/usr/bin/env python3
"""Build the separate optional PE32 original cursor mutex observer."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def build(output=None):
    output = output or ROOT / 'working/build/cursor-mutex-observer'
    output.mkdir(parents=True, exist_ok=True)
    imports = dict(GetProcAddress=8, GetModuleHandleA=4, GetEnvironmentVariableA=12,
                   GetLastError=0, SetLastError=4, GetCurrentThreadId=0, GetTickCount=0,
                   InitializeCriticalSection=4, EnterCriticalSection=4, LeaveCriticalSection=4,
                   VirtualQuery=12, VirtualProtect=16, CreateFileA=28, WriteFile=20,
                   CloseHandle=4, ExitProcess=4)
    definition = output / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(output / 'kernel32.lib')], check=True)
    source = ROOT / 'runtime/cursor-mutex/observer.c'
    subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-c', str(source), '-o', str(output / 'observer.obj')], check=True)
    binary = output / 'MnmCursorMutex.dll'
    subprocess.run(['lld-link', '/dll', '/machine:x86', '/entry:DllMain@12', '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/export:CursorMutexAnchor', '/out:' + str(binary), str(output / 'observer.obj'), str(output / 'kernel32.lib')], check=True)
    sources = ['runtime/cursor-mutex/observer.c', 'runtime/shadow/win32_min.h', 'tools/build-cursor-mutex-observer.py']
    (output / 'manifest.json').write_text(json.dumps(dict(schema=1, sha256=hashlib.sha256(binary.read_bytes()).hexdigest(), sources={name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in sources}, scope='Opt-in observation of pinned original CreateMutex/Wait/Release/Close imports; no added original wait or source synchronization.'), indent=2) + '\n')
    return binary


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    print(build(parser.parse_args().output))
