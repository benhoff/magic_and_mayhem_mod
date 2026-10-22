#!/usr/bin/env python3
"""Source-bound independent colour arithmetic, caching, eviction and ordering controls."""
from coverage_evidence import read_json as evidence_json
import json,os,subprocess,tempfile
from pathlib import Path
from profile_native_resident_helpers import sha
ROOT=Path(__file__).resolve().parents[1]
def main():
 parent=ROOT/'working/tests/native-colour-cache';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.native-colour-rectangle-cache','--scenario','native-colour-cache-fixture-20261012','--output',str(out/'claims.json')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
 d=evidence_json(out/'claims.json');r=dict(success=False,sources=d['sources'],claims=d['claims'],scope=__doc__);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
 for key in ['MESA_LOADER_DRIVER_OVERRIDE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']:env.pop(key,None)
 def run(label,args):
  with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in args],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
 try:
  build=ROOT/'working/build/qt-shell';run('configure',['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON']);run('build',['cmake','--build',build,'--target','colour-rectangle-cache-test','native-prepared-world-test','mnm-qt-shell','-j4'])
  run('fixture',['xvfb-run','-a',build/'world-rolling/canvas-producers/scenes/colour-rectangle-cache-test']);r['controls']=json.loads((out/'fixture.log').read_text().splitlines()[-1]);run('prepared',['xvfb-run','-a',build/'world-rolling/native-prepared-world-test']);r['prepared_controls']=json.loads((out/'prepared.log').read_text().splitlines()[-1]);r['success']=True
 except (OSError,ValueError,IndexError,subprocess.SubprocessError) as error:r['error']=str(error)
 finally:
  r['sources_stable']=all(sha(ROOT/n)==h for n,h in d['sources'].items());r['success']=r['success'] and r['sources_stable'];(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Colour cache report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
