#!/usr/bin/env python3
"""Bind one reviewed pilot execution and its controls to the scoped coverage contract."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_pilot import require
from rendering_pilot_ledger import payload
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('admission','controls','report'):p.add_argument('--'+name,required=True)
 a=p.parse_args()
 if not a.report.startswith('research/runtime/rendering-acceptance-pilot-results-20261031/') or (ROOT/a.report).exists():p.error('New owned immutable report required')
 with Reader(ROOT) as reader:
  admitted=reader.json(a.admission);controls=reader.json(a.controls)
  require(admitted['success'] is True and admitted['sources_stable'] is True and admitted['reopen_and_restore_verified'] is True,'Admission incomplete')
  require(controls['success'] is True and controls['sources_stable'] is True and len(controls['checks'])==4,'Executed controls incomplete')
  require(admitted['snapshot']['decisions']=={'supported-player':'pending','original-recovery':'pending'} and admitted['snapshot']['closes_full_requirement_rows']==0,'Broader decision changed')
  require(len(admitted['snapshot']['events'])==1,'Review this bounded single pilot explicitly')
  value=admitted['snapshot']['events'][0]['payload'];execution=payload(reader,value)
  sources={}
  for phase in (execution['current_sources'],admitted['sources'],controls['sources']):
   for n,h in phase.items():require(n not in sources or sources[n]==h,'Mixed execution source phases');sources[n]=h
  sources['tools/review-rendering-pilot.py']=reader.digest('tools/review-rendering-pilot.py')
  require(all(reader.digest(n)==h for n,h in sources.items()),'Source changed since execution')
  register=reader.json('research/runtime/coverage/register.json');behavior=next(b for b in register['behaviors'] if b['id']=='NP.rendering-acceptance-pilot');scenario=next(s for s in register['scenarios'] if s['id']=='RP31.public-quick')
  require(required_sources(behavior)<=set(sources),'Incomplete actual execution/control source closure: '+str(required_sources(behavior)-set(sources)))
  builds={b['id']:b for b in register['builds']}
  report=dict(schema=1,kind='scoped_public_pilot_execution_join',success=True,sources=sources,original_source_sha256=builds['nocd-40209ca7']['source_sha256'],
   original_manifest_verified_before_after=True,execution=execution,receipts={k:dict(record=v,sha256=reader.digest(v)) for k,v in [('admission',a.admission),('controls',a.controls)]},
   claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,builds),scenarios={scenario['id']:scenario_contract(scenario)})],
   scope=behavior['scope'],player_accepted=False,original_recovery_accepted=False,full_requirement_rows_closed=0,
   limitation='Joins existing current actual execution and synthetic controls for one scoped pipeline. This producer runs no game and performs no complete artifact5 per-requirement aggregation.')
  h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=a.report,sha256=h,sources=len(sources))))
if __name__=='__main__':main()
