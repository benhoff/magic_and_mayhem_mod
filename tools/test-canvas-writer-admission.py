#!/usr/bin/env python3
"""Validate opt-in canvas writer DLL admission against synthetic Win32 pages."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from fade_live_capture import load
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text());sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/canvas-writer-admission';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,experiment=str(out.relative_to(ROOT)),original_work_bypassed=False,live_replacement=False,
        scope='Synthetic PE32 DLL admission: explicit default-off/invalid-mode/signature refusal before patch, writable-page/guard-page/cross-region preflight, width/height/pitch/mask/phase/local-lock guards, LastError, native physical prefix/FX-independent entry metadata and capture128-call budget. No original executable or game run, owner-thread race or arbitrary DirectDraw lifetime proof.')
    commands=[];env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    def run(label,argv,environment=None):
        log=out/(label+'.log');result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180,env=environment);log.write_text(result.stdout+result.stderr);commands.append(dict(label=label,returncode=result.returncode,argv=[str(a) for a in argv],log=str(log.relative_to(ROOT)),sha256=sha(log)));result.check_returncode()
    try:
        run('build',['python3',ROOT/'tools/build-canvas-writers.py','--selftest'])
        build=ROOT/'working/build/canvas-writers-selftest'
        for name in ['selftest.exe','MnmCanvasWriters.dll','manifest.json']:shutil.copy2(build/name,out/name)
        run('clone-prefix',['cp','-a','--reflink=auto',ROOT/'working/wineprefix-x86_64',out/'wineprefix'])
        run('wineboot',['wineboot','-u'],env)
        for mode in ['off','bad','anchor','on']:
            capture=out/(mode+'.bin');local={**env,'MNM_FADE_CAPTURE':'Z:'+str(capture).replace('/','\\')}
            if mode!='off':local['MNM_FADE_TAKEOVER']='broken' if mode=='bad' else '1'
            if mode=='anchor':local['MNM_WRITER_TEST_BAD_ANCHOR']='1'
            run(mode,['wine',out/'selftest.exe'],local)
            if mode!='on' and capture.exists():raise ValueError('Refused/disabled DLL opened capture')
        rows=load(out/'on.bin')
        if len(rows)!=128 or rows[-1]['native']!=128 or rows[-1]['forwarded']!=14:raise ValueError('Completion/refusal/budget accounting')
        for row in rows:
            before=list(__import__('struct').unpack('<8H',row['before']));expected=[((p>>1)&0x7bef) if i<4 else p for i,p in enumerate(before)]
            if row['after']!=__import__('struct').pack('<8H',*expected):raise ValueError('Native physical prefix mismatch')
        report.update(success=True,handled=128,atomic_refusals=15,disabled_and_install_refusals=3,compared_words=128*8)
    except Exception as error:report['error']=repr(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=10,check=False)
        report.update(commands=commands,sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()));report['success']=report['success'] and report['sources_stable']
        report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
