#!/usr/bin/env python3
"""Build the optional PE32 original import call observer and ABI fixture."""
import hashlib
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
IMPORTS=dict(GetModuleHandleA=4,GetModuleHandleExA=12,GetModuleFileNameA=12,VirtualAlloc=16,VirtualProtect=16,VirtualQuery=12,FlushInstructionCache=12,GetCurrentProcess=0,GetEnvironmentVariableA=12,CreateFileA=28,GetFileSize=8,CreateFileMappingA=24,MapViewOfFile=20,CloseHandle=4,GetLastError=0,SetLastError=4,ExitProcess=4)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def build(root=None,selftest=False):
    root=(root or ROOT/('working/build/api-census-selftest' if selftest else 'working/build/api-census'));root.mkdir(parents=True,exist_ok=True)
    source=ROOT/'runtime/api-census'
    definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in IMPORTS.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(root/'kernel32.lib'),'--kill-at'],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags.append('-DMNM_API_CENSUS_SELFTEST')
    dependency=root/'observer.d'
    subprocess.run(flags+['-MD','-MF',str(dependency),'-c',str(source/'observer.c'),'-o',str(root/'observer.obj')],check=True)
    dll=root/'MnmApiCensus.dll'
    exports=['/export:ApiCensusAnchor']
    if selftest:exports+=['/export:CensusProxyForTest=_CensusProxyForTest@8','/export:CensusCountForTest=_CensusCountForTest@8','/export:CensusSeedForTest=_CensusSeedForTest@12']
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',*exports,f'/out:{dll}',str(root/'observer.obj'),str(root/'kernel32.lib')],check=True)
    import shlex
    closure=set()
    for token in shlex.split(dependency.read_text().replace('\\\n',' ')):
        p=Path(token).resolve()
        if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working') and p.is_file():closure.add(str(p.relative_to(ROOT)))
    (root/'manifest.json').write_text(json.dumps(dict(schema=1,architecture='PE32 i386',scope='Optional original190-import forwarding census; not all Wine calls',sha256=sha(dll),compiled_sources={p:sha(ROOT/p) for p in sorted(closure)}),indent=2)+'\n')
    if selftest:
        definition=root/'census.def';definition.write_text('LIBRARY MnmApiCensus.dll\nEXPORTS\nCensusProxyForTest@8\nCensusCountForTest@8\nCensusSeedForTest@12\n')
        subprocess.run(['llvm-dlltool','-m','i386','-D','MnmApiCensus.dll','-d',str(definition),'-l',str(root/'census.lib'),'--kill-at'],check=True)
        subprocess.run(flags+['-c',str(source/'selftest.c'),'-o',str(root/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root/"selftest.exe"}',str(root/'selftest.obj'),str(root/'census.lib'),str(root/'kernel32.lib')],check=True)
    return dll
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--selftest',action='store_true');print(build(selftest=p.parse_args().selftest))
