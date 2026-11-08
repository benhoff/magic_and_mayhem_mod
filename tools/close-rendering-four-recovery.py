#!/usr/bin/env python3
"""Admit only four transition/failure rows from independently bound raw domains."""
import argparse,hashlib,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from rendering_acceptance import requirements,validate
from rendering_transition_assessment import observed_epochs,assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/rendering-recovery-four-results-20261108/'
OLD='research/runtime/rendering-transition-recovery-results-20261107/'
BID='NP.four-recovery-operation-closeout';SID='native.four-recovery-operation-closeout'
LEAVES={'operation.transition-recovery','operation.failure-cleanup'}
HIGH=['default800-07-comparison-04.json','software800-01-comparison-02.json']
LOW=['four09-default640-01-comparison-01.json','four09-software640-01-comparison-01.json']
HISTORICAL_DRIFT={'runtime/menu/campaign_mini_observe.h','runtime/menu/drawing_frame_input.h','runtime/menu/observer.c','runtime/render/input_polling.h','tools/menu-game-runner.py'}
ANALYSIS=['tools/close-rendering-four-recovery.py','tests/test-rendering-four-recovery.py','tools/rendering_transition_assessment.py','tools/rendering_transition.py','tools/rendering_pause_assessment.py','tools/rendering_pause.py','tools/rendering_interaction.py','tools/rendering_menu_acceptance.py','tools/rendering_pilot.py','tools/rendering_capture.py','tools/campaign_storage.py','tools/pause_low_ingress.py','tools/join-rendering-normal-acceptance.py','tools/coverage_claims.py','tools/coverage_evidence.py','tools/rendering_normal_acceptance.py','tools/cpu_reader_journal.py']

ANALYSIS += ['tools/rendering_acceptance.py','docs/native-rendering-closure.json','tools/pause_focus.py','tools/native_cursor_journal.py','protocols/include/mnm/input_v1.h','protocols/include/mnm/render_commands_v3.h','protocols/include/mnm/render_control_v1.h']

def checked(reader,ref):
 require(reader.digest(ref['record'])==ref['sha256'],'Retained reference changed')
 return reader.json(ref['record'])

def read_case(reader,name,historical):
 c=reader.json(name)
 require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped'] and c['original_manifest_verified_before_after'],'Incomplete independent comparison')
 a=checked(reader,c['acquisition'])
 require(a['success'] and a['sources_stable'] and a['private_wine_stopped'] and a['original_game_runs']==2,'Incomplete two-generation acquisition')
 drift={n:dict(recorded=h,current=reader.digest(n)) for n,h in c['sources'].items() if reader.digest(n)!=h}
 require(set(drift)==HISTORICAL_DRIFT if historical else not drift,'Unexpected execution source phase drift')
 require(not c['original_pixels_used_as_native_inputs'] and not a['original_pixels_used_as_native_inputs'],'Dependent native inputs')
 _,epochs,partition=observed_epochs(reader,a);proof=assess(reader,a,c,epochs)
 require(proof==c['proof'] and partition==c['partition'],'Current raw assessment differs from retained result')
 require(proof['domain']['size']==([800,600] if historical else [640,480]),'Wrong source domain')
 require(c['comparison']['success'] and c['comparison']['pixels_compared']>0 and c['comparison']['mismatching_pixels']==0,'No independent presented output')
 for epoch in c['epochs']:
  ref=epoch['original_reference']
  require(not ref['native_player_pixels_read'] and not ref['original_destination_capture_read'] and epoch['comparison']['success'] and epoch['comparison']['mismatching_pixels']==0,'Incomplete independent epoch output')
 # The current claim is retained-data admission, never current whole-native execution.
 return dict(identity=a['session_identity'],domain=proof['domain'],proof=proof,comparison=dict(record=name,sha256=reader.digest(name)),acquisition=c['acquisition'],pixels=c['comparison']['pixels_compared'],provider=a['resident']['renderer'],historical_execution=historical,execution_sources=c['sources'],raw_source_drift=drift,original_game_runs=a['original_game_runs'])

def validate_cases(cases):
 require(len(cases)==4 and len({c['identity'] for c in cases})==4,'Four distinct actual sessions required')
 require({(c['domain']['software'],tuple(c['domain']['size'])) for c in cases}=={(v,s) for v in (False,True) for s in ((640,480),(800,600))},'Missing required profile/size domain')
 for c in cases:
  p=c['proof']
  require(p['verified'] and p['cleanup']['verified'] and p['physical_public_restart'] and p['fresh_positive_input'] and p['old_rendering_accepted'] is False and p['failed_epoch_normal_completion_claimed'] is False,'Failed generation or incomplete cleanup cannot earn credit')
  require(p['terminal_samples']>=8 and p['terminal_stability_ns']>=600_000_000 and len(set(p['distinct_world_nonces']))==2 and len({(r['pid'],r['start_ticks']) for r in p['distinct_runner_identities']})==2 and c['pixels']>0,'Missing stable failure/new-generation authority')

