#!/usr/bin/env python3
"""Freeze the last two unchanged-control recovery requests after setup failures."""
import json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/rendering-transition-recovery-results-20261107/'

def main():
 with Reader(ROOT) as r:
  q=r.json(P+'input-plan-08.json')
  require(q['success'] and all(r.digest(n)==h for n,h in q['sources'].items()),'Previous frozen controls changed')
  failures=[r.json(P+n+'-acquisition.json') for n in ('default640-02','default640-03')]
  require([v['original_game_runs'] for v in failures]==[1,0] and not any(v['success'] for v in failures),'Wrong setup failure accounting')
  require(failures[0]['actions'][0]['name']=='Launch game' and len(failures[0]['actions'])==1 and not failures[0]['epochs_observed'],'Accessibility failure reached runtime actions')
  require(failures[1]['error']=='Private X server closed its display pipe before newline','Wrong display failure')
  with sqlite3.connect(ROOT/'working/transition-attempts.sqlite3') as db:names=[v[0] for v in db.execute('SELECT report FROM attempts')]
  require(len(names)==10 and sum(r.json(n)['original_game_runs'] for n in names)==8,'Unexpected request/start history')
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']=='NP.transition-recovery-acceptance');s=next(v for v in reg['scenarios'] if v['id']=='native.transition-recovery-acceptance')
  q.update(previous_plan=dict(record=P+'input-plan-08.json',sha256=r.digest(P+'input-plan-08.json')),sources={n:r.digest(n) for n in sorted(required_sources(b))},claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})],
   final_setup_review=dict(failures=[dict(record=P+n+'-acquisition.json',sha256=r.digest(P+n+'-acquisition.json')) for n in ('default640-02','default640-03')],reason='First attempt lost accessibility during initial Launch before World/fault. Second command omitted the successful Mesa EGL environment and Xvfb crashed through NVIDIA EGL before any original launch. Preserve and charge both requests and one start. No native/runtime/controller repair; restore the exact successful Mesa environment and verify displays before the last two cases.',native_sources_unchanged=True,controls_unchanged=True),
   repair_phase=dict(maximum_new_requests=2,maximum_new_original_starts=4,requests_before=10,original_starts_before=8,no_800_recollection=True,no_further_expansion=True))
  q['bounds'].update(maximum_requests=12,maximum_original_games=12)
  print(write_report(ROOT,P+'input-plan-09.json',(json.dumps(q,indent=2)+'\n').encode()),flush=True)

if __name__=='__main__':main()
