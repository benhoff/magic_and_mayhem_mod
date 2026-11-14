#!/usr/bin/env python3
"""Complete only retained pilot admission/custody after exact identity review."""
import hashlib,json,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];MAIN=Path('/home/hoff/swdev/magic_and_mayhem_mod');P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 cid='bp20-world-attachment-default640-01';review_name=P+'relative-preferences-admission-review-01.json';rn=P+cid+'-coordinator-02.json';require(not(ROOT/rn).exists(),'Never overwrite a coordinator result');out=ROOT/'working/broader-player/domain-coordination'/cid/'retained-resume-02';out.mkdir();(out/'.retain').touch()
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));review=r.json(review_name);require(review['success']and review['original_game_runs']==review['new_request_reservations']==0 and review['all_other_world_assertions_byte_identical']and not review['pixel_or_lifetime_assertions_waived']and all(r.digest(n)==h for n,h in review['sources'].items()),'Unreviewed retained-data continuation');a=r.json(review['acquisition']['record']);plan=a['broader_phase'];v=r.json(review['new_validation']['record']);require(v['success']and v['acquisition']==review['acquisition']and v['comparison']==review['comparison'],'Different or refused retained validation');validation=review['new_validation']['record'];sources=dict(review['sources']);q=dict(schema=1,kind='broader_domain_case_coordinator',success=False,case=cid,family='attachment',group='world',plan=plan,original_game_runs_authorized=0,new_original_game_runs=0,new_request_reservations=0,new_native_executions=0,new_pixel_comparisons=0,automatic_retries=False,steps=[],source=dict(path='tools/resume-broader-relative-admission.py',sha256=r.digest('tools/resume-broader-relative-admission.py')),sources=sources,analysis_only_retained_resume=ref(review_name),old_failed_coordinator=review['old_failed_coordinator'])
 def execute(label,command):
  guard=['python3',str(ROOT/'tools/working-retention.py'),'--root',str(ROOT),'--policy',str(ROOT/'working/whole-player/cold-run-policy.json'),'run','--reserve-gib','.1','--','python3',str(MAIN/'tools/working-retention.py'),'--root',str(MAIN),'run','--reserve-gib','.5','--','env','-C',str(ROOT),*command];log=out/(label+'.log');start=time.time_ns()
  with log.open('xb')as f:run=subprocess.run(guard,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
  q['steps'].append(dict(stage=label,command=guard,returncode=run.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(log)));print(json.dumps(dict(case=cid,stage=label,returncode=run.returncode)),flush=True);require(run.returncode==0,'Retained admission/custody refused: '+label)
 try:
  execute('admit',['python3','tools/run-broader-namespace-source-case.py','admit','--plan',plan['record'],'--case',cid,'--validation',validation]);execute('retain-execution',['python3','tools/retire-broader-domain-case.py','--validation',validation,'--report',P+cid+'-handoff-01.json']);execute('retain-RAM-capture',['python3','tools/retire-broader-capture-copy.py','--validation',validation,'--report',P+cid+'-RAM-handoff-01.json'])
  with Reader(ROOT)as r:require(all(r.digest(n)==h for n,h in sources.items()),'Retained resumption source drift');q.update(success=True,sources_stable=True,validation=review['new_validation'],acquisition=review['acquisition'])
 except Exception as e:q['error']=repr(e)
 finally:
  for step in q['steps']:
   n=P+cid+'-coordinator-02-'+step['stage']+'.bin';step['log']=dict(record=n,sha256=write_report(ROOT,n,Path(step['log_path']).read_bytes()))
  print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=write_report(ROOT,rn,(json.dumps(q,indent=2)+'\n').encode()))),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
