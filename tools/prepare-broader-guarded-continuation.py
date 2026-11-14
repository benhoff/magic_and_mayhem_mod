#!/usr/bin/env python3
"""Plan only the exact guard-stopped replacement; preserve every prior cap/charge."""
import argparse,copy,json,sqlite3,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/';T='research/runtime/rendering-transition-recovery-results-20261107/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh guarded continuation review required')
 logs=[]
 for name in('tests/test-broader-guarded-continuation.py','tests/test-broader-repair-binding.py'):
  v=subprocess.run([sys.executable,name],cwd=ROOT,capture_output=True);require(v.returncode==0,'Guarded continuation controls failed');logs.append(dict(command=[sys.executable,name],returncode=0,log=(v.stdout+v.stderr).decode()))
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));failed='bp11-transition-default800-01';an=T+failed+'-acquisition.json';a=r.json(an);require(a['success']is False and a['original_game_runs']==2 and a['actual_start_count_complete']and a['case_accepted']is False,'Unbound guarded negative')
  final=ref(P+failed+'-guarded-negative-finalization-01.json');f=r.json(final['record']);require(f['success']and f['acquisition']==ref(an)and f['reservation_refunded']is False,'Negative/custody finalization missing')
  parent=P+'plan-09.json';g=copy.deepcopy(r.json(parent));require(g['maximum_additional_original_games']==112 and g['maximum_combined_original_games']==120,'Changed previous agent bound')
  q=dict(schema=1,kind='broader_guard_stopped_continuation_review',success=True,original_game_runs=0,case_accepted=False,parent_global_plan=ref(parent),failed_case_id=failed,failed_acquisition=ref(an),negative_finalization=final,old_additional_ceiling=112,new_additional_ceiling=113,old_combined_ceiling=120,new_combined_ceiling=121,ceiling_extension=1,replacement_original_starts=2,prior_remaining_unallocated_starts=1,minimum_final_additional_reservations=113,reserved_failed_starts_retained=2,observed_failed_original_starts=2,reservations_refunded=False,native_inputs_or_actions_changed=False,user_numeric_approval_claimed=False,agent_set_limit=True,reviewer_sources={n:r.digest(n)for n in('tools/broader_guarded_continuation.py','tools/run-broader-continued-case.py','tests/test-broader-guarded-continuation.py','tools/broader_repair_binding.py','tools/run-broader-recovery-repair.py','tests/test-broader-repair-binding.py','tools/prepare-broader-guarded-continuation.py')},controls=dict(tests=11,results=logs),reason='The100GiBCold guard stopped a two-start acquisition before its final proof was emitted. Both actual starts and all raw observations are retained; missing transient physical-action/launch data cannot be invented. An exact two-start replacement consumes the old single unallocated start plus one explicitly added agent-set start. Keep all old112/120plans/counters and every failure immutable;113/121is the smallest complete continuation with no unallocated repair starts.',storage_review='Increase the pair reservation from3.75to6GiB using the measured failed Cold/RAM/prefix footprint plus headroom. Launch only after the reviewed exact source/output retention restores at least106GiBCold and preserves main90/SATA10floors; original response/action deadlines and capture limits stay unchanged.')
  qh=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());review_ref=dict(record=args.report,sha256=qh)
  g.update(previous_plan=ref(parent),guard_stopped_review=review_ref,maximum_additional_original_games=113,maximum_combined_original_games=121,maximum_combined_WP10_and_new_original_games=121,minimum_final_additional_reservations=113,maximum_manually_reviewed_additional_repairs=0,retained_guard_stopped_current_starts=2,maximum_additional_ceiling_extension=1,request_driver='tools/run-broader-continued-case.py',budget_review=q['reason'])
  gn=P+'plan-10.json';gh=write_report(ROOT,gn,(json.dumps(g,indent=2)+'\n').encode())
  oldname=T+'bp11-input-plan-03.json';phase=copy.deepcopy(r.json(oldname));cases=[]
  for c in phase['broader_cases']:
   if c['id']=='bp11-transition-default640-01':continue
   c=copy.deepcopy(c)
   if c['id']==failed:
    key='bp11-transition-default800-02';new_report=T+key+'-acquisition.json';reason='Replace guard-stopped collection with complete proof after exact raw custody and increased storage headroom; no renderer, input action, deadline, fault or scope change.';c.update(id=key,report=new_report,repair_of=an,repair_reason=reason,command=['python3','tools/run-broader-recovery-repair.py','--kind','transition','--provider','nvidia','--plan',T+'bp11-input-plan-04.json','--report',new_report,'--repair-of',an,'--repair-reason',reason])
   else:
    cmd=c['command'];require(cmd.count('--plan')==1,'Unknown remaining collector interface');cmd[cmd.index('--plan')+1]=T+'bp11-input-plan-04.json'
   cases.append(c)
  require(len(cases)==3 and len({c['id']for c in cases})==3,'Changed remaining fault domains')
  phase.update(broader_plan=dict(record=gn,sha256=gh),request_driver=g['request_driver'],broader_cases=cases,supersedes_unused_or_failed_continuation=ref(oldname),guard_stopped_review=review_ref)
  phase['execution_paths']['reports']={c['report']:dict(software=c['profile']=='player-software',size=c['size'])for c in cases};phase['pilot']=cases[0]['report'];phase['sources'].update(q['reviewer_sources']);phase['pair_Cold_reservation_gib']=6
  # Existing local requests and the10original-start ceiling are unchanged.
  require(phase['bounds']['maximum_original_games']==10,'Do not reset the local two-generation budget')
  pn=T+'bp11-input-plan-04.json';ph=write_report(ROOT,pn,(json.dumps(phase,indent=2)+'\n').encode());print(json.dumps(dict(success=True,review_sha256=qh,global_plan=dict(record=gn,sha256=gh),phase=dict(record=pn,sha256=ph),replacement_original_starts=2,total_current_primary_domains_preserved=True,maximum_additional=113,maximum_combined=121)),flush=True)
if __name__=='__main__':raise SystemExit(main())
