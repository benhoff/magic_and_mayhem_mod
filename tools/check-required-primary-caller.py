#!/usr/bin/env python3
"""Pin the natural indexed-primary branch and exercise exact caller admission."""
import argparse,hashlib,json,subprocess,tempfile
from pathlib import Path
from coverage_evidence import write_report
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/required-boundaries-results-20261110/'

def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args()
 assert a.report.startswith(PREFIX) and not(ROOT/a.report).exists()
 names=['tools/check-required-primary-caller.py','tests/world-indexed-primary-continuation.cpp',
  'tests/world-raster-callers-test.cpp','tests/sprite-binary-reference.cpp','runtime/scene/world_raster_callers.h',
  'tools/inspect-world-raster-callers.py','runtime/scene/world_generic_state.h']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in names}
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(v for v in reg['behaviors']if v['id']=='NP.world-indexed-primary-caller')
 names=sorted(set(names)|required_sources(b));sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in names}
 result=dict(schema=1,kind='required_indexed_primary_caller_controls',original_sha256='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',success=False,sources=sources,commands=[],claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}))],original_game_runs=0,scope=b['scope'])
 def run(label,argv):
  q=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,timeout=120);record=a.report.removesuffix('.json')+'-'+label+'.log'
  result['commands'].append(dict(argv=list(map(str,argv)),exit_code=q.returncode,record=record,sha256=write_report(ROOT,record,q.stdout+q.stderr)))
  if q.returncode:raise ValueError(label+': '+q.stderr.decode(errors='replace')[-2000:])
  return q.stdout
 original=ROOT/'original/Arcane_Nocd/Chaos.exe';verified=False
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  assert hashlib.sha256(original.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
  with tempfile.TemporaryDirectory(prefix='mnm-required-primary-')as tmp:
   for label,source,flags in [('fragment','tests/world-indexed-primary-continuation.cpp',['-m32','-fno-pie','-no-pie']),('guards','tests/world-raster-callers-test.cpp',[])]:
    exe=Path(tmp)/label;run(label+'-build',['g++','-std=c++17','-O2',*flags,ROOT/source,'-o',exe]);result[label]=json.loads(run(label,[exe,original]))
    record=a.report.removesuffix('.json')+'-'+label+'.bin';write_report(ROOT,record,exe.read_bytes())
  result['success']=result['fragment']['success'] and result['guards']['success']
 except Exception as error:result['error']=str(error)
 finally:
  if verified:run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
  result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());result['success'] &= result['sources_stable']
  for n in names:write_report(ROOT,a.report.removesuffix('.json')+'-source-'+hashlib.sha256(n.encode()).hexdigest()[:16]+'.bin',(ROOT/n).read_bytes())
  h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'),fragment=result.get('fragment'),guards=result.get('guards'))))
 return not result['success']
if __name__=='__main__':raise SystemExit(main())
