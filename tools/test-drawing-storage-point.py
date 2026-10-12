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
    for name in ('tools/test-drawing-storage-point.py','tests/drawing-storage-point-test.c','runtime/scene/producer_trace_guard.h','runtime/scene/drawing_storage_point.h','protocols/include/mnm/drawing_storage_point_v1.h','runtime/shadow/win32_min.h'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-storage-point';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    imports=dict(ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CreateThread=24,WaitForSingleObject=8,Sleep=4,CloseHandle=4,VirtualQuery=12,VirtualAlloc=16,VirtualProtect=16)
    definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-storage-point-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1')
    results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        with (run/'copy.log').open('x') as out:process=subprocess.run(['wine',str(binary)],env=env,stdout=out,stderr=subprocess.STDOUT,timeout=30)
        if process.returncode:raise ValueError('Sourcebyteexportfixture exit:'+str(process.returncode))
        results=[dict(case='point-association-and-refusals',actual_exit=0)]
        if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,cache_only=True,unknown_ambiguous_extent_failure_stopped_output_guards=True,last_error_preserved=True,original_game_executed=False,source_storage_owned=False,scope='Actual PE32 cache-only pointer/pitch association, padded stride, unknown/ambiguous/mismatched/failed/stopped cache, nesting and output bounds; unreadable storage is never read. LastError preserved; no original ownership.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cache_only=True,refusal_guards=True)))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)

if __name__=='__main__':main()
