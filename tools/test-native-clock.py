#!/usr/bin/env python3
"""Run native real uptime forwarding guards and actual Wine clock sampling."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import struct
from collections import Counter
ROOT=Path(__file__).resolve().parents[1]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path);args=parser.parse_args()
    parent=ROOT/'working/tests/native-clock';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report=dict(success=False,sources={p:sha(ROOT/p) for p in ['tools/test-native-clock.py','tests/campaign-clock-reference.c','runtime/render/precise_clock.h', 'runtime/render/pacer_yield.h','runtime/shadow/win32_min.h','tools/build-render-bridge.py']})
    if args.claims:
        declaration=json.loads(args.claims.read_text());report['claims']=declaration['claims'];report['sources'].update(declaration['sources'])
    if any(sha(ROOT/p)!=h for p,h in report['sources'].items()):raise ValueError('Declared source changed before execution')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    try:
        with (out/'build.log').open('w') as log:
            subprocess.run(['python3',str(ROOT/'tools/build-render-bridge.py')],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
            subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/campaign-clock-reference.c'),'-o',str(out/'fixture.obj')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
            (out/'winmm.def').write_text('LIBRARY WINMM.dll\nEXPORTS\ntimeGetTime@0\n')
            subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(out/'winmm.def'),'-l',str(out/'winmm.lib')],stdout=log,stderr=subprocess.STDOUT,check=True)
            subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{out / "fixture.exe"}',str(out/'fixture.obj'),str(ROOT/'working/build/render/kernel32.lib'),str(out/'winmm.lib')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
        with (out/'wine.log').open('w') as log:
            result=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        report.update(exit_code=result.returncode,binary_sha256=sha(out/'fixture.exe'),checks='Original entry/exit LastError preserved with absent/real fine clock; original forwarding retained when disabled; natural DWORD wrap and32ms epoch guard;1024 real WinMM/GetTickCount samples')
        if result.returncode:raise RuntimeError('Native real-clock fixture failed')
        rows=list(struct.iter_unpack('<4I',(out/'clocks.bin').read_bytes()))
        if len(rows)!=1024 or [r[0] for r in rows]!=list(range(1024)):raise ValueError('Invalid real clock samples')
        hist={name:Counter((b[col]-a[col])&0xffffffff for a,b in zip(rows,rows[1:])) for name,col in [('coarse',1),('fine',2)]}
        if min(d for d in hist['fine'] if d)>min(d for d in hist['coarse'] if d) or (1023-hist['fine'][0])<4*(1023-hist['coarse'][0]):raise ValueError('Actual fine clock resolution did not improve')
        report['clock_samples']={'rows':1024,'deltas':{n:dict(h) for n,h in hist.items()},'epoch_delta_range':[min(((r[2]-r[1]+2**31)%2**32)-2**31 for r in rows),max(((r[2]-r[1]+2**31)%2**32)-2**31 for r in rows)],'sha256':sha(out/'clocks.bin')}
        report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        if not report['sources_unchanged']:raise RuntimeError('Source changed during fixture')
        report['success']=True
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if (out/'wineprefix').exists():subprocess.run(['wineserver','-k'],env=env,timeout=10,check=True)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
