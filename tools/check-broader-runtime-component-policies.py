#!/usr/bin/env python3
"""Bind fresh B07 runtime recorder/reader and real-GL controls to three narrow policies."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args();name=args.report;require(name.startswith(P)and not(ROOT/name).exists(),'Fresh immutable component conjunction required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));sources={};parents=[]
  for stem in ['parallel-command-controls-03','parallel-viewport-controls-01','viewport-command-history-controls-01','viewport-legacy-history-controls-02','viewport-prefix-parallel-replay-01','native-controls-08']:
   n=P+stem+'.json';q=r.json(n);require(q['success']and q['sources_stable']and not q.get('original_game_runs',0)and not q.get('original_game_executed',False),'Unaccepted or non-isolated control');require(all(r.digest(k)==h for k,h in q['sources'].items()),'Stale component execution')
   for k,h in q['sources'].items():require(k not in sources or sources[k]==h,'Mixed native source phases');sources[k]=h
   parents.append(ref(n))
   if stem=='viewport-prefix-parallel-replay-01':require(q['result']['retained_prefix_replayed_completely']and q['result']['copied_rows']==1977 and q['result']['every_written_payload_matches_input']and not q['result']['old_capture_completed']and not q['result']['synthetic_END_added'],'Incomplete retained-prefix diagnostic')
  gui=next(c for c in q['commands']if c['label']=='gui-target')['result'];require(gui['actual_target_destructions']==1 and gui['primary_gui_provider']and gui['independent_pixel_checks']==2 and gui['fresh_channel_identity']and gui['deferred_unpresented_and_borrowed_requests']==2,'Incomplete GUI policy controls')
  for n in ['tools/check-broader-component-policies.py','tools/check-broader-viewport-component-policies.py','tools/check-broader-copy-component-policies.py','tools/check-broader-runtime-component-policies.py']:sources[n]=r.digest(n)
  reg=r.json('research/runtime/coverage/register.json');claims=[]
  for bid in ['NP.command-capture-parallel-compression','NP.viewport-capture-parallel-compression','NP.gui-surface-loss-quiescence']:
   b=next(v for v in reg['behaviors']if v['id']==bid);s=next(v for v in reg['scenarios']if v['behaviors']==[bid]and v['level']=='headless');require(required_sources(b)|set(s['tests'])<=set(sources),'Incomplete policy compiler/test source closure: '+bid);claims.append(dict(behavior=bid,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)}))
  q=dict(schema=1,kind='broader_current_native_viewport_component_conjunction',success=True,sources_stable=True,original_game_runs=0,new_native_executions=0,parents=parents,gui=gui,parallel_optimized_and_sanitized_verified=True,unchanged_long_2300paint_control_verified=True,legacy_command_and_viewport_readers_verified=True,first_refusal_and_idle_close_verified=True,incomplete_prefix_replay_rows=1977,old_live_failure_cause_proved=False,old_incomplete_capture_completed=False,game_or_pixel_acceptance_granted=False,sources=sources,claims=claims,scope='Exact retained executed B07 runtime diagnostic controls at matching full source hashes. Only ordered parallel recording, first refusal/drain behavior and private GUI-loss admission are certified. No new execution, game-output equality, complete player acceptance or universal/historical root-cause claim.')
 h=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h)),flush=True)
if __name__=='__main__':main()
