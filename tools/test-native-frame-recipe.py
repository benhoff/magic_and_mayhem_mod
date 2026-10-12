#!/usr/bin/env python3
"""Source-bound native HUD composition recipe controls, no original/live claim."""
import hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parent=ROOT/'working/tests/native-frame-recipe';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Native UI recipe evidence:',out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.native-prepared-command-frame','--scenario','NR.native-prepared-command-frame.fixture','--behavior','NR.masked-update-scratch-reuse','--scenario','NR.masked-update-scratch-reuse.fixture','--behavior','NR.resident-prepared-shared-budget','--scenario','NR.resident-prepared-shared-budget.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
 d=json.loads((out/'claims.json').read_text());r=dict(success=False,scope=__doc__,claims=d['claims'],sources=d['sources']);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
 for k in ['MESA_LOADER_DRIVER_OVERRIDE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']:env.pop(k,None)
 try:
  build=ROOT/'working/build/qt-shell'
  with (out/'build.log').open('x') as log:subprocess.run(['cmake','--build',str(build),'--target','native-frame-recipe-test','--parallel','4'],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
  with (out/'fixture.log').open('x') as log:subprocess.run(['xvfb-run','-a','-e',str(out/'xvfb.log'),str(build/'renderer/native-frame-recipe-test')],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
  r['controls']=json.loads((out/'fixture.log').read_text().splitlines()[-1]);r['sources_unchanged']=all(sha(ROOT/p)==h for p,h in r['sources'].items());r['success']=r['sources_unchanged']
 except (OSError,ValueError,IndexError,subprocess.SubprocessError) as e:r['error']=str(e)
 (out/'report.json').write_text(json.dumps(r,indent=2)+'\n');print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
