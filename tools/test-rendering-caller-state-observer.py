#!/usr/bin/env python3
"""Sanitized caller-state codec and reader controls; no original execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-results-20261108/'

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name',required=True);parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+',args.name):parser.error('Unique lowercase name required')
    target=ROOT/(PREFIX+args.name+'.json')
    if target.exists():raise ValueError('Preserve immutable evidence')
    claims=json.loads(args.claims.read_text())
    names=set(claims['sources'])|{'tools/test-rendering-caller-state-observer.py'}
    frozen={p:(ROOT/p).read_bytes()for p in names}
    digest=lambda raw:hashlib.sha256(raw).hexdigest()
    sources={p:digest(b)for p,b in frozen.items()}
    if any(sources[p]!=h for p,h in claims['sources'].items()):raise ValueError('Prospective sources changed')
    report=dict(schema=1,success=False,kind='rendering_caller_state_observer_controls',claims=claims['claims'],sources=sources,
        scope='ASan/UBSan semantic FX codec over all2048TOP/tag combinations and fail-closed Python wire reader controls. No original executable, game, state equivalence or installed acceptance claim.',commands=[],original_game_runs=0,newly_passed_requirements=0)

    def retain(label,raw):
        p=PREFIX+args.name+'-'+label;write_report(ROOT,p,raw)
        return dict(path=p,sha256=digest(raw),bytes=len(raw))
    with tempfile.TemporaryDirectory(prefix='mnm-caller-state-codec-',dir='/tmp')as temporary:
        out=Path(temporary)
        def run(label,argv,env=None):
            r=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,capture_output=True,timeout=120)
            report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,
                log=retain(label+'.log.bin',r.stdout+r.stderr)))
            if r.returncode:raise RuntimeError(label+': exit '+str(r.returncode))
            return r.stdout+r.stderr
        try:
            report['source_preimages']={}
            for p,b in frozen.items():
                old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
                if old.returncode or old.stdout!=b:report['source_preimages'][p]=retain('source-'+digest(p.encode())[:16]+'.bin',b)
            run('compiler',['gcc','--version'])
            binary=out/'unit'
            run('build',['gcc','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-std=c11',
                '-Wall','-Wextra','-Werror','-pedantic',ROOT/'tests/drawing-caller-state-codec-test.c','-o',binary])
            report['tested_binary']=retain('unit.bin',binary.read_bytes())
            stdout=run('controls',[binary],dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
            if not stdout.startswith(b'2048 TOP/tag combinations;'):raise ValueError('Incomplete codec controls')
            reader_output=run('reader-controls',['python3',ROOT/'tests/test-drawing-caller-state.py'])
            if b'Ran 72 tests'not in reader_output:raise ValueError('Incomplete versioned reader controls')
            report.update(codec_combinations=2048,reader_controls=72,success=True)

        except (OSError,ValueError,RuntimeError,subprocess.SubprocessError)as error:
            report.update(success=False,error=str(error))
        finally:
            report['sources_stable']=all((ROOT/p).read_bytes()==b for p,b in frozen.items())
            report['success']=report['success']and report['sources_stable']
            write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report[k]for k in ('success','codec_combinations','reader_controls','error')if k in report}))
    return 0 if report['success']else 1

if __name__=='__main__':raise SystemExit(main())
