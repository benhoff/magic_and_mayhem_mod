#!/usr/bin/env python3
"""Exercise bounded private snapshot storage reuse and failure/lifetime boundaries."""
import argparse,hashlib,io,json,subprocess,sys,tarfile
from pathlib import Path
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tools'))
from coverage_evidence import write_report
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--report',required=True);args=parser.parse_args()
out=args.output.resolve()
if out.exists() or not out.is_relative_to(root/'working/tests') or (root/args.report).exists():parser.error('Use fresh owned output and report paths')
out.mkdir(parents=True);r=dict(success=False,commands=[],scope='Isolated private storage reuse; no original-game equivalence claim.')
inputs=['runtime/render/snapshot_storage.h','tests/render-snapshot-storage-test.c','tools/test-render-snapshot-storage.py'];r['sources']={p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in inputs}
try:
 for mode,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
  for label,cmd in [(mode+'-build',['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,'-pthread',str(root/inputs[1]),'-o',str(out/mode)]),(mode,[str(out/mode)])]:
   p=subprocess.run(cmd,capture_output=True,timeout=60);(out/(label+'.log')).write_bytes(p.stdout+p.stderr);r['commands'].append(dict(label=label,command=cmd,code=p.returncode));assert not p.returncode,(label,p.stderr.decode());print(label,p.stdout.decode(),flush=True)
 r['success']=True
except Exception as e:r['error']=str(e)
buf=io.BytesIO()
with tarfile.open(fileobj=buf,mode='w:gz') as tar:
 for p in out.iterdir():tar.add(p,arcname=p.name)
base=args.report
r['sources_stable']=all(hashlib.sha256((root/p).read_bytes()).hexdigest()==h for p,h in r['sources'].items());r['success'] &= r['sources_stable']
write_report(root,base+'.tar.gz',buf.getvalue());write_report(root,base,json.dumps(r,indent=2).encode());print(r.get('error',r['success']),flush=True)
sys.exit(not r['success'])
