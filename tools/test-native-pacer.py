#!/usr/bin/env python3
"""Run PE32 native wait-policy guards and unchanged clock/LastError forwarding."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path);args=parser.parse_args()
    parent=ROOT/'working/tests/native-pacer';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report=dict(success=False,sources={p:sha(ROOT/p) for p in ['tools/test-native-pacer.py','tests/campaign-pacer-reference.c','runtime/render/pacer_yield.h','runtime/shadow/win32_min.h','tools/build-render-bridge.py']})
    if args.claims:
        declaration=json.loads(args.claims.read_text());report['claims']=declaration['claims'];report['sources'].update(declaration['sources'])
    if any(sha(ROOT/p)!=h for p,h in report['sources'].items()):raise ValueError('Declared source changed before execution')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    try:
        with (out/'build.log').open('w') as log:
            subprocess.run(['python3',str(ROOT/'tools/build-render-bridge.py')],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
            subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/campaign-pacer-reference.c'),'-o',str(out/'fixture.obj')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
            subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{out / "fixture.exe"}',str(out/'fixture.obj'),str(ROOT/'working/build/render/kernel32.lib')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
        with (out/'wine.log').open('w') as log:
            result=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        report.update(exit_code=result.returncode,binary_sha256=sha(out/'fixture.exe'),checks='18 mutated instruction bytes; wrong base/import/null import; enabled, inactive, unrelated caller; unchanged original clock bits/entry and exit LastError; exactly one Sleep0')
        if result.returncode:raise RuntimeError('Native wait policy fixture failed')
        report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        if not report['sources_unchanged']:raise RuntimeError('Source changed during fixture')
        report['success']=True
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if (out/'wineprefix').exists():subprocess.run(['wineserver','-k'],env=env,timeout=10,check=True)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
