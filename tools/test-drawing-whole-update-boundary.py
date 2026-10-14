#!/usr/bin/env python3
"""Execute the actual whole-update boundary/callback code against owned stand-ins."""
import argparse,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--shared-fixture-prefix',type=Path);p.add_argument('--update-routes',action='store_true');p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():return all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/drawing-whole-update';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    spec=importlib.util.spec_from_file_location('cadence_build',ROOT/'tools/build-engine-cadence.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(run/'build')
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    definition=(dll.parent/'kernel32.def').read_text()+'InitializeCriticalSection@4\nEnterCriticalSection@4\nLeaveCriticalSection@4\nCreateThread@24\n'
    (run/'fixture-kernel32.def').write_text(definition);subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(run/'fixture-kernel32.def'),'-l',str(run/'fixture-kernel32.lib')],check=True)
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-whole-update-boundary-test.c'),'-o',str(run/'fixture.obj')],check=True);binary=run/'fixture.exe'
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(run/'fixture-kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_UPDATE_BINDING='1',MNM_DRAW_WHOLE_UPDATES='1')
    if a.update_routes:env['MNM_DRAW_UPDATE_ROUTES']='1'
    cases=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        for mode in ('g','s','t','r','o','p','a','n','e','c','f','w','z'):
            target=run/mode;target.mkdir();env.update(MNM_FIXTURE_MODE=mode,MNM_SCENE_DIR='Z:'+str(target).replace('/','\\'))
            with (target/'fixture.log').open('x') as out:execution=subprocess.run(['wine',str(binary)],env=env,stdout=out,stderr=subprocess.STDOUT,timeout=20)
            expected=0 if mode in ('g','w') else 99
            if execution.returncode!=expected:raise ValueError(f'{mode}: {execution.returncode}, expected {expected}')
            cases.append(dict(mode=mode,returncode=execution.returncode))
        done=(run/'g/canvas-producers.done').read_bytes()
        if len(done)!=32 or done[:8]!=b'MNMPDONE' or struct.unpack('<8I',done)[2:]!=(1,1,1,0,0,160):raise ValueError('Whole-update completion marker')
        if a.update_routes:
            data=(run/'g/source-update-windows-0000.bin').read_bytes()
            rows=list(struct.iter_unpack('<16I',data[64:]))
            if data[:8]!=b'MNMDWIN1' or len(rows)!=3 or [r[1] for r in rows]!=[1,1,4] or [r[3] for r in rows[:2]]!=[30,31] or rows[-1][7:10]!=(1,1,0):raise ValueError('Source boundary prefixes not retained')
        if not stable():raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=cases,done_sha256=sha(done),original_game_executed=False,scope='Actual PE32 boundary/callback code: early no-op, one closure/checkpoints/end marker/done/LastError/idempotence; sequence/counter/event/owner/frame/active/incomplete/failure/checkpoint/flush refusals. Owned emit/checkpoint/close stand-ins; no original source or hardware completion.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(cases))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
