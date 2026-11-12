#!/usr/bin/env python3
"""Execute protocol refusal controls and the real i386 observation stubs."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract, required_sources
from required_boundary_observation import PREFIX,decode
ROOT=Path(__file__).resolve().parents[1]

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 assert a.report.startswith(PREFIX) and not (ROOT/a.report).exists()
 names=['tools/check-required-boundary-observation.py','tools/required_boundary_observation.py',
  'tests/test-required-boundary-observation.py','tests/required-boundary-observer.cpp','tests/required-boundary-abi.cpp','tests/required-boundary-abi.S',
  'runtime/scene/required_boundaries.c','runtime/scene/required_boundaries.S',
  'protocols/include/mnm/required_boundaries_v1.h','runtime/shadow/win32_min.h']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(v for v in reg['behaviors'] if v['id']=='NP.required-boundary-observation')
 names=sorted(set(names)|required_sources(behavior))
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
 result=dict(schema=1,kind='required_boundary_observer_controls',success=False,sources=sources,commands=[],
  claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}))],
  scope='Production assembly and metadata decoder controls. No guest game output, exception unwinding, failed-provider completion, all supplier recovery or whole-player claim.')
 def run(label,argv):
  q=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,timeout=120)
  record=a.report.removesuffix('.json')+'-'+label+'.log'
  result['commands'].append(dict(argv=list(map(str,argv)),exit_code=q.returncode,record=record,sha256=write_report(ROOT,record,q.stdout+q.stderr)))
  if q.returncode:raise ValueError(label+': '+q.stderr.decode(errors='replace')[-2000:])
 try:
  run('protocol',['python3',ROOT/'tests/test-required-boundary-observation.py'])
  with tempfile.TemporaryDirectory(prefix='mnm-boundary-abi-') as out:
   binary=Path(out)/'abi'
   run('build',['g++','-m32','-msse2','-O2','-fno-pie','-no-pie',ROOT/'tests/required-boundary-abi.cpp',ROOT/'tests/required-boundary-abi.S',ROOT/'runtime/scene/required_boundaries.S','-o',binary])
   run('abi',[binary]);record=a.report.removesuffix('.json')+'-tested-abi.bin'
   result['binary']=dict(record=record,sha256=write_report(ROOT,record,binary.read_bytes()))
   metadata=Path(out)/'metadata';wire=Path(out)/'metadata.bin'
   run('metadata-build',['clang++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wno-ignored-attributes',ROOT/'tests/required-boundary-observer.cpp',ROOT/'runtime/scene/required_boundaries.S','-o',metadata])
   run('metadata',[metadata,wire]);observed=decode(wire.read_bytes())
   assert len(observed['pairs'])==21 and observed['worlds_observed']==2
   for label,path in [('metadata-binary',metadata),('metadata-wire',wire)]:write_report(ROOT,a.report.removesuffix('.json')+'-'+label+'.bin',path.read_bytes())
   result['production_encoder']=dict(normal_pairs=21,entry_only=2,screen_classes=4,callbacks=2,worlds=2,changes=1,LastError_preserved=True)
  result['success']=True
 except Exception as error:result['error']=str(error)
 finally:
  result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
  result['success'] &= result['sources_stable']
  for n in names:write_report(ROOT,a.report.removesuffix('.json')+'-source-'+hashlib.sha256(n.encode()).hexdigest()[:16]+'.bin',(ROOT/n).read_bytes())
  h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))))
 return not result['success']
if __name__=='__main__':raise SystemExit(main())
