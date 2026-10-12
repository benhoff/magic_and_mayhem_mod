#!/usr/bin/env python3
"""Execute the bounded real X11 metadata-preload forwarding fixture."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_x11_delivery import decode_delivery
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources']
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
 parent=ROOT/'working/tests/drawing-x11-delivery';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 spec=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-x11-delivery.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),original_game_executed=False,original_pixels_used_as_native_inputs=False,scope='Actual private Xvfb selected X11 calls, identical control/observed results,errno and server pixels; strict paired metadata and malformed admissions. No original Wine/game,all routes,driver loss or presented-frame proof.')
 try:
  library=module.build();binary=run/'fixture';subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-x11-delivery-test.c'),'-lX11','-lXext','-o',str(binary)],check=True)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None)
  control=subprocess.run(['xvfb-run','-a',str(binary)],env=env,capture_output=True,check=True,timeout=30);(run/'control.log').write_bytes(control.stdout+control.stderr)
  observed=subprocess.run(['xvfb-run','-a','env','LD_PRELOAD='+str(library),'MNM_DRAW_X11_DIRECTORY='+str(run),str(binary)],env=env,capture_output=True,check=True,timeout=30);(run/'observed.log').write_bytes(observed.stdout+observed.stderr)
  if control.stdout!=observed.stdout:raise ValueError('Original X11 result/errno/server-pixel output changed')
  decoded=decode_delivery((run/'x11-delivery.bin').read_bytes())
  if set(decoded['methods'])!={'XPutImage','XShmPutImage','XCopyArea','XCopyPlane','XFillRectangle','XFillRectangles','XClearArea','XFlush','XSync'}:raise ValueError('Wrapped X11 route omitted from fixture')
  subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-x11-delivery.py')],check=True)
  report.update(success=True,control_sha256=sha(control.stdout),observed_sha256=sha(observed.stdout),library_sha256=sha(library.read_bytes()),binary_sha256=sha(binary.read_bytes()),observation=decoded,result_errno_and_pixels_preserved=True)
 except Exception as error:report['error']=repr(error)
 report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
