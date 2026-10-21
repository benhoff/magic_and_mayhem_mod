#!/usr/bin/env python3
"""Build the opt-in native world cell producer adapter without consuming game input."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def build(output):
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    temporary=output.resolve()/'tmp'
    temporary.mkdir(exist_ok=True)
    env=dict(os.environ,TMPDIR=str(temporary))
    spec = importlib.util.spec_from_file_location('world_cell_imports', ROOT/'tools/build-menu-observer.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    definition = output/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(
        f'{name}@{size}\n' for name, size in module.IMPORTS.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', str(definition),
                    '-l', str(output/'kernel32.lib'), '--kill-at'], check=True,env=env)
    flags = ['clang++', '-std=c++17', '--target=i686-pc-windows-msvc', '-O2', '-fwrapv', '-ffreestanding',
             '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
             '-Wall', '-Wextra', '-Werror', '-fno-exceptions', '-fno-rtti']
    sources = [ROOT/'runtime/world-cell-producer'/n for n in ('adapter.cpp', 'entry.S')]
    for source in sources:
        subprocess.run([*flags, '-c', str(source), '-o', str(output/(source.name+'.obj'))], check=True,env=env)
    dll = output/'MnmWorldCellProducer.dll'
    subprocess.run(['lld-link', '/dll', '/machine:x86', '/entry:DllMain@12', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/export:WorldCellProducerAnchor', f'/out:{dll}',
                    *[str(output/(p.name+'.obj')) for p in sources], str(output/'kernel32.lib')], check=True,env=env)
    sources += [ROOT/'runtime/world-cell-producer/original_bytes.h', 
                ROOT/'reconstruction/rendering/world_cell_producer.hpp',
                ROOT/'runtime/shadow/win32_min.h', ROOT/'tools/build-menu-observer.py', Path(__file__).resolve()]
    (output/'manifest.json').write_text(json.dumps({
        'schema': 1, 'sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'scope': 'Native complete world cell draw decisions, resource binding, picking/state writes and native queue publication; named animation/gameplay/math services retain their explicit boundaries.',
        'sources': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
    }, indent=2)+'\n')
    return dll


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    print(build(parser.parse_args().output))
