#!/usr/bin/env python3
"""Execute bounded caller-census, independent GL-copy and witness refusal controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build',type=Path,required=True);p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--output-parent',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing historical report overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    if any(sha(ROOT/n)!=h for n,h in sources.items()):p.error('Prospective sources changed')
    parent=a.output_parent.resolve()
    if not parent.is_relative_to(ROOT/'working'):p.error('Use owned working output')
    parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text(__doc__+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
    report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(out),scope=__doc__,checks={})
    def run(name,command):
        with (out/(name+'.log')).open('xb') as log:subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=log,check=True,timeout=180)
        return (out/(name+'.log')).read_text()
    try:
        run('census-build',['clang','-std=c11','-Wall','-Wextra','-Werror','-pedantic',ROOT/'tests/native-gpu-copy-census-test.c','-o',out/'census-test'])
        report['checks']['census']=json.loads(run('census',[out/'census-test']).splitlines()[-1])
        run('routes-build',['clang','-std=c11','-Wall','-Wextra','-Werror','-pedantic',ROOT/'tests/native-copy-routes-test.c','-o',out/'routes-test'])
        report['checks']['routes_census']=json.loads(run('routes',[out/'routes-test']).splitlines()[-1])
        run('routes-refusals',[sys.executable,'-B',ROOT/'tests/test-native-copy-routes.py','-v'])
        report['checks']['routes_refusals']=dict(success=True,tests=10,log_sha256=sha(out/'routes-refusals.log'))
        binary=a.build/'render-copy-diagnostics-test';binary_hash=sha(binary)
        report['checks']['native_gl']=json.loads(run('native-gl',['xvfb-run','-a',binary]).splitlines()[-1])
        report['binary_sha256']=binary_hash
        if sha(binary)!=binary_hash:raise ValueError('Control binary changed')
        run('witness-refusals',[sys.executable,'-B',ROOT/'tests/test-native-gpu-copy-diagnostics.py','-v'])
        report['checks']['witness_refusals']=dict(success=True,tests=8,log_sha256=sha(out/'witness-refusals.log'))
        report['success']=all(check.get('success') is True for check in report['checks'].values())
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({k:report[k] for k in ('success','sources_stable','artifacts')}));return int(not report['success'])


if __name__=='__main__':raise SystemExit(main())
