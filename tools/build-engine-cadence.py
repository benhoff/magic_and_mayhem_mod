#!/usr/bin/env python3
"""Build an isolated PE32 engine-cadence observer, without consuming game inputs."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def build(output):
    output.mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location('menu_build', ROOT/'tools/build-menu-observer.py')
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    imports = dict(module.IMPORTS, QueryPerformanceCounter=4, QueryPerformanceFrequency=4)
    definition = output/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    flags = ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
             '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    sources = [ROOT/'runtime/engine-cadence/observer.c', ROOT/'runtime/engine-cadence/entry.S']
    for source in sources:
        subprocess.run([*flags,'-c',str(source),'-o',str(output/(source.name+'.obj'))],check=True)
    dll = output/'MnmEngineCadence.dll'
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no',
                    '/timestamp:0','/export:EngineCadenceAnchor','/export:EngineCadenceQueuePointV1=_EngineCadenceQueuePointV1@16',f'/out:{dll}',
                    *[str(output/(s.name+'.obj')) for s in sources],str(output/'kernel32.lib')],check=True)
    sources += [ROOT/'runtime/engine-cadence/update_boundary.h',ROOT/'runtime/engine-cadence/queue_point.h', ROOT/'protocols/include/mnm/engine_cadence_queue_point_v1.h', ROOT/'runtime/shadow/win32_min.h', ROOT/'tools/build-menu-observer.py',Path(__file__).resolve()]
    (output/'manifest.json').write_text(json.dumps(dict(sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),
        sources={s.relative_to(ROOT).as_posix():hashlib.sha256(s.read_bytes()).hexdigest() for s in sources},
        scope='Opt-in source-guarded No-CD engine cadence observation; original clock, state and targets unchanged.'),indent=2)+'\n')
    return dll

if __name__ == '__main__':
    print(build(ROOT/'working/build/engine-cadence'))
