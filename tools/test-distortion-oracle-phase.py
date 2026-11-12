#!/usr/bin/env python3
"""Exercise actual original phase/state versus effective-table reconstruction."""
import argparse,json,hashlib,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
from coverage_claims import behavior_contract
ROOT=Path(__file__).resolve().parents[1]
NAMES=['tools/test-distortion-oracle-phase.py','tests/distortion-phase-reference.cpp','tests/world-raster-reference.hpp','tests/sprite-binary-reference.cpp','tools/coverage_claims.py']
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--executable',type=Path,required=True);a=p.parse_args()
 if (ROOT/a.report).exists() or a.output.exists():p.error('New report/output required')
 a.output.mkdir(parents=True);result=dict(schema=1,success=False,sources={n:sha((ROOT/n).read_bytes()) for n in NAMES},original_pixels_used_as_native_inputs=False)
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in register['behaviors'] if x['id']=='TOOL.original-distortion-oracle-phase')
 result['claims']=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in register['builds']}),scenarios={})]
 result['source_records']={}
 for n in NAMES:
  record=str(Path(a.report).with_suffix(''))+'-source-'+sha(n.encode())[:16]+'.bin'
  result['source_records'][n]=dict(record=record,sha256=write_report(ROOT,record,(ROOT/n).read_bytes()))
 before=False
 try:
  subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.PIPE);before=True
  result['source_executable_sha256']=sha(a.executable.read_bytes())
  if result['source_executable_sha256']!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Unsupported original PE')
  binary=a.output/'reference';command=['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/distortion-phase-reference.cpp','-o',binary]
  built=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60);result['build']=dict(command=list(map(str,command)),returncode=built.returncode,output=built.stdout+built.stderr)
  if built.returncode:raise ValueError('Original fixture compile failed')
  result['binary_sha256']=sha(binary.read_bytes());run=subprocess.run([binary,a.executable],capture_output=True,text=True,timeout=60)
  result['returncode']=run.returncode;result['output']=run.stdout+run.stderr
  if run.stdout.strip():result['comparison']=json.loads(run.stdout)
  result['success']=run.returncode==0 and result['comparison']['success']
 except Exception as e:result['error']=str(e)
 finally:
  if before:
   done=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=subprocess.PIPE);result['original_manifest_verified_before_after']=done.returncode==0;result['success']&=done.returncode==0
  result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items());result['success']&=result['sources_stable']
  write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
 print(json.dumps({k:result.get(k) for k in ['success','error','comparison','sources_stable','output']}),flush=True)
 return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
