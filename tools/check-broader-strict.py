#!/usr/bin/env python3
"""Admit strict readiness separately from independently compared viewport pixels."""
import argparse,importlib.util,json,math
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_acceptance import native_phase
from campaign_functional_acceptance import validate_strict_functional
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def assess(r,name):
 a=r.json(name);phase=a['broader_phase'];require(r.digest(phase['record'])==phase['sha256'],'Strict phase changed');p=r.json(phase['record']);case=next(c for c in p['broader_cases']if c['report']==name)
 require(a['success']and a['delegate_success']and a['sources_stable']and a['actual_provider_verified']and a['actual_start_count_complete']and a['original_game_runs']==1,'Incomplete strict execution')
 require(all(r.digest(n)==h for n,h in a['sources'].items()),'Current strict source changed')
 proof=validate_strict_functional(a);require(a['stress_seconds']==case['stress_seconds']and len(proof['phases'])==2,'Changed strict duration')
 provenance=native_phase(ROOT,r,a,p['player_binary']['sha256']);require(provenance['current_native_source_candidate'],'Historical strict native input')
 samples=a['resident']['stage_profile'];rows=samples['samples'];require(samples['samples_dropped']==0 and len(rows)==int(a['resident']['completed'])and [int(v['generation'])for v in rows]==list(range(1,len(rows)+1)),'Incomplete strict source timing')
 maxima={k:max(float(v[k])for v in rows)for k in('producer_wait_ack_ms','native_consume_ms')}
 require(all(math.isfinite(float(v[k]))and 0<=float(v[k])<120000 for v in rows for k in maxima),'Strict source acknowledgement deadline exceeded')
 launch=a['player_launch'];require(r.digest(launch['record'])==launch['sha256'],'Strict executable launch changed');l=r.json(launch['record']);require(l['success']and l['binary_stable']and not l['player_reconfigured_or_rebuilt'],'Wrong executable launch')
 global_ref=p['broader_plan'];require(r.digest(global_ref['record'])==global_ref['sha256'],'Strict global plan changed');g=r.json(global_ref['record']);fixture=r.json(g['owned_input']['record']);initial=fixture['input_fingerprints'];actual=a['installation_inputs'];allowed=set(fixture['permitted_mutations']);require(actual.keys()==initial.keys()and all(actual[k]==v for k,v in initial.items()if k not in allowed),'Strict original fixture changed')
 return dict(schema=1,kind='current_broader_strict_campaign_validation',success=True,sources_stable=True,original_game_runs=0,actual_acquisition_original_games=1,acquisition=dict(record=name,sha256=r.digest(name)),profile=case['profile'],provider=case['provider'],stress_seconds=case['stress_seconds'],size=case['size'],actual_driver=a['resident']['renderer'],current_native_phase=provenance,functional_acceptance=proof,response_progress=dict(verified=True,samples=len(rows),maximum_observed_ms=maxima,source_ack_deadline_ms=120000),sources=a['sources']|{'tools/check-broader-strict.py':r.digest('tools/check-broader-strict.py')},scope='Strict B05 readiness/lifecycle only. No viewport comparison or fixed FPS credit.')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh strict validation required')
 with Reader(ROOT)as r:
  try:q=assess(r,args.acquisition)
  except(ValueError,KeyError,OSError,TypeError)as e:q=dict(schema=1,kind='current_broader_strict_campaign_validation',success=False,original_game_runs=0,error=str(e),acquisition=dict(record=args.acquisition,sha256=r.digest(args.acquisition)))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
