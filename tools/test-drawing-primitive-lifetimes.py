#!/usr/bin/env python3
"""Execute the actual opt-in PE32 primitive lifetime journal on two threads."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_primitive_lifetimes import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('prefix-template','claims','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_bytes());sources=claims['sources']
    stable=lambda:all(sha((ROOT/name).read_bytes())==digest for name,digest in sources.items())
    if not stable():raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-primitive-lifetimes';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    imports=dict(GetProcessHeap=0,HeapAlloc=12,HeapFree=12,CreateFileA=28,WriteFile=20,CloseHandle=4,GetLastError=0,SetLastError=4,GetEnvironmentVariableA=12,ExitProcess=4,GetCurrentThreadId=0,QueryPerformanceCounter=4,QueryPerformanceFrequency=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,CreateThread=24,WaitForSingleObject=8,Sleep=4)
    (run/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(run/'kernel32.def'),'-l',str(run/'kernel32.lib')],check=True)
    subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/drawing-primitive-lifetimes-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe'
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_PRIMITIVE_LIFETIMES='1')
    cases=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        for label,flag,expected in [('normal','1',0),('invalid','0',0),('invalid-long','11',0),('prerequisite','1',0),('budget','1',99),('write','1',99)]:
            target=run/label;target.mkdir();case_env=dict(env,MNM_FIXTURE_MODE=label,MNM_DRAW_PRIMITIVE_LIFETIMES=flag,MNM_SCENE_DIR='Z:'+str(target).replace('/','\\'))
            with (target/'fixture.log').open('x') as out:result=subprocess.run(['wine',str(binary)],env=case_env,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if result.returncode!=expected:raise ValueError(f'{label}: expected {expected}, got {result.returncode}')
            cases.append(dict(case=label,expected=expected,actual=result.returncode))
        journal=(run/'normal/primitive-lifetimes-0000.bin').read_bytes();observed=decode(journal)
        if observed['calls']!=3 or [b['calls'] for b in observed['boundaries']]!=[[1,2],[3]] or len({c['thread'] for c in observed['pending_boundary_calls']})!=2:raise ValueError('Pending nested/foreign call identities lost')
        if not stable():raise ValueError('Fixture source changed')
        report=dict(schema=1,success=True,claims=claims['claims'],sources=sources,sources_stable=True,experiment=str(run),binary_sha256=sha(binary.read_bytes()),cases=cases,observation=observed,original_game_executed=False,scope='Actual PE32 recorder with owned two-thread original-body stand-ins; exact nested/foreign pending call identities, acquisition/return envelopes, LastError and bounded malformed/prerequisite/budget/write refusal. No original source-byte ownership or game equivalence.')
        with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
        print(json.dumps(dict(success=True,cases=len(cases),calls=observed['calls'])))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        subprocess.run(['wineserver','-w'],env=env,check=False,timeout=30)


if __name__=='__main__':main()
