#!/usr/bin/env python3
"""Execute the bounded PE32 producer-recorder mutex/thread ownership fixture."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    for name in ('tools/test-drawing-source-lease.py','tests/drawing-source-lease-test.c','runtime/scene/producer_trace_guard.h','runtime/scene/drawing_source_point.h','runtime/scene/drawing_source_bytes.h','runtime/scene/drawing_source_lease.h','runtime/shadow/win32_min.h'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-source-lease';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    imports=dict(ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CreateThread=24,WaitForSingleObject=8,Sleep=4,CloseHandle=4,VirtualQuery=12,VirtualAlloc=16,VirtualProtect=16,AddVectoredExceptionHandler=8,CreateFileA=28,WriteFile=20,GetTickCount=0)
    definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-lease-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_SOURCE_LEASE='1')
    results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        for label,extra,expected in [('restore',{},0),('write-fault',dict(MNM_LEASE_TEST='fault'),99)]:
            with (run/(label+'.log')).open('x') as out:process=subprocess.run(['wine',str(binary)],cwd=run,env=dict(env,**extra),stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if process.returncode!=expected:raise ValueError(label+'expected'+str(expected)+'got'+str(process.returncode))
            results.append(dict(case=label,actual_exit=process.returncode))
        fault=(run/'source-lease-fault-.bin').read_bytes()
        if fault[:8]!=b'MNMSLFT1' or len(fault)!=48:raise ValueError('Leasefaultrecord')
        if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,read_only_protection_verified=True,wrong_lease_refused=True,exact_protection_restored=True,write_fault_refused=True,fault_sha256=sha(fault),original_game_executed=False,scope='ActualPE32isolatedVirtualAllocpagelease/protection/restore/writefaultfixture; nooriginalsharedheap/asyncEGLsourceownership orcompletejourney.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,restore=True,write_fault_refused=True)))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)

if __name__=='__main__':main()
