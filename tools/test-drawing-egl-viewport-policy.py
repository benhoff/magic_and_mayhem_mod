#!/usr/bin/env python3
"""Owned GL source/resize/viewport-restore controls for the opt-in driver policy."""
import argparse,json,os,subprocess,tempfile
from pathlib import Path
from drawing_corpus import ROOT,sha

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources'];parent=ROOT/'working/tests/egl-viewport-policy';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));binary=run/'fixture'
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Viewport policy source changed')
 subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-egl-resize-forwarding-test.c'),'-lEGL','-lGL','-lX11','-o',str(binary)],check=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);env['LIBGL_ALWAYS_SOFTWARE']='1'
 command=['xvfb-run','-a','-s','-screen 0 1280x1024x24']
 control=subprocess.run([*command,str(binary)],env=env,capture_output=True,timeout=60);(run/'control.log').write_bytes(control.stdout+control.stderr)
 observed=subprocess.run([*command,'env','LD_PRELOAD='+str(ROOT/'working/build/drawing-egl-delivery/libmnm-drawing-egl-delivery.so'),'MNM_DRAW_X11_DIRECTORY='+str(run),'MNM_DRAW_EGL_SWAP_COMPLETION=1','MNM_DRAW_EGL_REFRESH_VIEWPORT=1',str(binary)],env=env,capture_output=True,timeout=60);(run/'observed.log').write_bytes(observed.stdout+observed.stderr)
 stable=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());success=not control.returncode and not observed.returncode and control.stdout==observed.stdout and len(control.stdout.splitlines())==7 and all(b' mismatch 0 ' in row and row.endswith(b'error0') for row in control.stdout.splitlines()) and stable
 report=dict(schema=1,success=success,sources=sources,claims=claims['claims'],sources_stable=stable,original_execution=False,frames=7,pixels_compared=2*(800*600*3+640*480*4),viewport_restored=not observed.returncode,control_exit=control.returncode,observed_exit=observed.returncode,control_sha256=sha(control.stdout),observed_sha256=sha(observed.stdout),binary_sha256=sha(binary.read_bytes()),library_sha256=sha((ROOT/'working/build/drawing-egl-delivery/libmnm-drawing-egl-delivery.so').read_bytes()),artifacts=str(run.relative_to(ROOT)),scope='Owned source RGB565 full-frame comparison across both resize directions and repeated extents, exact original viewport restoration and GL error checks. No original-game or general driver equivalence claim.')
 with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
 print(json.dumps(dict(success=success,control_exit=control.returncode,observed_exit=observed.returncode)));return int(not success)
if __name__=='__main__':raise SystemExit(main())
