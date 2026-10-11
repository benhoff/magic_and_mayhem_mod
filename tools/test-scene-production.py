#!/usr/bin/env python3
"""Actual PE32 forwarding, extra-call, state mutation and admission controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Changed prospective fixture sources')
    parent=ROOT/'working/tests/scene-production-controls';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    report=dict(success=False,claims=declaration['claims'],sources=declaration['sources'],cases=[])
    try:
        spec=importlib.util.spec_from_file_location('production_build',ROOT/'tools/build-scene-production.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        dll=module.build(out,selftest=True)
        with (out/'build.log').open('x') as log:
            for name in ('scene-production-reference.c','scene-production-reference.S'):
                subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
                                '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests'/name),'-o',str(out/(name+'.obj'))],stdout=log,stderr=subprocess.STDOUT,check=True)
            executable=out/'fixture.exe'
            subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/fixed','/dynamicbase:no','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{executable}',
                            str(out/'scene-production-reference.c.obj'),str(out/'scene-production-reference.S.obj'),str(dll.with_suffix('.lib')),str(out/'kernel32.lib')],stdout=log,stderr=subprocess.STDOUT,check=True)
        for mode in (1,2):
            for case in range(9):
                key=f'{case}{mode}';case_env=dict(env,MNM_PRODUCTION_TEST_CASE=key)
                with (out/f'case-{key}.log').open('x') as log:
                    result=subprocess.run(['xvfb-run','-a','wine',str(executable)],env=case_env,cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=120)
                report['cases'].append(dict(case=case,mode=mode,exit_code=result.returncode))
                if result.returncode:raise ValueError(f'PE32 production control failed {key}:exit{result.returncode}')
        report.update(binary_sha256=sha(executable),dll_sha256=sha(dll),sources_unchanged=all(sha(ROOT/p)==h for p,h in report['sources'].items()),
                      checks='18 PE32 full/slice controls: original caller registers/flags/512B FP/error state and exact stack restoration, normal+extra once-only queue/call counts;28 single-byte guard refusals and bad/repeated admission; counter/whole-creature/scheduler/last RNG word/gameplay object/map context changes, changed pool shape and unreadable pool refuse. Authored stand-ins, not original engine execution.')
        if not report['sources_unchanged']:raise ValueError('Fixture sources changed')
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if (out/'wineprefix').exists():subprocess.run(['wineserver','-k'],env=env,timeout=10,check=True)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
