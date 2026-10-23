#!/usr/bin/env python3
"""Execute the bounded PE32 producer-recorder mutex/thread ownership fixture."""
from coverage_evidence import read_json, evidence_digest, write_report
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--borrow-fixture-prefix',type=Path);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    for name in ('tools/test-drawing-producer-trace.py','tests/drawing-producer-trace-test.c','runtime/scene/producer_trace_guard.h','runtime/shadow/win32_min.h'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-producer-trace';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    imports=dict(ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CreateThread=24,WaitForSingleObject=8,Sleep=4,CloseHandle=4)
    definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-producer-trace-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import borrow
    if a.borrow_fixture_prefix:prefix,_=borrow(ROOT,a.borrow_fixture_prefix)
    else:
        subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
        prefix=run/'wineprefix'
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1')
    results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        cases=[('ownership',{},0),('exhaustion',dict(MNM_TRACE_TEST='exhaustion'),98)]
        cases += [('invalid-'+str(i),dict(MNM_TRACE_TEST='invalid',MNM_DRAW02_PRODUCER_BIND=value),0) for i,value in enumerate(('0','2','11','', 'invalid'))]
        for label,extra,expected in cases:
            with (run/(label+'.log')).open('x') as out:process=subprocess.run(['wine',str(binary)],env=dict(env,**extra),stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if process.returncode!=expected:raise ValueError(f'{label}: expected {expected}, got {process.returncode}')
            results.append(dict(case=label,expected_exit=expected,actual_exit=process.returncode))
        if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,shared_increments=2000,thread_local_primitive_depth=True,mutex_released_across_simulated_original_body=True,last_error_preserved=True,thread_slot_limit=32,slot_exhaustion_refused=True,original_game_executed=False,producer_bytes_immutable=False,physical_frame_identity_established=False,scope='Actual PE32 Win32 critical-section/thread fixture for opt-in recorder synchronization, independent primitive nesting, exact shared increments, bounded thread-slot and malformed-mode refusal. No original body/source bytes, PE32 producer trampoline or physical presentation equivalence.')
        a.report.parent.mkdir(parents=True,exist_ok=True);write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,cases=len(results),shared_increments=2000)))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)

if __name__=='__main__':main()
