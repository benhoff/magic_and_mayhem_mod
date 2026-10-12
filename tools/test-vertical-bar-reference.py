#!/usr/bin/env python3
"""Compare bounded reconstruction with pinned original x86 bar generator bytes."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources']
 if any(sha(ROOT/n)!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
 parent=ROOT/'working/tests/vertical-bar-reference';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),source_executable_sha256=BUILD,original_game_executed=False,original_pixels_used_as_native_inputs=False,scope='Isolated32-bit x86 execution of exact No-CD58f890generator bytes against native bounded logical-parameter reconstruction; both RGB565/555 and surrounding storage. No whole-game draw/lifetime/physical or replacement proof.')
 def execute(label,argv):
  with (run/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in argv],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
 execute('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
  if sha(pe)!=BUILD:raise ValueError('Executable hash changed')
  image=pe.read_bytes();offset=struct.unpack_from('<I',image,60)[0];sections=offset+24+struct.unpack_from('<H',image,offset+20)[0];code=None
  for k in range(struct.unpack_from('<H',image,offset+6)[0]):
   rva,size,raw=struct.unpack_from('<3I',image,sections+k*40+12)
   if rva<=0x18f890 and 0x18f9b9<=rva+size:code=image[raw+0x18f890-rva:raw+0x18f9b9-rva];break
  if code is None or code[:8]!=bytes.fromhex('83ec08538bd95556') or code[-8:]!=bytes.fromhex('5f5e5d5b83c408c3'):raise ValueError('Original generator signature changed')
  (run/'vertical-bar-original.inc').write_text('static const unsigned char original_bar[]={'+','.join(str(x) for x in code)+'};\n')
  report['original_function']=dict(address=0x58f890,bytes=len(code),sha256=hashlib.sha256(code).hexdigest())
  execute('build',['gcc','-m32','-O2','-std=c11','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-I',run,ROOT/'tests/vertical-bar-reference-test.c','-o',run/'reference'])
  execute('comparison',[run/'reference']);report.update(json.loads((run/'comparison.log').read_text()));report['binary_sha256']=sha(run/'reference')
 except Exception as error:report.update(success=False,error=repr(error))
 finally:
  execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','cases','words_compared','error')}))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
