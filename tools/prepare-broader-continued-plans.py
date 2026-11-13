#!/usr/bin/env python3
"""Rebind only unused requests to the exact guard-stop continuation review."""
import argparse,copy,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_guarded_continuation import bound,predecessors
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh continuation output required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));gn=P+'plan-10.json';g=r.json(gn);require(bound(r,g)==113,'Exact reviewed continuation required')
  with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:
   db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select * from requests')]
  charge=predecessors(r,rows,g);existing={v['case_id']for v in rows};outputs=[]
  extra=['tools/broader_guarded_continuation.py','tools/run-broader-continued-case.py']
  def phase(old,new):
   require(not(ROOT/new).exists(),'Never overwrite a source phase');q=copy.deepcopy(r.json(old))
   require(q['success']and q['original_game_runs']==0 and all(r.digest(n)==h for n,h in q['sources'].items()),'Prospective source changed: '+old)
   require(all(v['id']not in existing and not(ROOT/v['report']).exists()for v in q['broader_cases']),'Never rebind requested/executed cases')
   q.update(broader_plan=ref(gn),request_driver=g['request_driver'],supersedes_unused_plan=ref(old),original_native_inputs_actions_local_budgets_and_case_ids_unchanged=True)
   q['sources'].update({n:r.digest(n)for n in extra})
   for c in q['broader_cases']:
    cmd=c['command'];require(cmd.count('--plan')==1 and cmd[cmd.index('--plan')+1]==old,'Unknown collector plan binding');cmd[cmd.index('--plan')+1]=new
   h=write_report(ROOT,new,(json.dumps(q,indent=2)+'\n').encode());return dict(record=new,sha256=h,family=q.get('family','strict'),cases=len(q['broader_cases']))
  candidates=[]
  for group in('world','menu','component'):
   candidates.extend(v['record']for v in r.json(P+group+'-request-plans-bp11-02.json')['plans'])
  candidates +=[P+'strict-input-plan-02.json',P+'boundary-input-plan-01.json']
  for name in candidates:
   q=r.json(name);require(q['success']and q['original_game_runs']==0 and all(r.digest(n)==h for n,h in q['sources'].items()),'Prospective phase source changed: '+name)
   for c in q['broader_cases']:
    require(c['id']not in existing and not(ROOT/c['report']).exists(),'Already requested case')
    cmd=c['command'];require(cmd.count('--plan')==1 and cmd[cmd.index('--plan')+1]==name,'Unknown collector plan binding: '+name)
  for group in('world','menu','component'):
   old=P+group+'-request-plans-bp11-02.json';bundle=copy.deepcopy(r.json(old));plans=[]
   for item in bundle['plans']:
    require(r.digest(item['record'])==item['sha256'],'Changed bundle member');name=item['record'];new=name.replace('input-plan-02.json','input-plan-03.json');require(new!=name,'Unknown plan version');plans.append(phase(name,new))
   bundle.update(source_preflight=ref(old),global_plan=ref(gn),plans=plans,minimum_final_additional_reservations=113,maximum_additional_reservations=113)
   name=P+group+'-request-plans-bp11-03.json';require(not(ROOT/name).exists(),'Fresh bundle required');write_report(ROOT,name,(json.dumps(bundle,indent=2)+'\n').encode());outputs.append(ref(name))
  outputs.append(phase(P+'strict-input-plan-02.json',P+'strict-input-plan-03.json'))
  outputs.append(phase(P+'boundary-input-plan-01.json',P+'boundary-input-plan-02.json'))
  for old,new in [('native-edge-analysis-plan-02.json','native-edge-analysis-plan-03.json'),('world-native-edge-analysis-plan-01.json','world-native-edge-analysis-plan-02.json'),('boundary-native-edge-analysis-plan-01.json','boundary-native-edge-analysis-plan-02.json')]:
   q=copy.deepcopy(r.json(P+old));require(q['success']and all(r.digest(n)==h for n,h in q['sources'].items()),'Analysis source changed');q.update(global_plan=ref(gn),previous_analysis_plan=ref(P+old));write_report(ROOT,P+new,(json.dumps(q,indent=2)+'\n').encode());outputs.append(ref(P+new))
  old=P+'strict-extent-analysis-plan-01.json';q=copy.deepcopy(r.json(old));require(q['success']and all(r.digest(n)==h for n,h in q['sources'].items()),'Strict extent source changed')
  # Preserve the frozen checker/coordinator source vector; only its prospective phase changes.
  for k,v in list(q.items()):
   if isinstance(v,dict)and v.get('record')==P+'strict-input-plan-02.json':q[k]=ref(P+'strict-input-plan-03.json')
  q['previous_analysis_plan']=ref(old);write_report(ROOT,P+'strict-extent-analysis-plan-02.json',(json.dumps(q,indent=2)+'\n').encode());outputs.append(ref(P+'strict-extent-analysis-plan-02.json'))
  q=dict(schema=1,kind='unused_broader_guarded_continuation_plans',success=True,original_game_runs=0,global_plan=ref(gn),reserved_starts_unchanged=charge,outputs=outputs,native_inputs_or_actions_changed=False,source=dict(path=str(Path(__file__).relative_to(ROOT)),sha256=r.digest(Path(__file__).relative_to(ROOT))))
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,outputs=len(outputs),reserved=charge)),flush=True)
if __name__=='__main__':main()