def close_rows(plan,parent,cases):
 validate(plan);validate_cases(cases)
 require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan)) and len(parent['requirements'])==156,'Acceptance dossier changed')
 require({(r['configuration'],r['id']) for r in parent['requirements']}=={(r['configuration'],r['id']) for r in requirements(plan)} and sum(r['state']=='passed' for r in parent['requirements'])==parent['passed_requirements'],'Parent row identity/count mismatch')
 rows=[];count=0
 for old in parent['requirements']:
  row=dict(old)
  if row['id'] in LEAVES:
   require(row['state']=='pending','Requested row already admitted')
   count+=1;row.update(state='passed',admitted_execution_evidence=[dict(acquisition=c['acquisition'],comparison=c['comparison'],actual_world_extents=[c['domain']['size']],provider=c['provider'],reason='Actual declared INVALID on drained Mini producer; exact failure wire and all native retirement predicates, stable no-World interval, physical public restart with distinct original runner/channel/World identities, fresh positive input and ordinary Campaign/Main Quit. All native World swaps have independent per-epoch output comparison. Failed old generation receives no normal-completion credit; historical source phases stay pinned.') for c in cases if c['domain']['software']==(row['configuration']=='player-software')])
  rows.append(row)
 require(count==4 and sum(v['state']=='passed' for v in rows)==parent['passed_requirements']+4,'Closeout exceeds four rows')
 require(all(n==o for n,o in zip(rows,parent['requirements']) if o['id'] not in LEAVES),'Unrelated acceptance changed')
 return rows

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--high-only',action='store_true');p.add_argument('--parent',default='research/runtime/rendering-defeat-results-20261108/acceptance-01.json');p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(P) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh immutable report required')
 q=dict(schema=1,kind='four_recovery_operation_admission',success=False,newly_passed_requirements=0,new_original_reference_execution=False,new_high_pixel_comparison=False)
 try:
  with Reader(ROOT) as r:
   sources={n:r.digest(n) for n in ANALYSIS};q['analysis_sources']=sources
   cases=[read_case(r,OLD+n,True) for n in HIGH]
   q.update(cases=cases,verified_domains=2,historical_pixel_values=sum(c['pixels'] for c in cases),historical_high_execution_current=False)
   if not a.high_only:
    cases += [read_case(r,OLD+n,False) for n in LOW];validate_cases(cases)
    parent_name=a.parent;parent=r.json(parent_name)
    rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
    reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID)
    require(required_sources(b)==set(ANALYSIS),'Analysis source closure changed')
    with sqlite3.connect('file:'+str(ROOT/'working/transition-four09-attempts.sqlite3')+'?mode=ro',uri=True) as db:names=[v[0] for v in db.execute('SELECT report FROM attempts')]
    require(set(names)=={OLD+n.replace('-comparison-01.json','-acquisition.json') for n in LOW},'Unexpected new collection requests')
    starts=sum(r.json(n)['original_game_runs'] for n in names);require(starts==4,'New original-start bound changed')
    q.update(sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},parent_source_phases=parent['sources'],cases=cases,requirements=rows,verified_domains=4,required_domains=4,newly_passed_requirements=4,passed_requirements=parent['passed_requirements']+4,pending_requirements=parent['pending_requirements']-4,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],parent=dict(record=parent_name,sha256=r.digest(parent_name)),claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={SID:scenario_contract(s)})],original_game_runs=4,historical_verified_original_game_runs=4,historical_failures_retired=False,checked_pixels=sum(c['pixels'] for c in cases),mismatching_pixels=0,current_whole_native_execution=False)
   q.update(success=all(r.digest(n)==h for n,h in sources.items()),sources_stable=all(r.digest(n)==h for n,h in sources.items()),scope='Raw retained-data admission of declared terminal failure, bounded retirement, physical fresh-generation restart and normal fresh Quit. High1019-source phase stays historical; Low uses exact current conservative catalog. No current whole-player equivalence, physical context reset, pending-CPU interruption, in-place/save continuation, arbitrary faults, font/DC/ABI/provider or final rendering decision claim.')
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),verified_domains=q.get('verified_domains'),newly_passed=q['newly_passed_requirements'],sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
