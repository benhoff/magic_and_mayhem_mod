#!/usr/bin/env python3
"""Retain two verified recovery domains without closing either four-domain row."""
import json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import required_sources,behavior_contract,scenario_contract
from rendering_transition_assessment import assess,observed_epochs
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/rendering-transition-recovery-results-20261107/'

def main():
 with Reader(ROOT) as r:
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']=='NP.transition-recovery-acceptance');s=next(v for v in reg['scenarios'] if v['id']=='native.transition-recovery-acceptance')
  sources={n:r.digest(n) for n in required_sources(b)};cases=[]
  for name in ('default800-07-comparison-04.json','software800-01-comparison-02.json'):
   c=r.json(P+name);require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped'] and c['original_manifest_verified_before_after'],'Incomplete retained comparison')
   require(all(r.digest(n)==h for n,h in c['sources'].items()),'Changed executed source phase')
   a=r.json(c['acquisition']['record']);require(r.digest(c['acquisition']['record'])==c['acquisition']['sha256'],'Changed acquisition')
   _,epochs,_=observed_epochs(r,a);proof=assess(r,a,c,epochs);require(proof==c['proof'] and proof['domain']['size']==[800,600],'Changed raw recovery proof')
   cases.append(dict(comparison=dict(record=P+name,sha256=r.digest(P+name)),acquisition=c['acquisition'],proof=proof,pixels=c['comparison']['pixels_compared'],mismatching_pixels=c['comparison']['mismatching_pixels']))
   sources.update(c['sources'])
  require({v['proof']['domain']['software'] for v in cases}=={False,True},'Missing800profile')
  with sqlite3.connect(ROOT/'working/transition-attempts.sqlite3') as db:names=[v[0] for v in db.execute('SELECT report FROM attempts ORDER BY report')]
  attempts=[]
  for name in names:
   a=r.json(name);attempts.append(dict(record=name,sha256=r.digest(name),success=a['success'],original_game_runs=a['original_game_runs'],error=a.get('error'),repair_of=a.get('repair_of')))
  require(len(attempts)==11 and sum(v['original_game_runs'] for v in attempts)==9,'Unexpected stopped budget')
  require(all(r.digest(n)==h for n,h in sources.items()),'Partial binding source drift')
  q=dict(schema=1,kind='partial_public_campaign_transition_recovery',success=True,sources=sources,sources_stable=True,
   claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})],
   cases=cases,verified_domains=2,required_domains=4,newly_passed_requirements=0,acceptance_closed=False,
   checked_pixels=sum(v['pixels'] for v in cases),mismatching_pixels=sum(v['mismatching_pixels'] for v in cases),attempts=attempts,
   original_game_runs=9,verified_case_original_game_runs=4,failed_control_original_game_runs=5,new_analysis_original_game_runs=0,
   unused_final_requests=1,collection_stopped=True,historical_failures_retired=False,
   pending_domains=[dict(software=v,size=[640,480]) for v in (False,True)],
   scope='Two actual800profiles only: real drained-Mini INVALID, terminal native owner/input/frame/timer retirement, stable interval, physical public Hard Restart, distinct runner/channel/World identities, fresh positive gameplay input and ordinary Quit; every actual native World swap independently compared. Four-domain acceptance remains pending. Default640-01 reached recovery but stopped before Quit confirmation input; subsequent640-02/04 timed out on accessibility during starting with no World output, and640-03 crashed Xvfb before a game. Startup cause remains unresolved; no proven raster defect, no engine repair or all-clock/provider/context-loss/pending-CPU/final renderer claim.')
  print(write_report(ROOT,P+'transition-partial-01.json',(json.dumps(q,indent=2)+'\n').encode()),flush=True)

if __name__=='__main__':main()
