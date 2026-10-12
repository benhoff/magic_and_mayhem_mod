#!/usr/bin/env python3
"""Source-bound dedicated render-owner controls; no original/live equivalence."""
import hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parent=ROOT/'working/tests/native-render-worker';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Render owner evidence:',out,flush=True)
    subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','HOST.native-render-owner','--scenario','HOST.native-render-owner.fixture','--behavior','HOST.native-display-deadlines','--scenario','HOST.native-display-deadlines.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
    declared=json.loads((out/'claims.json').read_text());report=dict(success=False,scope=__doc__,claims=declared['claims'],sources=declared['sources'])
    env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
    for key in ['MESA_LOADER_DRIVER_OVERRIDE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']:env.pop(key,None)
    try:
        build=ROOT/'working/build/qt-shell'
        with (out/'build.log').open('x') as log:subprocess.run(['cmake','--build',str(build),'--target','native-render-worker-test','gpu-presentation-test','--parallel','4'],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        for target in ['native-render-worker-test','gpu-presentation-test']:
            with (out/(target+'.log')).open('x') as log:subprocess.run(['xvfb-run','-a','-e',str(out/(target+'-xvfb.log')),str(build/target)],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
            report[target]=json.loads((out/(target+'.log')).read_text().splitlines()[-1])
        report['sources_unchanged']=all(sha(ROOT/path)==digest for path,digest in report['sources'].items())
        report['success']=report['sources_unchanged'] and all(report[target]['success'] for target in ['native-render-worker-test','gpu-presentation-test'])
    except (OSError,ValueError,IndexError,subprocess.SubprocessError) as error:report['error']=str(error)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Report:',out/'report.json',flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
