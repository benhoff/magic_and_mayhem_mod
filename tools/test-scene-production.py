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
import time

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Changed prospective fixture sources')
    if not {'NR.scene-production-repetition','NR.scene-draw-budget'}<=set(c['behavior'] for c in declaration['claims']):raise ValueError('Fixture declaration must include both repetition and draw-budget policies')
    parent=ROOT/'working/tests/scene-production-controls';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    report=dict(success=False,claims=declaration['claims'],sources=declaration['sources'],cases=[])
    server=None
    try:
        subprocess.run(['python3',str(ROOT/'tests/test-scene-production-decoder.py'),str(out/'decoder-report.json')],check=True)
        report['decoder_controls']=json.loads((out/'decoder-report.json').read_text())
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
        # One owned display/server prevents44 repeated startup/teardown cycles.
        with (out/'display-number').open('w+') as number, (out/'display.log').open('x') as log:
            server=subprocess.Popen(['Xvfb','-displayfd',str(number.fileno()),'-screen','0','1280x1024x24','-nolisten','tcp'],pass_fds=(number.fileno(),),stdout=log,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+30
            while True:
                number.seek(0);display=number.read().strip()
                if display:break
                if server.poll() is not None or time.monotonic()>deadline:raise ValueError('Private fixture display failed')
                time.sleep(.05)
        env['DISPLAY']=':'+display
        (out/'wineprefix').mkdir(mode=0o700)
        subprocess.run(['wineserver','-p'],env=env,cwd=out,check=True,timeout=10)
        for mode in (1,2,3,4,5):
            for case in range(16 if mode==5 else 11):
                key=f'{case if case<10 else chr(ord("A")+case-10)}{mode}';case_env=dict(env,MNM_PRODUCTION_TEST_CASE=key)
                with (out/f'case-{key}.log').open('x') as log:
                    result=subprocess.run(['wine',str(executable)],env=case_env,cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=120)
                report['cases'].append(dict(case=case,mode=mode,exit_code=result.returncode))
                if result.returncode:raise ValueError(f'PE32 production control failed {key}:exit{result.returncode}')
        report.update(binary_sha256=sha(executable),dll_sha256=sha(dll),sources_unchanged=all(sha(ROOT/p)==h for p,h in report['sources'].items()),
                      checks='60 PE32 full/slice/frozen-history/prepared-scene controls: original caller registers/flags/512B FP/error state and exact stack restoration, normal+extra once-only queue/call counts;62 single-byte guard refusals and bad/repeated admission; counter/whole-creature/scheduler/last RNG word/gameplay object/map context changes, changed pool shape/unreadable and post-primary queue leak refuse. Original queue postconditions hold after every extra. Prepared-phase repeat precedes the original once-only post-work, including authored post leak (not an extra admission input). History forwards twice for full/slice and once plus one skip for history policies. Mode5 additionally checks18 pacing anchor mutations, nine overflow/deadline arithmetic cases, clock wrap, warmed authored admission with two extra draws/history skips, insufficient-budget/target refusal and actual60ms prediction overrun and exclusion of the first mixed marker-to-entry interval with exact subsequent50ms counter/histogram accounting. The authored warm bypass does not claim16 executed warm rows. Consumer interval entry/exit hooks preserve the original consumer return PCs and both resumed MOV instructions; one extra consumer completion is counted. Authored stand-ins, not original engine execution.')
        if not report['sources_unchanged']:raise ValueError('Fixture sources changed')
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        try:
            if (out/'wineprefix').exists():
                stopped=subprocess.run(['wineserver','-k'],env=env,timeout=10)
                if stopped.returncode not in (0,1):raise ValueError('Private wineserver stop failed')
                subprocess.run(['wineserver','-w'],env=env,timeout=10,check=True)
            report['private_wine_stopped']=True
        except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,cleanup_error=str(error))
        if server:
            server.terminate()
            try:server.wait(timeout=10)
            except subprocess.TimeoutExpired:server.kill();server.wait()
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
