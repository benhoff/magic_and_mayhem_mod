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
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources']);sources['tools/drawing_fixture_prefix.py']=sha((ROOT/'tools/drawing_fixture_prefix.py').read_bytes())
    for name in ('tools/test-drawing-source-pending-policy.py','tests/drawing-source-pending-policy-test.c','runtime/scene/producer_trace_guard.h','runtime/scene/drawing_source_point.h','runtime/scene/source_pending_policy.h','runtime/scene/drawing_source_bytes.h','runtime/scene/drawing_source_lease.h','runtime/scene/source_pending_refusal.h','tools/drawing_source_pending_refusal.py','tests/test-drawing-source-pending-admission.py','tools/drawing_source_pending_admission.py','runtime/shadow/win32_min.h'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-source-pending-policy';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    imports=dict(ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CreateThread=24,WaitForSingleObject=8,Sleep=4,CloseHandle=4,VirtualQuery=12,VirtualAlloc=16,VirtualProtect=16,AddVectoredExceptionHandler=8,CreateFileA=28,WriteFile=20,GetTickCount=0)
    definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-pending-policy-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,prefix_lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_SOURCE_LEASE='1',MNM_FIXTURE_DIRECTORY='Z:'+str(run).replace('/','\\'))
    results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        from drawing_source_pending_admission import decode
        cases=[]
        for label,extra,expected in [('disabled',{},0),('enabled',dict(MNM_DRAW_SOURCE_PENDING_COPY='1'),0),('alias-write',dict(MNM_DRAW_SOURCE_PENDING_COPY='1',MNM_PENDING_POLICY_TEST='fault'),99)]:
            case=run/label;case.mkdir()
            local=dict(env,**extra);local['MNM_FIXTURE_DIRECTORY']='Z:'+str(case).replace('/','\\')
            with (case/'fixture.log').open('x') as out:process=subprocess.run(['wine',str(binary)],cwd=case,env=local,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if process.returncode!=expected:raise ValueError(label+' expected '+str(expected)+' got '+str(process.returncode))
            admissions=[dict(path=f.name,**decode(f.read_bytes())) for f in sorted(case.glob('source-pending-admission-*.bin'))]
            if len(admissions)!={'disabled':0,'enabled':2,'alias-write':1}[label]:raise ValueError('Admission bounded count')
            fault=next(iter(case.glob('source-lease-fault-*.bin')),None)
            if label=='alias-write':
                import struct
                words=struct.unpack('<12I',fault.read_bytes())
                if fault.read_bytes()[:8]!=b'MNMSLFT1' or words[6]==words[7] or not words[6] or words[3]!=13:raise ValueError('Foreign aliased source write fault')
            cases.append(dict(case=label,exit=process.returncode,admissions=admissions,fault_sha256=sha(fault.read_bytes()) if fault else None))
        if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),shared_fixture_prefix=str(prefix.relative_to(ROOT)) if a.shared_fixture_prefix else None,binary_sha256=sha(binary.read_bytes()),cases=cases,guard_refusals=16,unleased_copy_refused=True,protection_loss_refused=True,source_identity_change_refused=True,foreign_aliased_write_refused=True,last_error_preserved=True,bytes_unchanged_on_refusal=True,original_game_executed=False,disjoint_backing_proved=False,scope='Owned PE32 pages: opt-in exact foreign tags32/39 source-copy admission only under checked readonly lease; disabled/unleased/unknown/lifetime/primitive/reserved/duplicate/refusal guards and real foreign-thread aliased protected-page write exits99. Diagnostic policy, not unobtrusive original ownership.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=3,guard_refusals=16,foreign_aliased_write_refused=True)))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)

if __name__=='__main__':main()
