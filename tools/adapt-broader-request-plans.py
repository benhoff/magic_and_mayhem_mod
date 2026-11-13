#!/usr/bin/env python3
"""Rebind unused prospective cases to exact reviewed prelaunch accounting."""
import argparse,copy,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('preflight','global-plan','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh request adaptation review required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));old=r.json(args.preflight);global_plan=r.json(args.global_plan)
  require(old['success']and old['original_game_runs']==0 and global_plan['success']and global_plan['request_driver']=='tools/run-broader-reviewed-case.py','Accepted unused preflight and reviewed request driver required')
  with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:existing={v[0]for v in db.execute('select case_id from requests')}
  plans=[]
  for item in old['plans']:
   name=item['record'];require(r.digest(name)==item['sha256'],'Changed original prospective plan');q=copy.deepcopy(r.json(name))
   require(q['success']and q['original_game_runs']==0 and all(r.digest(n)==h for n,h in q['sources'].items()),'Original prospective source changed')
   require(all(v['id']not in existing and not(ROOT/v['report']).exists()for v in q['broader_cases']),'Never rebind an executed or requested case')
   updated=name.replace('input-plan-01.json','input-plan-02.json');require(updated!=name and not(ROOT/updated).exists(),'Fresh plan interface required')
   q.update(broader_plan=ref(args.global_plan),request_driver=global_plan['request_driver'],supersedes_unused_plan=ref(name),original_native_inputs_actions_local_budgets_and_case_ids_unchanged=True)
   q['sources'].update({n:r.digest(n)for n in['tools/run-broader-reviewed-case.py','tools/broader_prelaunch_review.py']})
   for case in q['broader_cases']:
    command=case['command'];require(command.count('--plan')==1 and command[command.index('--plan')+1]==name,'Unexpected collector plan interface');command[command.index('--plan')+1]=updated
   plans.append(dict(record=updated,sha256=write_report(ROOT,updated,(json.dumps(q,indent=2)+'\n').encode()),family=q['family'],cases=len(q['broader_cases'])))
  q=dict(schema=1,kind='unused_broader_request_plan_adaptation',success=True,original_game_runs=0,source_preflight=ref(args.preflight),global_plan=ref(args.global_plan),plans=plans,native_inputs_or_actions_changed=False,prior_counters_reset=False,minimum_final_additional_reservations=106,maximum_additional_reservations=112,source=dict(path='tools/adapt-broader-request-plans.py',sha256=r.digest('tools/adapt-broader-request-plans.py')))
  h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,plans=len(plans),cases=sum(v['cases']for v in plans))),flush=True)
if __name__=='__main__':main()
