#!/usr/bin/env python3
"""Exercise scoped stale-image crop against actual owned XCB SHM pixels."""
import argparse,json,os,subprocess,tempfile
from pathlib import Path
from drawing_corpus import ROOT,sha

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources']
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
 parent=ROOT/'working/tests/egl-xcb-crop';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));binary=run/'fixture'
 subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-egl-xcb-crop-test.c'),'-lxcb','-lxcb-shm','-o',str(binary)],check=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);r=subprocess.run(['xvfb-run','-a','-s','-screen 0 1280x1024x24',str(binary)],env=env,capture_output=True,timeout=60);(run/'output.log').write_bytes(r.stdout+r.stderr)
 outcome=json.loads(r.stdout) if r.returncode==0 else {};stable=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());success=stable and not r.returncode and outcome.get('success') is True and outcome.get('cases')==5 and outcome.get('stale_uncorrected',0)>0 and outcome.get('stale_corrected')==0 and outcome.get('refusals')==6
 result=dict(schema=1,success=success,sources=sources,claims=claims['claims'],sources_stable=stable,artifacts=str(run.relative_to(ROOT)),original_execution=False,exit_code=r.returncode,outcome=outcome,binary_sha256=sha(binary.read_bytes()),scope='Five actual owned SHM image uploads with independent known-pattern pixel verification: stale-buffer negative/corrected positive, unchanged nominal, fresh shrink, growth and repeat; six unsupported identity/partial-route refusals. Explicit pinned software-resize policy, not universal image-origin inference.')
 with a.report.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
 print(json.dumps(dict(success=success,exit_code=r.returncode,outcome=outcome)));return int(not success)
if __name__=='__main__':raise SystemExit(main())
