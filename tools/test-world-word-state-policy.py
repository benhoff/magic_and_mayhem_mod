#!/usr/bin/env python3
"""Sanitized atomic World WORD state admission controls; no original execution."""
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
    names=set(claims['sources'])|{'tools/test-world-word-state-policy.py'}
    frozen={p:(ROOT/p).read_bytes()for p in names}
    digest=lambda raw:hashlib.sha256(raw).hexdigest()
    sources={p:digest(b)for p,b in frozen.items()}
    if any(sources[p]!=h for p,h in claims['sources'].items()):raise ValueError('Prospective sources changed')
    report=dict(schema=1,success=False,kind='world_word_state_policy_controls',claims=claims['claims'],sources=sources,
        scope='ASan/UBSan atomic admission/refusal and positive/empty commit controls for native World WORD state. No original executable, game, driver or whole-operation execution.',commands=[],original_game_runs=0,newly_passed_requirements=0,
        earlier_manual_attempt='The same unit assertions ran under default ASan leak checking, then LeakSanitizer refused the sandbox ptrace environment. This is a transcript-level observation; no raw log or engine proof is claimed for that attempt. The controlled execution below disables leak checking and retains its actual output and binary.')
    def retain(label,raw):
        p=PREFIX+args.name+'-'+label;write_report(ROOT,p,raw)
        return dict(path=p,sha256=digest(raw),bytes=len(raw))
    with tempfile.TemporaryDirectory(prefix='mnm-world-word-policy-',dir='/tmp')as temporary:
        out=Path(temporary)
        def run(label,argv,env=None):
            r=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,capture_output=True,timeout=120)
            report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,
                log=retain(label+'.log.bin',r.stdout+r.stderr)))
            if r.returncode:raise RuntimeError(label+': exit '+str(r.returncode))
            return r.stdout
        try:
            report['source_preimages']={}
            for p,b in frozen.items():
                old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
                if old.returncode or old.stdout!=b:report['source_preimages'][p]=retain('source-'+digest(p.encode())[:16]+'.bin',b)
            run('compiler',['gcc','--version'])
            binary=out/'unit'
            run('build',['gcc','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-std=c11',
                '-Wall','-Wextra','-Werror','-pedantic',ROOT/'tests/world-word-state-test.c',
                ROOT/'reconstruction/rendering/word_backend_state.c','-o',binary])
            report['tested_binary']=retain('unit.bin',binary.read_bytes())
            stdout=run('controls',[binary],dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
            if stdout!=b'4 atomic state refusals; positive and empty commits pass\n':raise ValueError('Incomplete policy controls')
            report.update(atomic_refusals=4,positive_and_empty_commits=True,success=True)
            # Retain the already-built production artifact with exact builder
            # source checks. This is linking evidence, not DLL execution.
            directory=ROOT/'working/build/scene-observer'
            metadata=(directory/'manifest.json').read_bytes();build=json.loads(metadata)
            dll=(directory/'MnmScene.dll').read_bytes()
            if digest(dll)!=build['sha256']or any(digest((ROOT/p).read_bytes())!=h for p,h in build['sources'].items()):
                raise ValueError('Scene build artifact/source identity differs')
            report['scene_link']=dict(manifest=retain('scene-build.bin',metadata),
                dll=retain('MnmScene.dll.bin',dll),sources_verified=True,execution=False)
        except (OSError,ValueError,RuntimeError,subprocess.SubprocessError)as error:
            report.update(success=False,error=str(error))
        finally:
            report['sources_stable']=all((ROOT/p).read_bytes()==b for p,b in frozen.items())
            report['success']=report['success']and report['sources_stable']
            write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report[k]for k in ('success','atomic_refusals','error')if k in report}))
    return 0 if report['success']else 1

if __name__=='__main__':raise SystemExit(main())
