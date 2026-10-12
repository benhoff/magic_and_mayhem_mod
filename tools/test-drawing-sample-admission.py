#!/usr/bin/env python3
"""Actual PE32 read-only cadence point and producer journal refusal fixtures."""
import argparse,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_producer_updates import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():return all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/drawing-sample-admission';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    spec=importlib.util.spec_from_file_location('cadence_build',ROOT/'tools/build-engine-cadence.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(run/'build')
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-ffunction-sections','-Wall','-Wextra','-Werror','-Wno-unused-function']
    name='drawing-sample-admission-test'
    subprocess.run(flags+['-c',str(ROOT/'tests'/f'{name}.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'kernel32.lib')],check=True)
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_UPDATE_BINDING='1')
    cases=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        with (run/'fixture.log').open('x') as out:execution=subprocess.run(['wine',str(binary)],env=env,stdout=out,stderr=subprocess.STDOUT,timeout=20)
        execution.check_returncode();cases.append(dict(mode='admission',returncode=0))
        tests=subprocess.run(['python3',str(ROOT/'tests/test-drawing-source-completion.py')],capture_output=True,text=True,check=True);cases.append(dict(mode='completion',stdout=tests.stdout,stderr=tests.stderr,returncode=tests.returncode))
        if not stable():raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=cases,original_game_executed=False,scope='PE32shared Whole120/legacy admission combinations and Python boundedcompletion heartbeat/exactmarker/death/timeout/corruption tests;nooriginal120source orpresentation claim.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(cases))))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__=='__main__':main()
