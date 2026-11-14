#!/usr/bin/env python3
"""Freeze wholly fresh B05 plans from the exact authored90-case declaration."""
import copy,hashlib,json,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from broader_player_worker_matrix import specs as historical_specs
from broader_viewport_source_budget import bound,predecessors
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'


def main():
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n))
  emit=lambda n,q:dict(record=n,sha256=write_report(ROOT,n,(json.dumps(q,indent=2)+'\n').encode()))
  test=subprocess.run(['python3','tests/test-broader-viewport-source-budget.py'],cwd=ROOT,capture_output=True);require(test.returncode==0,'Current budget refusal controls failed')
  with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
   db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns,case_id')]
  require(len(rows)==25 and sum(v['reserved_games']for v in rows)==49,'Stopped request set changed')
  b=r.json(P+'build-current-05.json');old=r.json(P+'build-current-04.json');changed=sorted(n for n,h in old['compiler_sources'].items()if b['compiler_sources'][n]!=h);rt=sorted(n for n,h in old['runtime_sources'].items()if b['runtime_sources'][n]!=h)
  require(b['success']and b['sources_stable']and changed==['apps/qt-shell/acceptance_capture.hpp']and not rt,'Unexpected B05 native delta')
  controls=[P+n+'.json'for n in ['native-controls-06','parallel-viewport-controls-01','parallel-command-controls-03','viewport-command-history-controls-01','viewport-legacy-history-controls-02','drain-admission-controls-01','input-reference-controls-01']]
  # The input reference control has a historical spelling; it remains exact.
  if not(ROOT/controls[-1]).exists():controls[-1]=P+'input-reference-controls-01.json'
  for n in controls:
   c=r.json(n);require(c['success']and c.get('sources_stable',True)and all(r.digest(k)==h for k,h in c.get('sources',{}).items()),'Current controls changed: '+n)
  reviewer=['tools/prepare-broader-viewport-phase.py','tools/broader_viewport_source_budget.py','tools/run-broader-viewport-source-case.py','tests/test-broader-viewport-source-budget.py']
  new_tools=sorted('tools/'+v.name for v in(ROOT/'tools').glob('*broader*viewport*case.py'))+['tools/check-broader-viewport-menu.py'];reviewer=sorted(set(reviewer+new_tools+['tools/broader_player_viewport_matrix.py']))
  q=dict(schema=1,kind='broader_viewport_source_continuation_review',success=True,original_game_runs=0,old_source_current_credit=False,old_reserved_starts=49,old_additional_ceiling=130,new_additional_ceiling=147,old_combined_ceiling=138,new_combined_ceiling=155,fresh_positive_cases=90,fresh_positive_original_starts=98,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,queue_limits_increased=False,game_response_deadlines_changed=False,reviewer_sources={n:r.digest(n)for n in reviewer},current_build=ref(P+'build-current-05.json'),changed_compiler_files=changed,changed_runtime_files=rt,current_controls=[ref(n)for n in controls],old_requests=rows,parent_global_plan=ref(P+'plan-15.json'),staging=ref(P+'recorder-player-staging-05.json'),offline_replay=ref(P+'viewport-prefix-parallel-replay-01.json'),old_negative_custody=ref(P+'bp13-boundary-default800-01-observation-custody-01.json'),controls_log=(test.stdout+test.stderr).decode(),reason='Parallelize only the opt-in campaign/movie diagnostic recorder after a retained-input replay reproduced queue overflow in the old recorder. Stateful delta preparation and writes remain FIFO, deep ownership and all caps/assertions/game deadlines remain exact. The old live failure error branch is unknown. Every49earlier reservation and all historical positive/negative source phases remain immutable. The full fresh90case/98start B05conjunction needs147additional/155combined reservations with no spare or automatic repair. First execute the actual NVIDIA fullscreen boundary; expand only after its independent comparison/admission/retention. This is an agent-set finite bound, not user numeric approval; no old Native file freshness waiver or Original recovery promotion.')
  review=emit(P+'viewport-source-review-01.json',q);g=copy.deepcopy(r.json(P+'plan-15.json'))
  for k in ['drain_continuation_review','recorder_source_review','previous_input_incomplete_plan','input_metadata_completion']:g.pop(k,None)
  g.update(kind='broader_current_player_viewport_source_plan',previous_plan=q['parent_global_plan'],viewport_source_review=review,current_build=q['current_build'],current_player_binary_sha256=b['player']['sha256'],maximum_additional_original_games=147,maximum_combined_original_games=155,maximum_combined_WP10_and_new_original_games=155,minimum_final_additional_reservations=147,retained_predecessor_reservations=49,remaining_positive_cases=90,remaining_positive_original_starts=98,request_driver='tools/run-broader-viewport-source-case.py',scope=q['reason'])
  require(bound(r,g)==147 and predecessors(r,rows,g)==49,'New bounded review refused');gr=emit(P+'plan-16.json',g)
  declared=historical_specs(r);require(len(declared)==90 and sum(s['case']['reserved_original_games']for s in declared)==98,'Authored cohort changed')
  old_names=sorted({s['plan']for s in declared})
  def renamed(n):return n.replace('bp13','bp16').replace('strict-input-plan-13.json','strict-input-plan-16.json').replace('boundary-input-plan-13.json','boundary-input-plan-16.json')
  names={n:renamed(n)for n in old_names};require(len(set(names.values()))==24,'Changed phase denominator')
  aliases={P+'plan-13.json':gr,P+'build-current-04.json':q['current_build'],P+'native-controls-05.json':ref(P+'native-controls-06.json'),P+'player-build-only-binding-04.json':ref(P+'player-build-only-binding-05.json')}
  for k in ['player','world-history','original-reference']:aliases[old[k]['record']]=b[k]
  def adapt(v):
   if isinstance(v,dict):
    if v.get('record')in aliases:return copy.deepcopy(aliases[v['record']])|{k:adapt(x)for k,x in v.items()if k not in ['record','sha256','bytes','physical_path']}
    return {adapt(k):adapt(x)for k,x in v.items()}
   if isinstance(v,list):return [adapt(x)for x in v]
   if isinstance(v,str):
    if v in names:return names[v]
    return v.replace('bp13','bp16').replace('qt-shell-04','qt-shell-05').replace(old['player']['sha256'],b['player']['sha256']).replace('tools/run-broader-worker-source-case.py','tools/run-broader-viewport-source-case.py')
   return v
  bindings=[];phase_refs=[];source_bytes={}
  for n in old_names:
   original=r.json(n);phase=adapt(original);phase.update(broader_plan=gr,request_driver='tools/run-broader-viewport-source-case.py',supersedes_prospective_plan=ref(n),current_controls=ref(P+'native-controls-06.json'),compiler_sources={k:r.digest(k)for k in original['compiler_sources']},runtime_sources={k:r.digest(k)for k in original['runtime_sources']},player_binary=dict(b['player'],path='working/build/qt-shell/mnm-qt-shell'))
   if 'current_player_build'in phase:phase['current_player_build']=q['current_build']
   if 'current_player_build_binding'in phase:phase['current_player_build_binding']=ref(P+'player-build-only-binding-05.json')
   if 'oracle'in phase:
    phase['oracle']['native_sources']={k:r.digest(k)for k in original['oracle']['native_sources']}
    phase['oracle']['binaries']=[dict(b[k],path='working/build/qt-shell/mnm-acceptance-world-history'if k=='world-history'else'working/original-reference')for k in ['world-history','original-reference']]
   # Discard old preimage routing only from the new declaration; old originals remain exact.
   phase['historical_source_preimages']=original.get('source_preimages');phase['source_preimages']=[]
   for field in ['oracle_sources','clip_oracle_sources']:
    if field in phase:phase[field]={k:r.digest(k)for k in phase[field]}
   names_to_pin=set(original['sources'])|set(reviewer)
   if original.get('fault_policy')=='drained-Mini-producer-invalid':
    names_to_pin.update(['tools/run-drained-transition-fault.py','tools/run-broader-drained-recovery.py']);phase['unchanged_native_fault_binary']=original['fault_binary'];raw=(ROOT/'tools/run-drained-transition-fault.py').read_bytes();fn=names[n].removesuffix('.json')+'-fault-adapter.bin';phase['fault_binary']=dict(path='tools/run-drained-transition-fault.py',record=fn,sha256=write_report(ROOT,fn,raw),bytes=len(raw))
    for c in phase['broader_cases']:c['command']=['tools/run-broader-drained-recovery.py'if x=='tools/run-broader-recovery.py'else x for x in c['command']]
   if n==P+'strict-input-plan-13.json':phase['actual_context_analysis_plan']=P+'strict-extent-analysis-plan-16.json'
   phase['sources']={k:r.digest(k)for k in sorted(names_to_pin)}
   for k in names_to_pin:source_bytes[k]=(ROOT/k).read_bytes()
   pr=emit(names[n],phase);phase_refs.append(pr)
   for s in declared:
    if s['plan']==n:
     c=next(v for v in phase['broader_cases']if v['id']==s['case']['id'].replace('bp13','bp16'));require(all(c[k]==s['case'][k]for k in ['profile','provider','size','reserved_original_games']),'Finite context changed');bindings.append(dict(previous_case_id=s['case']['id'],case_id=c['id'],plan=pr,role=s['role'],family=s['family']))
  analysis={}
  for stem in ['native-edge','world-native-edge','boundary-native-edge']:
   n=P+stem+'-analysis-plan-13.json';v=copy.deepcopy(r.json(n));require(all(r.digest(k)==h for k,h in v['sources'].items()),'Independent oracle source drift');v.update(global_plan=gr,previous_analysis_plan=ref(n));analysis[stem]=emit(P+stem+'-analysis-plan-16.json',v)
  n=P+'strict-extent-analysis-plan-14.json';v=copy.deepcopy(r.json(n));v.update(strict_phase=next(x for x in phase_refs if x['record']==P+'strict-input-plan-16.json'),previous_analysis_plan=ref(n));v['sources']={k:r.digest(k)for k in v['sources']};analysis['strict-extent']=emit(P+'strict-extent-analysis-plan-16.json',v)
  with configured(ROOT)as pair:
   with pair.batch():sr={n:dict(sha256=pair.bytes(raw),size=len(raw))for n,raw in sorted(source_bytes.items())}
   source_root=pair.root('broader_viewport_phase_current_source_preimages',ROOT,dict(schema=1,sources=sr))
  catalog=dict(schema=1,kind='broader_fresh_viewport_source_catalog',success=True,original_game_runs=0,global_plan=gr,review=review,current_build=q['current_build'],retained_current_cases=0,total_positive_cases=90,total_positive_original_starts=98,case_bindings=bindings,analysis_plans=analysis,prospective_phases=phase_refs,source_preimages=source_root,first_pilot='bp16-boundary-default800-01',old_source_current_credit=False,global_player_accepted=False)
  require(len(bindings)==90 and len({x['case_id']for x in bindings})==90,'Fresh cohort lost cases');cr=emit(P+'viewport-source-catalog-01.json',catalog);print(json.dumps(dict(success=True,catalog=cr,global_plan=gr,source_preimages=source_root)),flush=True)
if __name__=='__main__':main()
