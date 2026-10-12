#!/usr/bin/env python3
"""Covered-sample admission and captured public boundary geometry controls."""
import json,os,subprocess,tempfile
from pathlib import Path
from profile_native_resident_helpers import sha
ROOT=Path(__file__).resolve().parents[1]
def main():
 parent=ROOT/'working/tests/masked-displacement';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.masked-displacement-admission','--scenario','masked-displacement-fixture-20261012','--output',str(out/'claims.json')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
 d=json.load(open(out/'claims.json'));r=dict(success=False,sources=d['sources'],claims=d['claims'],scope=__doc__);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
 for k in ['MESA_LOADER_DRIVER_OVERRIDE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']:env.pop(k,None)
 def run(label,args):
  with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in args],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
 inputs={};before=False
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
  source=ROOT/'working/tests/native-colour-cache-displacement/source.bin';assets=ROOT/'working/experiments/menu-observer/run-1knatgxy/game';inputs[str(source)]=sha(source);manifest=assets.parent/'input-fingerprints.json';inputs[str(manifest)]=sha(manifest)
  for n,h in json.load(open(manifest)).items():assert sha(assets/n)==h,n;inputs[str(assets/n)]=h
  build=ROOT/'working/build/qt-shell';run('configure',['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON']);run('build',['cmake','--build',build,'--target','masked-displacement-test','scene-renderer-test','sprite-atlas-test','colour-rectangle-cache-test','native-prepared-world-test','mnm-qt-shell','-j4'])
  run('fixture',['xvfb-run','-a',build/'world-rolling/masked-displacement-test',source,assets]);r['controls']=json.loads((out/'fixture.log').read_text().splitlines()[-1])
  for name,path in [('scene',build/'world-rolling/canvas-producers/scenes/scene-renderer-test'),('atlas',build/'world-rolling/canvas-producers/scenes/sprite-atlas-test'),('colour',build/'world-rolling/canvas-producers/scenes/colour-rectangle-cache-test'),('prepared',build/'world-rolling/native-prepared-world-test')]:run(name,['xvfb-run','-a',path])
  r['regressions']=4;r['success']=True
 except (OSError,ValueError,IndexError,AssertionError,subprocess.SubprocessError) as error:r['error']=str(error)
 finally:
  if before:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);r['original_manifest_verified_before_after']=True
   except (OSError,subprocess.SubprocessError) as error:r.update(success=False,manifest_error=str(error))
  r['inputs']=inputs;r['inputs_stable']=all(sha(Path(n))==h for n,h in inputs.items());r['sources_stable']=all(sha(ROOT/n)==h for n,h in d['sources'].items());r['success']=r['success'] and r['inputs_stable'] and r['sources_stable'];(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Masked displacement report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
