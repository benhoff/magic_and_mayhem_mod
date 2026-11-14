#!/usr/bin/env python3
"""Freeze one manually reviewed replacement of a genuinely incomplete command capture."""
import argparse,copy,json,sqlite3,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
from broader_capture_continuation import bound,predecessors
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/';T='research/runtime/rendering-transition-recovery-results-20261107/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh explicit continuation required')
 run=subprocess.run([sys.executable,'tests/test-broader-capture-continuation.py'],capture_output=True,cwd=ROOT);require(run.returncode==0,'Capture continuation guards failed')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));case='bp11-transition-default800-02';ar=ref(T+case+'-acquisition.json');cr=ref(T+case+'-comparison-01.json');dr=ref(P+case+'-capture-refusal-analysis-01.json');hr=ref(P+case+'-capture-refused-custody-01.json');mr=ref(P+'storage-maintenance-checkpoint-02.json');m=r.json(mr['record']);require(m['success']and m['total_completed_source_copies']==58,'Complete all exact source handoffs first')
  old=P+'plan-10.json';g=copy.deepcopy(r.json(old));require(g['maximum_additional_original_games']==113 and g['maximum_combined_original_games']==121,'Prior cap changed')
  names=['tools/broader_capture_continuation.py','tools/run-broader-capture-continued-case.py','tools/prepare-broader-capture-continuation.py','tests/test-broader-capture-continuation.py'];sources={n:r.digest(n)for n in names}
  # All unused interfaces and source fingerprints are checked before emitting
  # any new review or plan, so a planning refusal cannot consume a game start.
  candidates=[]
  for group in ['world','menu','component']:
   candidates +=[v['record']for v in r.json(P+group+'-request-plans-bp11-03.json')['plans']]
  candidates +=[P+'strict-input-plan-03.json',P+'boundary-input-plan-02.json']
  with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:existing_now={v[0]for v in db.execute('select case_id from requests')}
  for old_phase in candidates:
   candidate=r.json(old_phase);require(candidate['success']and all(r.digest(n)==h for n,h in candidate['sources'].items()),'Unused phase changed: '+old_phase)
   for c in candidate['broader_cases']:
    cmd=c['command'];require(c['id']not in existing_now and not(ROOT/c['report']).exists()and cmd.count('--plan')==1 and cmd[cmd.index('--plan')+1]==old_phase,'Unknown or requested unused case: '+c['id'])
  tbase=r.json(T+'bp11-input-plan-04.json');require(all(r.digest(n)==h for n,h in (tbase['sources']|tbase['compiler_sources']|tbase['runtime_sources']).items()),'Unexecuted B02continuation is superseded when any player source changes')
  with sqlite3.connect(ROOT/tbase['execution_paths']['budget'])as db:require(db.execute('select count(*),sum(charged_games)from attempts').fetchone()==(3,6),'Local transition accounting changed')
  nr=dict(schema=1,kind='broader_player_negative_predecessor_review',success=True,case_accepted=False,original_game_runs=0,reserved_original_starts_retained=2,acquisition=ar,failed_comparison=cr,source_continuation_review=g['source_continuation_review'],reviewer_sources=sources,diagnosis=dr,negative_custody=hr,reservations_refunded=False,reason='Actual collection returned but the exact fresh command capture is incomplete after a bounded queue refusal;30,089,412command bytes exceed the retained1MiBring. Keep all raw bytes, original incomplete flags and both starts. No synthetic END/reply, waived capture/pixel assertion or acceptance follows.')
  nn=P+case+'-capture-negative-predecessor-review-01.json';nh=write_report(ROOT,nn,(json.dumps(nr,indent=2)+'\n').encode());negative_ref=dict(record=nn,sha256=nh)
  review=dict(schema=1,kind='broader_capture_refusal_continuation_review',success=True,original_game_runs=0,case_accepted=False,parent_global_plan=ref(old),failed_case_id=case,failed_acquisition=ar,failed_comparison=cr,capture_diagnosis=dr,negative_custody=hr,maintenance_checkpoint=mr,negative_predecessor_review=negative_ref,old_additional_ceiling=113,new_additional_ceiling=115,old_combined_ceiling=121,new_combined_ceiling=123,ceiling_extension=2,replacement_original_starts=2,reserved_failed_starts_retained=2,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,native_sources_actions_deadlines_or_capture_limits_changed=False,automatic_repairs_remaining=0,reviewer_sources=sources,controls=dict(tests=7,command=[sys.executable,'tests/test-broader-capture-continuation.py'],returncode=0,log=(run.stdout+run.stderr).decode()),reason='The incomplete fresh command history genuinely lacks bytes not recoverable from its stopped producer ring. A new exact physical-fault/Hard-Restart pair is required; mixing a later process with the old native-shell lifetime cannot establish the contract. Preserve every113/121parent cap and charge, and explicitly raise only the prospective agent-set bound by two to115/123. Run only after all owned source compaction/custody producers have stopped, using the exact same player, input actions, deadlines and256MiB/4096job capture policy. Concurrent maintenance is a scheduling difference, not a proved root cause or repair.')
  rh=write_report(ROOT,a.report,(json.dumps(review,indent=2)+'\n').encode());rr=dict(record=a.report,sha256=rh);g['preserved_negative_predecessors'][case]=negative_ref;g.update(previous_plan=ref(old),capture_refusal_review=rr,maximum_additional_original_games=115,maximum_combined_original_games=123,maximum_combined_WP10_and_new_original_games=123,minimum_final_additional_reservations=115,maximum_manually_reviewed_additional_repairs=0,request_driver='tools/run-broader-capture-continued-case.py')
  require(bound(r,g)==115,'Continuation bound check failed')
  with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests')]
  require(predecessors(r,rows,g)==28,'Prior request charges changed');existing={v['case_id']for v in rows};gn=P+'plan-11.json';gh=write_report(ROOT,gn,(json.dumps(g,indent=2)+'\n').encode());gr=dict(record=gn,sha256=gh)
  tp=copy.deepcopy(r.json(T+'bp11-input-plan-04.json'));cases=[];pn=T+'bp11-input-plan-05.json'
  for c in tp['broader_cases']:
   c=copy.deepcopy(c)
   if c['id']==case:
    key='bp11-transition-default800-03';report=T+key+'-acquisition.json';reason='Replace the exact incomplete command-capture pair after complete owned storage maintenance; preserve all prior charges, raw failures, native binary, actions, deadlines and capture limits.';c.update(id=key,report=report,repair_of=ar['record'],repair_reason=reason,command=['python3','tools/run-broader-recovery-repair.py','--kind','transition','--provider','nvidia','--plan',pn,'--report',report,'--repair-of',ar['record'],'--repair-reason',reason])
   else:c['command'][c['command'].index('--plan')+1]=pn
   require(c['id']not in existing and not(ROOT/c['report']).exists(),'Never repeat a requested case');cases.append(c)
  require(len(cases)==3 and tp['bounds']['maximum_original_games']==10,'Changed remaining two-generation domains');tp['bounds']['maximum_original_games']=12;tp['bounds']['maximum_requests']=max(tp['bounds']['maximum_requests'],6);tp.update(broader_plan=gr,request_driver=g['request_driver'],broader_cases=cases,supersedes_unused_or_failed_continuation=ref(T+'bp11-input-plan-04.json'),capture_refusal_review=rr);tp['sources'].update(sources);tp['execution_paths']['reports']={c['report']:dict(software=c['profile']=='player-software',size=c['size'])for c in cases};tp['pilot']=cases[0]['report'];write_report(ROOT,pn,(json.dumps(tp,indent=2)+'\n').encode())
  def phase(old,new):
   q=copy.deepcopy(r.json(old));require(q['success']and all(r.digest(n)==h for n,h in q['sources'].items()),'Unused source phase changed');require(all(c['id']not in existing and not(ROOT/c['report']).exists()for c in q['broader_cases']),'Do not rebind executed cases');require(not(ROOT/new).exists(),'Fresh unused phase required');q.update(broader_plan=gr,request_driver=g['request_driver'],supersedes_unused_plan=ref(old),original_native_inputs_actions_local_budgets_and_case_ids_unchanged=True);q['sources'].update(sources)
   for c in q['broader_cases']:
    cmd=c['command'];require(cmd.count('--plan')==1 and cmd[cmd.index('--plan')+1]==old,'Unknown collector plan binding');cmd[cmd.index('--plan')+1]=new
   sha=write_report(ROOT,new,(json.dumps(q,indent=2)+'\n').encode());return dict(record=new,sha256=sha,family=q.get('family','strict'),cases=len(q['broader_cases']))
  for group in ['world','menu','component']:
   old=P+group+'-request-plans-bp11-03.json';bundle=copy.deepcopy(r.json(old));plans=[]
   for v in bundle['plans']:
    require(r.digest(v['record'])==v['sha256'],'Bundle member changed');name=v['record'];new=name.replace('input-plan-03.json','input-plan-04.json');require(new!=name,'Unknown unused phase version');plans.append(phase(name,new))
   bundle.update(source_preflight=ref(old),global_plan=gr,plans=plans,minimum_final_additional_reservations=115,maximum_additional_reservations=115);write_report(ROOT,P+group+'-request-plans-bp11-04.json',(json.dumps(bundle,indent=2)+'\n').encode())
  phase(P+'strict-input-plan-03.json',P+'strict-input-plan-04.json');phase(P+'boundary-input-plan-02.json',P+'boundary-input-plan-03.json')
  for old,new in [('native-edge-analysis-plan-03.json','native-edge-analysis-plan-04.json'),('world-native-edge-analysis-plan-02.json','world-native-edge-analysis-plan-03.json'),('boundary-native-edge-analysis-plan-02.json','boundary-native-edge-analysis-plan-03.json')]:
   q=copy.deepcopy(r.json(P+old));require(all(r.digest(n)==h for n,h in q['sources'].items()),'Analysis sources changed');q.update(global_plan=gr,previous_analysis_plan=ref(P+old));write_report(ROOT,P+new,(json.dumps(q,indent=2)+'\n').encode())
  old=P+'strict-extent-analysis-plan-02.json';q=copy.deepcopy(r.json(old));require(all(r.digest(n)==h for n,h in q['sources'].items()),'Extent sources changed');q.update(strict_phase=ref(P+'strict-input-plan-04.json'),previous_analysis_plan=ref(old));write_report(ROOT,P+'strict-extent-analysis-plan-03.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,review_sha256=rh,global_plan=dict(record=gn,sha256=gh),maximum_additional=115,maximum_combined=123,local_transition_maximum_original_starts=12,original_starts_added=0)),flush=True)
if __name__=='__main__':main()
