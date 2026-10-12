#!/usr/bin/env python3
"""Source-bound native GPU baseline and prepared World replay controls."""
import hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parent=ROOT/'working/tests/native-prepared-world';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Prepared World evidence:',out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.native-prepared-world-replay','--scenario','NR.native-prepared-world-replay.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
 d=json.loads((out/'claims.json').read_text());r=dict(success=False,scope=__doc__,claims=d['claims'],sources=d['sources']);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
 for k in ['MESA_LOADER_DRIVER_OVERRIDE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']:env.pop(k,None)
 try:
  with (out/'original-before.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  source=ROOT/'working/tests/world-owned-snapshots/run-d5jy7cxs/source.bin';request=source.with_name('request.bin');assets=ROOT/'working/experiments/menu-observer/run-zg7i_g8n/game';r['inputs']=dict(source=str(source),source_sha256=sha(source),request=str(request),request_sha256=sha(request),assets=str(assets))
  build=ROOT/'working/build/qt-shell'
  with (out/'build.log').open('x') as log:
   subprocess.run(['cmake','-S',str(ROOT/'apps/qt-shell'),'-B',str(build),'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
   subprocess.run(['cmake','--build',str(build),'--target','native-prepared-world-test','canvas-world-test','--parallel','4'],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  with (out/'fixture.log').open('x') as log:subprocess.run(['xvfb-run','-a','-e',str(out/'xvfb.log'),str(build/'world-rolling/native-prepared-world-test'),str(source),str(request),str(assets)],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
  r['controls']=json.loads((out/'fixture.log').read_text().splitlines()[-1]);r['success']=True
 except (OSError,ValueError,IndexError,subprocess.SubprocessError) as e:r['error']=str(e)
 finally:
  try:
   with (out/'original-after.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
   r['original_manifest_verified_after']=True
  except (OSError,subprocess.SubprocessError) as e:r.update(success=False,integrity_error=str(e))
  r['sources_unchanged']=all(sha(ROOT/p)==h for p,h in d['sources'].items())
  if not r['sources_unchanged']:r.update(success=False,error='Prepared replay sources changed during controls')
  (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
