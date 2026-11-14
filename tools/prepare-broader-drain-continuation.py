#!/usr/bin/env python3
"""Freeze one exact drain replacement and the still unstarted current B04 matrix."""
import copy,hashlib,json,sqlite3
from contextlib import closing
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_worker_matrix import specs,current,P,BINARY
from broader_worker_source_budget import bound as old_bound
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];FAILED='bp13-transition-default800-01';REPLACEMENT='bp13-transition-default800-02';CAT=P+'drain-continuation-catalog-01.json';GLOBAL=P+'plan-14.json'

def main():
 require(not(ROOT/CAT).exists()and not(ROOT/GLOBAL).exists(),'Never overwrite a continuation')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));declared=specs(r);parent=r.json(P+'plan-13.json');require(old_bound(r,parent)==128,'Parent source/cap changed')
  with closing(sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3'))as db:
   db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
  require(len(rows)==21 and sum(v['reserved_games']for v in rows)==42,'Review exact stopped request set');by={v['case_id']:v for v in rows};positives=[s for s in declared if s['case']['id']in by and by[s['case']['id']]['state']=='admitted'];require(len(positives)==5,'Preserve exactly five complete current cases');retained=[current(ROOT,r,s,by[s['case']['id']])for s in positives];remaining=[s for s in declared if s not in positives];require(len(remaining)==85 and sum(v['case']['reserved_original_games']for v in remaining)==88,'Changed remaining positive denominator')
  negative=P+FAILED+'-drain-refusal-custody-01.json';custody=r.json(negative);require(custody['success']and custody['case_accepted']is False and custody['request_row_preserved_exactly']==by[FAILED]and by[FAILED]['state']=='failed'and by[FAILED]['reserved_games']==2,'Retained failed row changed')
  controls=[P+'drain-admission-controls-01.json',P+'isolated-tooling-controls-08.json']
  for n in controls:
   v=r.json(n);require(v['success']and v['sources_stable']and all(r.digest(k)==h for k,h in v['sources'].items()),'Current controls stale')
  reviewer_names=['tools/prepare-broader-drain-continuation.py','tools/broader_drained_source_budget.py','tools/run-broader-drained-source-case.py','tools/run-broader-drained-recovery.py','tools/run-drained-transition-fault.py','tools/broader_player_worker_matrix.py','tests/test-broader-drained-budget.py','tests/test-broader-drained-fault.py']
  q=dict(schema=1,kind='broader_drained_admission_continuation_review',success=True,original_game_runs=0,parent_global_plan=ref(P+'plan-13.json'),negative_custody=ref(negative),old_additional_ceiling=128,new_additional_ceiling=130,old_combined_ceiling=136,new_combined_ceiling=138,retained_predecessor_reservations=42,predecessor_requests=rows,retained_current_positive_cases=[dict(case=v['spec']['case']['id'],validation=v['validation'],acquisition=v['acquisition'])for v in retained],remaining_positive_cases=85,remaining_positive_original_starts=88,remaining_case_ids=[REPLACEMENT if s['case']['id']==FAILED else s['case']['id']for s in remaining],fresh_positive_cases=90,fresh_positive_original_starts=98,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,native_player_changed=False,response_deadlines_changed=False,queue_or_capture_limits_increased=False,reviewer_sources={n:r.digest(n)for n in reviewer_names},controls=[ref(n)for n in controls],reason='The old helper refused its own non-drained check after a separate collector drain observation and before fault mutation. Keep that negative and both reservations. The new adapter retries only that exact pre-mutation refusal inside the existing5second helper call, with a2second/128attempt inner bound. All identity/state/uncancelled/equal-cursor and nonmutation assertions, physical actions,600second case deadline, pixels and complete-history requirements remain. Five fully compared/admitted/retained current B04 pairs keep their exact unchanged Native and collector source hashes. No old native-file hash is waived or old row rewritten. One exact2start replacement and88remaining positive starts use the smallest130additional/138combined conservative ceiling; no spare repair starts remain.')
  review_name=P+'drain-continuation-review-01.json';review_hash=write_report(ROOT,review_name,(json.dumps(q,indent=2)+'\n').encode());review_ref=dict(record=review_name,sha256=review_hash)
  g=dict(schema=1,kind='broader_current_player_drain_continuation_plan',success=True,original_game_runs=0,previous_WP10_original_games=8,maximum_additional_original_games=130,maximum_combined_original_games=138,maximum_combined_WP10_and_new_original_games=138,minimum_final_additional_reservations=130,maximum_manually_reviewed_additional_repairs=0,previous_plan=ref(P+'plan-13.json'),drain_continuation_review=review_ref,recorder_source_review=parent['recorder_source_review'],current_build=parent['current_build'],current_player_binary_sha256=BINARY,requirements=parent['requirements'],bounds=parent['bounds'],retained_predecessor_reservations=42,remaining_positive_original_starts=88,remaining_positive_cases=85,fresh_source_positive_cases=90,fresh_source_original_starts=98,denominator=parent['denominator'],providers=parent['providers'],request_driver='tools/run-broader-drained-source-case.py',supported_player_accepted=False,original_recovery_complete=False,scope=q['reason'])
  gh=write_report(ROOT,GLOBAL,(json.dumps(g,indent=2)+'\n').encode());gr=dict(record=GLOBAL,sha256=gh)
  # Clone only prospective declarations; every prior payload stays immutable.
  phase_map={};bindings=[];phase_refs=[]
  for old_name in sorted({v['plan']for v in remaining}):
   old=r.json(old_name);phase=copy.deepcopy(old);new_name=old_name.removesuffix('.json')+'-drained-01.json';phase_map[old_name]=new_name;phase['broader_plan']=gr;phase['request_driver']='tools/run-broader-drained-source-case.py';phase['supersedes_unrequested_plan']=ref(old_name);phase['native_player_and_authored_contracts_unchanged']=True
   phase['sources'].update({n:r.digest(n)for n in ['tools/broader_drained_source_budget.py','tools/run-broader-drained-source-case.py']})
   for case in phase['broader_cases']:
    for field in ['command','comparison_command']:
     if field in case:case[field]=[new_name if x==old_name else x for x in case[field]]
    if case['id']==FAILED:
     case['id']=REPLACEMENT;case['report']=case['report'].replace(FAILED,REPLACEMENT);case['command']=[x.replace(FAILED,REPLACEMENT)for x in case['command']];case['repair_of']=ref(next(s['case']['report']for s in remaining if s['case']['id']==FAILED));case['repair_reason']='Exact retained helper drain refusal before mutation; use bounded unchanged-injector admission and preserve both old reservations.'
   if old.get('fault_policy')=='drained-Mini-producer-invalid':
    phase['sources'].update({n:r.digest(n)for n in ['tools/run-drained-transition-fault.py','tools/run-broader-drained-recovery.py']});raw=(ROOT/'tools/run-drained-transition-fault.py').read_bytes();fn=new_name.removesuffix('.json')+'-fault-adapter.bin';phase['fault_binary']=dict(path='tools/run-drained-transition-fault.py',record=fn,sha256=write_report(ROOT,fn,raw),bytes=len(raw));phase['unchanged_native_fault_binary']=old['fault_binary'];phase['bounds'].update(maximum_requests=5,maximum_original_games=10,maximum_repair_original_starts=2)
    for case in phase['broader_cases']:case['command']=['tools/run-broader-drained-recovery.py'if x=='tools/run-broader-recovery.py'else x for x in case['command']]
   if old_name==P+'strict-input-plan-13.json':phase['actual_context_analysis_plan']=P+'strict-extent-analysis-plan-14.json'
   phase['source_preimages']=list(phase.get('source_preimages',[]))
   for n in sorted(set(phase['sources'])-set(old['sources'])):
    raw=(ROOT/n).read_bytes();sn=new_name.removesuffix('.json')+'-source-'+hashlib.sha256(raw).hexdigest()+'.bin';phase['source_preimages'].append(dict(path=n,record=sn,sha256=write_report(ROOT,sn,raw)))
   h=write_report(ROOT,new_name,(json.dumps(phase,indent=2)+'\n').encode());nr=dict(record=new_name,sha256=h);phase_refs.append(nr)
   for s in remaining:
    if s['plan']==old_name:bindings.append(dict(previous_case_id=s['case']['id'],case_id=REPLACEMENT if s['case']['id']==FAILED else s['case']['id'],plan=nr,role=s['role'],family=s['family']))
  analysis_refs={}
  for stem in ['native-edge','world-native-edge','boundary-native-edge']:
   old_name=P+stem+'-analysis-plan-13.json';new_name=P+stem+'-analysis-plan-14.json';v=copy.deepcopy(r.json(old_name));v['global_plan']=gr;v['previous_analysis_plan']=ref(old_name);h=write_report(ROOT,new_name,(json.dumps(v,indent=2)+'\n').encode());analysis_refs[stem]=dict(record=new_name,sha256=h)
  n=P+'strict-extent-analysis-plan-13.json';v=copy.deepcopy(r.json(n));v['strict_phase']=dict(record=phase_map[P+'strict-input-plan-13.json'],sha256=next(x['sha256']for x in phase_refs if x['record']==phase_map[P+'strict-input-plan-13.json']));v['previous_analysis_plan']=ref(n);v['sources']={n:r.digest(n)for n in v['sources']};n=P+'strict-extent-analysis-plan-14.json';h=write_report(ROOT,n,(json.dumps(v,indent=2)+'\n').encode());analysis_refs['strict-extent']=dict(record=n,sha256=h)
  for group in ['world','menu','component']:
   old_name=P+group+'-request-plans-bp13-01.json';v=copy.deepcopy(r.json(old_name));v['plans']=[next(x for x in phase_refs if x['record']==phase_map[p['record']])for p in v['plans']];v.update(previous_bundle=ref(old_name),global_additional_primary_starts=124,maximum_additional_original_games=130)
   if 'total_additional_primary_original_games'in v:v['total_additional_primary_original_games']=124
   name=P+group+'-request-plans-bp13-drained-01.json';write_report(ROOT,name,(json.dumps(v,indent=2)+'\n').encode())
  catalog=dict(schema=1,kind='broader_drained_admission_prospective_catalog',success=True,original_game_runs=0,global_plan=gr,review=review_ref,retained_current_cases=5,remaining_positive_cases=85,remaining_positive_original_starts=88,total_positive_cases=90,total_positive_original_starts=98,case_bindings=bindings,analysis_plans=analysis_refs,prospective_phases=phase_refs,global_player_accepted=False,scope='Only five completed exact current B04 cases are retained. All85other positive cells have fresh immutable prospective plans. The drain-refused default800context has one named replacement and explicit collector ancestry; its failed row/cap remains. All authored domains,14journeys,94cells,42operation cells and original suppliers stay unchanged.')
  require(len(bindings)==85 and len({v['previous_case_id']for v in bindings})==85,'Lost continuation binding');ch=write_report(ROOT,CAT,(json.dumps(catalog,indent=2)+'\n').encode());print(json.dumps(dict(success=True,review_sha256=review_hash,global_plan_sha256=gh,catalog_sha256=ch,retained_current_cases=5,remaining_cases=85,additional_ceiling=130)),flush=True)
if __name__=='__main__':main()
