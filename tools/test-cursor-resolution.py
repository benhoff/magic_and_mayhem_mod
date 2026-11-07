#!/usr/bin/env python3
"""Bounded original cursor mismatch reproduction and native admission controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/cursor-resolution-results-20261107/'
BID='NP.cursor-resolution-admission'
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True)
    p.add_argument('--prefix-template',type=Path,required=True)
    p.add_argument('--original-root',type=Path,default=Path('/home/hoff/swdev/magic_and_mayhem_mod'))
    a=p.parse_args();out=a.output.resolve();original=a.original_root.resolve()
    if out.exists() or not out.is_relative_to(Path('/tmp')) or not a.report.startswith(PREFIX) or '..' in Path(a.report).parts or (ROOT/a.report).exists():
        p.error('Use new RAM-backed output and owned immutable report paths')
    out.mkdir(parents=True);report=dict(schema=1,kind='cursor_resolution_admission_controls',success=False,
        commands=[],profiles={},original_game_runs=0,scope='Unchanged original foreground mismatch with scripted endpoints; real owning native installer/provider and original foreground/timer/background bodies at controlled mode and saved-coordinate boundaries. No game entrypoint, historical-capture attribution, physical driver or whole-player acceptance.')
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in reg['behaviors'] if b['id']==BID)
    paths=set(required_sources(behavior))
    dependency_text=subprocess.check_output(['g++','-m32','-std=c++17','-MM','tests/cursor-resolution-reference.cpp'],cwd=ROOT,text=True)
    paths.update(token for token in dependency_text.replace('\\\n',' ').split() if (ROOT/token).is_file())
    for d in ['runtime/native-draw','runtime/render','runtime/scene','protocols/include','protocols/schemas']:
        paths.update(f.relative_to(ROOT).as_posix() for f in (ROOT/d).rglob('*') if f.is_file() and f.suffix in ('.c','.h','.hpp','.inc','.json'))
    report['sources']={n:digest(ROOT/n) for n in sorted(paths)}
    report['claims']=[dict(behavior=BID,contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}),
        scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if BID in s['behaviors']})]
    image=original/'original/Arcane_Nocd/Chaos.exe';verified=False
    def run(name,args):
        with (out/(name+'.log')).open('xb') as f:
            result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180)
        report['commands'].append(dict(name=name,exit=result.returncode,argv=list(map(str,args))))
        if result.returncode:raise ValueError(name+' failed: '+(out/(name+'.log')).read_text(errors='replace')[-2000:])
    try:
        subprocess.run([original/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.DEVNULL);verified=True
        if digest(image)!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Original image differs')
        report['original_sha256']=digest(image)
        run('reference-build',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-mstackrealign','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-misleading-indentation',ROOT/'tests/cursor-resolution-reference.cpp','-o',out/'reference'])
        run('reference',[out/'reference',image]);report['original_reproduction']=json.loads((out/'reference.log').read_text())
        builder=module('cursor_geometry_build','tools/build-native-draw.py')
        builder.build(out/'provider-build',report['sources'])
        import shutil
        shutil.copyfile(out/'provider-build/MnmDraw.dll',out/'MnmDraw.dll')
        imports={'LoadLibraryA':4,'GetProcAddress':8,'GetModuleHandleA':4,'ExitProcess':4,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'CreateThread':24,'CreateEventA':16,'SetEvent':4,'WaitForSingleObject':8,'GetCurrentThreadId':0,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'VirtualAlloc':16,'VirtualProtect':16,'VirtualQuery':12,'ReadFile':20,'GetFileSize':8,'GetEnvironmentVariableA':12,'QueryPerformanceCounter':4,'QueryPerformanceFrequency':4,'GetLastError':0,'SetLastError':4}
        d=out/'kernel32.def';d.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',d,'-l',out/'kernel32.lib'])
        d=out/'gdi32.def';d.write_text('LIBRARY GDI32.dll\nEXPORTS\nSetPixel@16\n');run('gdi-imports',['llvm-dlltool','-m','i386','--kill-at','-d',d,'-l',out/'gdi32.lib'])
        run('compile',[*builder.FLAGS,'-c',ROOT/'tests/native-cursor-resolution-probe.c','-o',out/'probe.obj'])
        pad=out/'pad.S';pad.write_text('.text\n.globl _cursor_fixture_pad\n_cursor_fixture_pad:\n.space 0x400000\n')
        run('pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/base:0x400000','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'pad.obj',out/'probe.obj',out/'kernel32.lib',out/'gdi32.lib','/dynamicbase:no','/include:_cursor_fixture_pad'])
        runner=module('cursor_geometry_execution','tools/test-native-cursor-lifetime.py')
        ram=out/'ram';ram.mkdir()
        runner.execute(out,a.prefix_template.resolve(),ram,True,original)
        for name in ['plain','protected','early-plain','early-protected']:
            values=struct.unpack('<32I',(out/name/'cursor-resolution.bin').read_bytes());state=values[4:]
            if values[0]!=1 or values[2:4]!=(27,18) or not state[1] or not state[2] or any(state[5:11]) or state[12]!=state[13]+state[14]:raise ValueError('Incomplete geometry/retirement result')
            report['profiles'][name]=dict(assertions=values[1],complete_planes=values[2],installed_hooks=values[3],final_status=list(state),remaining_resources=0)
        report.update(success=True,changed_anchor_refusals=18)
    except (ValueError,OSError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        if verified:
            try:subprocess.run([original/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.DEVNULL);report['original_manifest_before_after']=True
            except (OSError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(digest(ROOT/n)==h for n,h in report['sources'].items());report['success'] &= report['sources_stable']
        # Persist exact executables, preimages and diagnostics directly in SQLite.
        stem=Path(a.report).stem;artifacts=[]
        for source in sorted(required_sources(behavior)):
            data=(ROOT/source).read_bytes();name=PREFIX+stem+'-source-'+hashlib.sha256(source.encode()).hexdigest()[:20]+'.bin'
            sha=write_report(ROOT,name,data);artifacts.append(dict(path=source,record=name,sha256=sha,bytes=len(data),source_preimage=True))
        for f in sorted(out.rglob('*')):
            if f.is_file() and 'ram' not in f.relative_to(out).parts and f.suffix in ('.log','.bin','.exe','.dll','.txt'):
                data=f.read_bytes();name=PREFIX+stem+'-'+hashlib.sha256(f.relative_to(out).as_posix().encode()).hexdigest()[:20]+'.bin'
                sha=write_report(ROOT,name,data);artifacts.append(dict(path=f.relative_to(out).as_posix(),record=name,sha256=sha,bytes=len(data)))
        report['artifacts']=artifacts
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report[k] for k in ['success','sources_stable','profiles']}|dict(error=report.get('error'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
