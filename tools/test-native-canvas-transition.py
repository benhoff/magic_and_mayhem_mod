#!/usr/bin/env python3
"""Source-only resident canvas lifetime, replay and initial-primary controls."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent=ROOT/'working/tests/resident-canvas-transition';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    spec=importlib.util.spec_from_file_location('transition_claims',ROOT/'tools/draft-coverage-claims.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    declaration=module.declaration(ROOT,register,['NR.resident-world-canvas-transition','NP.native-primary-initial-history','NP.native-source-copy-refusal'],['NR.resident-world-canvas-transition.synthetic'])
    for directory in ['assets','renderer','compat','reconstruction','protocols']:
        for path in (ROOT/directory).rglob('*'):
            if path.is_file() and (path.suffix in ('.cpp','.hpp','.c','.h','.S') or path.name=='CMakeLists.txt'):
                declaration['sources'][path.relative_to(ROOT).as_posix()]=sha(path)
    for path in ['tests/resource-fixtures.hpp','tests/native-prepared-world-test.cpp','tests/native-retained-world-test.cpp']:
        declaration['sources'][path]=sha(ROOT/path)
    declaration['sources']['tests/native-world-rolling/CMakeLists.txt']=sha(ROOT/'tests/native-world-rolling/CMakeLists.txt')
    (out/'prospective.json').write_text(json.dumps(declaration,indent=2)+'\n')
    report=dict(success=False,sources=declaration['sources'],claims=declaration['claims'],original_artifacts_consumed=False)
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4')
    def run(name,args):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(v) for v in args],cwd=ROOT,env=env,stdout=log,stderr=log,check=True,timeout=600)
    try:
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/native-world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'])
        targets=['resident-canvas-transition-test','world-gpu-resident-test','native-prepared-world-test','native-retained-world-test']
        run('build',['cmake','--build',build,'--target',*targets,'--parallel','4'])
        for target in targets:run(target,['xvfb-run','-a',build/'world-rolling'/target])
        run('primary-build',['cc','-std=c11','-Wall','-Wextra','-Werror','-MD','-MF',out/'primary.d',ROOT/'tests/native-primary-initial-test.c','-o',out/'primary-test'])
        run('primary-controls',[out/'primary-test'])
        compiled=set()
        for dep in [*build.rglob('*.o.d'),out/'primary.d']:
            for token in shlex.split(dep.read_text().replace('\\\n',' ')):
                path=Path(token).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    compiled.add(path.relative_to(ROOT).as_posix())
        if not compiled or compiled-set(report['sources']):raise ValueError('Undeclared compiler sources: '+repr(sorted(compiled-set(report['sources']))))
        report['compiled_sources']={p:sha(ROOT/p) for p in sorted(compiled)}
        report['transition']=json.loads((out/'resident-canvas-transition-test.log').read_text().splitlines()[-1])
        report['controls']={target:(out/(target+'.log')).read_text() for target in targets[1:]+['primary-controls']}
        report['source_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        if not report['source_stable']:raise ValueError('Declared sources changed during controls')
        report['success']=True
    except Exception as error:
        report['error']=str(error);raise
    finally:
        report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        (out/'.retain').write_text('Resident canvas lifetime synthetic controls; preserve source and compiler receipts.\n')
    print('Resident canvas transition controls passed',flush=True)

if __name__=='__main__':main()
