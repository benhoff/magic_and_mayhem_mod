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
    parent=ROOT/'working/tests/drawing-producer-update';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    spec=importlib.util.spec_from_file_location('cadence_build',ROOT/'tools/build-engine-cadence.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(run/'build')
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-ffunction-sections','-Wall','-Wextra','-Werror','-Wno-unused-function']
    binaries={}
    for name in ('drawing-cadence-queue-point-test','drawing-producer-update-test'):
        subprocess.run(flags+['-c',str(ROOT/'tests'/f'{name}.c'),'-o',str(run/f'{name}.obj')],check=True)
        command=['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/opt:ref','/out:'+str(run/f'{name}.exe'),str(run/f'{name}.obj'),str(dll.parent/'kernel32.lib')]
        if 'cadence' in name:command+=['/base:0x400000','/fixed','/dynamicbase:no',str(dll.parent/'entry.S.obj')]
        subprocess.run(command,check=True);binaries[name]=run/f'{name}.exe'
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_UPDATE_BINDING='1')
    cases=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        for mode,expected in [('provider',0),('g',0),('m',99),('r',99),('d',99),('q',99)]:
            target=run/mode;target.mkdir();env.update(MNM_FIXTURE_MODE=mode,MNM_SCENE_DIR='Z:'+str(target).replace('/','\\'))
            binary=binaries['drawing-cadence-queue-point-test' if mode=='provider' else 'drawing-producer-update-test']
            with (target/'fixture.log').open('x') as out:execution=subprocess.run(['wine',str(binary)],env=env,stdout=out,stderr=subprocess.STDOUT,timeout=20)
            if execution.returncode!=expected:raise ValueError(f'{mode}: {execution.returncode}, expected {expected}')
            case=dict(mode=mode,returncode=execution.returncode)
            if mode=='g':
                raw=(target/'producer-update-points-0000.bin').read_bytes();observed=decode(raw)
                if observed['queue_count']!=1:raise ValueError('Producer queue closure')
                case.update(journal_sha256=sha(raw),observed=observed)
            cases.append(case)
        if not stable():raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binaries={n:sha(p.read_bytes()) for n,p in binaries.items()},cases=cases,original_game_executed=False,scope='Actual PE32 cadence getter success/twelve guards/two caller IDs/LastError; producer point order/closure and changed point,refused getter,nested queue,wrong queue refusals. No original source/update ownership or presentation claim.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(cases))))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__=='__main__':main()
