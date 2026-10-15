#!/usr/bin/env python3
"""Compare bounded actual EGL swap metadata with an unchanged window control."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import struct
from drawing_api_session import sha
from drawing_x11_delivery import decode_delivery
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources'];parent=ROOT/'working/tests/drawing-egl-worker-context';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),original_game_executed=False,original_pixels_used_as_native_inputs=False,scope='Actual private Xvfb EGL window context,direct/named/proc/default-scope swaps with identical control known source pixels,Boolean/errno/EGL error and missing-symbol outcome. No original game,full handle generation/lifetime,alternate swaps or physical-source causality.')
 try:
  if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
  spec=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-x11-delivery.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);library=module.build(egl=True)
  binary=run/'fixture';subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-egl-delivery-test.c'),'-lEGL','-lGLESv2','-lX11','-ldl','-o',str(binary)],check=True)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);env['LIBGL_ALWAYS_SOFTWARE']='1'
  control=subprocess.run(['xvfb-run','-a',str(binary)],env=env,capture_output=True,timeout=30);(run/'control.log').write_bytes(control.stdout+control.stderr)
  if control.returncode:raise ValueError('Unchanged EGL fixture refused: '+str(control.returncode))
  from drawing_worker_context import decode
  cases=[]
  for label,active in [('active',1),('inactive',0),('missing',None)]:
   directory=run/label;directory.mkdir()
   if active is not None:
    words=[0]*16;words[:2]=struct.unpack('<2I',b'MNMWCX01');words[2:6]=[1,64,2,active]
    if active:words[6:11]=[7,8,9,10,11]
    (directory/'wine-worker-context.bin').write_bytes(struct.pack('<16I',*words))
   observed=subprocess.run(['xvfb-run','-a','env','LD_PRELOAD='+str(library),'MNM_DRAW_X11_DIRECTORY='+str(directory),'MNM_DRAW_WINE_WORKER_CONTEXT=1',str(binary)],env=env,capture_output=True,timeout=30);(directory/'observed.log').write_bytes(observed.stdout+observed.stderr)
   if observed.returncode or control.stdout!=observed.stdout:raise ValueError('Workercontext alteredEGLforwarding:'+str(observed.returncode))
   decoded=decode_delivery((directory/'x11-delivery.bin').read_bytes());contexts=decode((directory/'egl-worker-context.bin').read_bytes())
   if decoded['methods'].get('eglSwapBuffers')!=4 or len(contexts['calls'])!=4 or contexts['stable_active_contexts']!=(4 if active else 0) or contexts['missing_or_unstable_contexts']!=(4 if active is None else 0):raise ValueError('Workercontextfixtureadmission')
   cases.append(dict(case=label,contexts=contexts,observed_sha256=sha(observed.stdout)))
  report.update(success=True,control_sha256=sha(control.stdout),observed_sha256=sha(observed.stdout),cases=cases,observation=decoded,library_sha256=sha(library.read_bytes()),binary_sha256=sha(binary.read_bytes()),swap_routes=4,boolean_errno_egl_error_source_pixels_and_missing_symbol_preserved=True)
 except Exception as error:report['error']=repr(error)
 report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
