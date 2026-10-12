#!/usr/bin/env python3
"""Exercise post-EGL request/ACK with a synthetic peer,not physical frame proof."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time
from drawing_api_session import sha
from drawing_egl_physical import decode_request
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources'];parent=ROOT/'working/tests/drawing-egl-fence';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),original_game_executed=False,original_pixels_used_as_native_inputs=False,physical_frame_proof=False,scope='Actual EGL window fixture forwards through synthetic exact ACKs after swap and preserves control source/Boolean/errno/EGL error; malformed/missing/mode ACK failures are diagnostic,not actual driver failure or physical presentation proof.')
 try:
  if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
  spec=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-x11-delivery.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);library=module.build(egl=True)
  binary=run/'fixture';subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-egl-delivery-test.c'),'-lEGL','-lGLESv2','-lX11','-ldl','-o',str(binary)],check=True)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);env['LIBGL_ALWAYS_SOFTWARE']='1'
  control=subprocess.run(['xvfb-run','-a',str(binary)],env=env,capture_output=True,check=True,timeout=30);(run/'control.log').write_bytes(control.stdout+control.stderr);cases=[]
  for mode in ('exact','wrong-id','oversize','missing','bad-mode'):
   target=run/mode;target.mkdir();extra=dict(LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(target),MNM_DRAW_EGL_PHYSICAL='0' if mode=='bad-mode' else '1')
   with (target/'fixture.log').open('xb') as out:
    process=subprocess.Popen(['xvfb-run','-a','env',*[k+'='+v for k,v in extra.items()],str(binary)],env=env,stdout=subprocess.PIPE,stderr=out);ordinal=1;deadline=time.monotonic()+30;requests=[]
    while process.poll() is None:
     if time.monotonic()>deadline:process.kill();process.wait();raise ValueError('EGL fence fixture timed out')
     request=target/f'egl-physical-request-{ordinal:06d}.bin'
     if request.exists() and request.stat().st_size==128:
      r=decode_request(request.read_bytes());requests.append(r[7]);ack=b'MNMEACK1'+r[4].to_bytes(4,'little')+(r[7]+(mode=='wrong-id')).to_bytes(4,'little')
      if mode=='oversize':ack+=b'0'
      if mode!='missing':
       temporary=target/f'egl-physical-ack-{ordinal:06d}.bin.tmp';temporary.write_bytes(ack);temporary.rename(target/f'egl-physical-ack-{ordinal:06d}.bin')
      ordinal+=1
     time.sleep(.001)
    code=process.wait();output=process.stdout.read();(target/'stdout.log').write_bytes(output)
   expected=0 if mode=='exact' else 97
   if code!=expected:raise ValueError(mode+' unexpected exit '+str(code))
   if mode=='exact' and (output!=control.stdout or len(requests)!=4):raise ValueError('EGL fence changed result/error/source output')
   cases.append(dict(mode=mode,exit=code,requests=len(requests)))
  subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-egl-physical.py')],check=True)
  report.update(success=True,cases=cases,synthetic_ack_only=True,exact_forwarding_boolean_errno_egl_error_and_source=True,negative_ack_and_mode_refusals=True,library_sha256=sha(library.read_bytes()))
 except Exception as error:report['error']=repr(error)
 report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
