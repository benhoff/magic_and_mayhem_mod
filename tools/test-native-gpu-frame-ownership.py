#!/usr/bin/env python3
"""Source-bound native GPU history proof; live acknowledgement is a later gate."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--corpus',type=Path,required=True)
    args=parser.parse_args();decl=json.loads(args.claims.read_text())
    for name,digest in decl['sources'].items():
        if sha(ROOT/name)!=digest:raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/native-gpu-frame-ownership';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,claims=decl['claims'],sources=decl['sources'],
      scope='Source-only native GPU retained history versus independent native CPU composition, synthetic refusal/copy tests and retained32 original-input queues. No original live acknowledgement, independent original pixel equivalence, DirectDraw retirement or FPS assertion.',
      live_replacement=False,original_destination_pixels_used_as_inputs=False)
    inputs={};before=False
    env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'}
    def run(label,command,timeout=240):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=ROOT,env=env,
              stdout=log,stderr=subprocess.STDOUT,timeout=timeout,check=True)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        game=ROOT/'working/game-nocd';corpus=args.corpus.resolve()
        inputs[str(corpus)]=sha(corpus)
        for path in sorted(game.resolve().rglob('*')):
            if path.is_file():inputs[str(path)]=sha(path)
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'compat/legacy/world-rolling','-B',build,
          '-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -g','-DBUILD_TESTING=ON'])
        run('build',['cmake','--build',build,'--target','world-gpu-resident-test',
          'world-gpu-resident-replay','world-continuous-normal-test','-j4'])
        closure=set()
        for path in build.rglob('*.o.d'):
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(str(p.relative_to(ROOT)))
        report['compiled_sources']={n:sha(ROOT/n) for n in sorted(closure)}
        if closure-set(report['sources']):raise ValueError('Undeclared compiler closure: '+repr(sorted(closure-set(report['sources']))))
        report['native_binaries']={n:sha(build/n) for n in ['world-gpu-resident-test','world-gpu-resident-replay','world-continuous-normal-test']}
        run('synthetic',['xvfb-run','-a',build/'world-gpu-resident-test'])
        report['synthetic_512_passed']=True
        run('legacy-normal',['xvfb-run','-a',build/'world-continuous-normal-test'])
        report['legacy_512_passed']=True
        run('retained-original',['xvfb-run','-a',build/'world-gpu-resident-replay',corpus,game,out/'resident.json'])
        report['resident']=json.loads((out/'resident.json').read_text())
        if report['resident']['queues']!=32:raise ValueError('Expected the retained32-queue corpus')
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except Exception as error:report['manifest_error']=str(error);report['success']=False
        report['inputs']=inputs
        report['inputs_stable']=all(sha(Path(n))==h for n,h in inputs.items())
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
