#!/usr/bin/env python3
"""Exercise exact bounded GLX provider ownership using independent fake DSOs."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_x11_delivery import decode_delivery
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources'];parent=ROOT/'working/tests/drawing-glx-aliases';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),original_game_executed=False,original_pixels_used_as_native_inputs=False,scope='Synthetic independent native ELF providers test16exact original-function aliases,repeated lookup and swap/proc exhaustion before17thoriginal dispatch. No real graphics/driver/game or presentation proof.')
 try:
  if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
  spec=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-x11-delivery.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);library=module.build(True)
  provider=run/'provider.so';subprocess.run(['gcc','-O2','-fPIC','-shared','-Wl,-Bsymbolic-functions','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-glx-alias-provider.c'),'-o',str(provider)],check=True)
  for i in range(17):shutil.copyfile(provider,run/f'provider-{i:02d}.so')
  binary=run/'fixture';subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-glx-alias-test.c'),'-ldl','-o',str(binary)],check=True)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);cases=[]
  for mode,expected in [('swap',0),('proc',0),('exhaust',97),('proc-exhaust',97)]:
   target=run/mode;target.mkdir();result=subprocess.run([str(binary),str(run),mode],env=dict(env,LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(target)),capture_output=True,timeout=30);(target/'fixture.log').write_bytes(result.stdout+result.stderr)
   if result.returncode!=expected or expected==0 and result.stdout!=b'MNM-GLX-ALIASES-PASS-16-EXACT\n':raise ValueError('Alias fixture failed: '+mode+' '+str(result.returncode))
   if expected and b'aliases exhausted' not in result.stderr:raise ValueError('Wrong alias refusal: '+mode)
   decoded=decode_delivery((target/'x11-delivery.bin').read_bytes())
   if decoded['methods']!={'glXSwapBuffers':32} or len({c['arguments'][1] for c in decoded['calls']})!=16:raise ValueError('Original alias ownership/count changed')
   cases.append(dict(mode=mode,exit=result.returncode,swaps=32,exact_aliases=16))
  report.update(success=True,cases=cases,original_functions_forwarded_once=True,repeated_lookup_owns_same_provider=True,alias_exhaustion_refused_before_original_dispatch=True,library_sha256=sha(library.read_bytes()),provider_sha256=sha(provider.read_bytes()),binary_sha256=sha(binary.read_bytes()))
 except Exception as error:report['error']=repr(error)
 report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
