#!/usr/bin/env python3
"""Exercise bounded fence refusals and actual PE32 worker ABI forwarding."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_gpu_completion import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    stable();parent=ROOT/'working/tests/drawing-gpu-completion';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True);(run/'.retain').write_text('DRAW-02 owned fence/worker ABI raw records.\n')
    imports=dict(GetTickCount=0,ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CloseHandle=4,VirtualQuery=12,CreateFileA=28,WriteFile=20)
    definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()));subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-gpu-completion-test.c'),'-o',str(run/'fixture.obj')],check=True);binary=run/'fixture.exe'
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix);env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_BLIT_WORKER='1');results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        cases=[('satisfied',0,0),('already-signaled',1,0),('no-context',2,0),('sentinel-procedures',3,99),('null-fence',4,99),('timeout',5,99),('wait-failed',6,99),('context-changed',8,99),('unknown-wait',9,99),('invalid-flag',10,0),('default-off',11,0)]
        for label,mode,expected in cases:
            target=run/label;target.mkdir();trial=dict(env,MNM_GPU_TEST=chr(ord('0')+mode))
            if mode==10:trial['MNM_DRAW_WINE_GPU_COMPLETION']='x'
            if mode==11:trial.pop('MNM_DRAW_WINE_GPU_COMPLETION')
            with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],cwd=target,env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if x.returncode!=expected:raise ValueError(f'{label}: exit {x.returncode}')
            record=target/'wine-gpu-completion.bin';observed=decode(record.read_bytes(),complete=not expected) if record.exists() else None
            if mode in (10,11):
                if observed is not None:raise ValueError('Disabled diagnostic wrote a journal')
            elif observed is None or len(observed['calls'])!=1 or (observed['calls'][0]['worker'],observed['calls'][0]['command'],observed['calls'][0]['submission'])!=(1,7,8):raise ValueError('Worker binding missing')
            results.append(dict(case=label,exit_code=x.returncode,observation=observed))
        checks=subprocess.run(['python3',str(ROOT/'tests/test-drawing-gpu-completion.py')],capture_output=True,text=True,check=True);stable()
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,decoder_tests=checks.stderr,worker_abi_fp_last_error_preserved=True,original_callback_forwarded_once=True,failed_fence_prevents_worker_return=True,original_game_executed=False,actual_gl_context_executed=False,scope='Owned PE32 fake GL public ABI responses through the actual worker wrapper. Signal/no-context/default-off accepted, six failures stop before lease-return publication. Not hardware or original GPU proof.')
        with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
        print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
