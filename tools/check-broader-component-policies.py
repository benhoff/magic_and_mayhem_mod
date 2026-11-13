#!/usr/bin/env python3
"""Bind retained current native controls to narrow recorder and diagnostic policies."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--native-controls',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh component-policy analysis required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));names=[P+'parallel-command-controls-02.json',P+'parallel-worker-controls-03.json',P+'parallel-history-controls-03.json',a.native_controls];sources={};parents=[]
  for name in names:
   q=r.json(name);require(q['success']and q['sources_stable']and not q.get('original_game_runs',0)and q.get('original_game_executed',False)is False,'Unaccepted or non-isolated component controls')
   require(all(r.digest(n)==h for n,h in q['sources'].items()),'Component execution sources changed')
   for n,h in q['sources'].items():require(n not in sources or sources[n]==h,'Mixed component source phases');sources[n]=h
   parents.append(ref(name))
  gui=next(v for v in q['commands']if v['label']=='gui-target')['result'];require(gui['actual_target_destructions']==1 and gui['primary_gui_provider']and gui['independent_pixel_checks']==2 and gui['fresh_channel_identity']and gui['deferred_unpresented_and_borrowed_requests']==2,'Incomplete GUI quiescence/destruction proof')
  sources['tools/check-broader-component-policies.py']=r.digest('tools/check-broader-component-policies.py');reg=r.json('research/runtime/coverage/register.json');claims=[]
  for bid in ['NP.command-capture-parallel-compression','NP.gui-surface-loss-quiescence']:
   b=next(v for v in reg['behaviors']if v['id']==bid);sc=next(v for v in reg['scenarios']if v['behaviors']==[bid]and v['level']=='headless');require(required_sources(b)|set(sc['tests'])<=set(sources),'Incomplete policy source closure');claims.append(dict(behavior=bid,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={sc['id']:scenario_contract(sc)}))
  q=dict(schema=1,kind='broader_current_native_component_control_conjunction',success=True,sources_stable=True,original_game_runs=0,new_native_executions=0,parents=parents,gui=gui,parallel_optimized_and_sanitized_verified=True,legacy_and_long_history_controls_preserved=True,game_or_pixel_acceptance_granted=False,sources=sources,claims=claims,scope='Reanalyze exact retained executed native controls at matching source fingerprints. Bind only bounded parallel FIFO recording and deferred private GUI-loss admission. No new native execution, original game, complete game capture, output acceptance or historical root-cause claim.')
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,original_game_runs=0)),flush=True)
if __name__=='__main__':main()
