#!/usr/bin/env python3
"""Observe unchanged original canvas wrappers with actual Wine loss/Restore outcomes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    p.add_argument('--prefix-template',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        p.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    ids=['RS.canvas-driver-recovery']
    paths=set().union(*(required_sources(bs[i]) for i in ids))
    paths.update(['tools/coverage_claims.py','runtime/shadow/win32_min.h'])
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    claims=[{'behavior':i,'contract_sha256':behavior_contract(bs[i],builds)} for i in ids]
    report={'schema':1,'success':False,'original_sha256':BUILD,'sources':sources,
            'claims':claims,'scope':__doc__,'original_instruction_bytes_patched':False,
            'scripted_hresult':False,'live_replacement':False,'physical_driver_equivalence':False}
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEDEBUG='-all',WINEPREFIX=str(out/'wineprefix'),
               WINEDLLOVERRIDES='ddraw=b',LIBGL_ALWAYS_SOFTWARE='1',MNM_ORIGINAL_IMAGE='Z:'+str(a.executable.resolve()).replace('/','\\'))

    def run(name,command,timeout=120):
        print(name,flush=True)
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=log,
                           stderr=subprocess.STDOUT,check=True,timeout=timeout)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported original input')
        run('prefix',['cp','-a','--reflink=auto',a.prefix_template.resolve(),out/'wineprefix'])
        imports={'GetEnvironmentVariableA':12,'CreateFileA':28,'GetFileSize':8,
                 'ReadFile':20,'CloseHandle':4,'GetProcessHeap':0,'HeapAlloc':12,
                 'HeapFree':12,'VirtualAlloc':16,'WriteFile':20,'ExitProcess':4}
        imports.update(GetModuleHandleA=4,VirtualProtect=16,Sleep=4,LoadLibraryA=4,GetProcAddress=8,GetLastError=0,SetLastError=4)
        definition=out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('original-build',['clang','--target=i686-pc-windows-msvc','-O2',
            '-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
            '-Wall','-Wextra','-Werror','-MD','-MF',out/'probe.d','-c',
            ROOT/'tests/original-canvas-driver-reference.c','-o',out/'probe.obj'])
        closure=[]
        for name in (out/'probe.d').read_text().replace('\\\n',' ').split()[1:]:
            path=Path(name)
            if path.is_file() and path.resolve().is_relative_to(ROOT):
                name=path.resolve().relative_to(ROOT).as_posix()
                if name not in sources:raise ValueError('Undeclared compiler input: '+name)
                closure.append(name)
        report['compiled_original_sources']=closure
        pad=out/'private-image-pad.S'
        pad.write_text('.text\n.globl _original_fixture_pad\n_original_fixture_pad:\n.space 0x400000\n')
        run('original-pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('original-link',['lld-link','/machine:x86','/entry:start','/subsystem:console',
            '/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0',
            '/include:_original_fixture_pad','/out:'+str(out/'probe.exe'),out/'pad.obj',
            out/'probe.obj',out/'kernel32.lib'])
        run('original-execute',['wine','explorer','/desktop=original-driver,1024x768',out/'probe.exe'])
        from original_canvas_driver import analyze
        report.update(analyze((out/'original-driver.bin').read_bytes()))
        if sha(a.executable)!=BUILD:raise ValueError('Original input changed')
        report.update(success=True,probe_sha256=sha(out/'probe.exe'),
                      original_result_sha256=sha(out/'original-driver.bin'))
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if (out/'wineprefix').exists():
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
                report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as e:report.update(success=False,preservation_error=str(e))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
