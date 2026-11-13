#!/usr/bin/env python3
"""Run one exact reviewed guarded continuation with increased storage headroom."""
import argparse,hashlib,json,os,sqlite3,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
MAIN=Path('/home/hoff/swdev/magic_and_mayhem_mod')
P='research/runtime/broader-player-acceptance-results-20261110/'
def raw_baseline_complete(baseline,acquired):
 require(baseline.get('sources_stable')and baseline.get('comparison_processes_stopped')and baseline.get('original_manifest_verified_before_after')and len(baseline.get('epochs',[]))==2,'Incomplete raw-original comparison')
 if baseline['success']:return
 require(baseline['error']=='Uncompared source/output epoch' and sum(v['comparison']['mismatching_pixels']for v in baseline['epochs'])>0,'Unexpected raw-original failure')
 for index,(item,epoch)in enumerate(zip(baseline['epochs'],acquired['proof']['epochs'])):
  require(item['original_reference']['worlds']==int(epoch['resident']['completed'])and item['original_reference']['source_chunks']==int(epoch['resident']['source_chunks']),'Raw-original source count incomplete')
  compared=item['comparison']
  if index==0:
   require(compared['normal_completion_claimed']is False and compared['synthetic_end_added']is False and compared['required_prefix_commands']>0 and compared['required_prefix_bytes']>0 and compared['uncredited_terminal_tail_bytes']>=0,'Wrong declared-failure prefix boundary')
  else:require(compared['remaining_surfaces']==0,'Fresh original comparison did not retire')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('plan','case','analysis-plan'):p.add_argument('--'+n,required=True)
 p.add_argument('--attempt',default='01');p.add_argument('--resume-review')
 args=p.parse_args();require(args.attempt.isdecimal(),'Safe coordinator attempt identity')
 out=ROOT/'working/broader-player/fault-coordination'/args.case/('attempt-'+args.attempt);out.mkdir(parents=True);(out/'.retain').touch()
 with Reader(ROOT)as r:
  phase=r.json(args.plan);analysis=r.json(args.analysis_plan);case=next(v for v in phase['broader_cases']if v['id']==args.case)
  require(analysis['success']and analysis['global_plan']==phase['broader_plan']and analysis['all_viewport_pixels_required']and analysis['ignored_pixels']==0 and analysis['comparison_only']and analysis['additional_original_starts']==0,'Unbound native-policy analysis plan')
  require(all(r.digest(n)==h for n,h in analysis['sources'].items()),'Prospective policy sources changed')
  plan_refs={n:dict(record=v,sha256=r.digest(v))for n,v in [('acquisition_plan',args.plan),('analysis_plan',args.analysis_plan)]}
  driver=phase.get('request_driver','tools/run-broader-player-case.py')
  require(driver in('tools/run-broader-discovery-source-case.py',),'Unknown request accounting driver')
  if args.attempt!='01':
   require(args.resume_review is not None,'Manual refusal review required')
   review=r.json(args.resume_review);require(review['success']and review['case']==args.case and review['original_starts_reserved']==0 and review['original_game_runs']==0,'Only a reviewed pre-reservation refusal may resume')
   prior=review['failed_coordinator'];require(r.digest(prior['record'])==prior['sha256']and not r.json(prior['record'])['success'],'Unbound prior coordinator refusal')
   plan_refs['manual_reservation_review']=dict(record=args.resume_review,sha256=r.digest(args.resume_review))
 with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:require(db.execute('select 1 from requests where case_id=?',(args.case,)).fetchone()is None,'Never automatically repeat a requested case')
 q=dict(schema=1,kind='broader_guarded_continuation_fault_coordinator',success=False,case=args.case,original_game_runs_authorized=2,automatic_retries=False,plans=plan_refs,steps=[],sources={str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
 def execute(label,cmd,*,cold=.5,main=1,allow_pixel_negative=False):
  guard=['python3',str(ROOT/'tools/working-retention.py'),'--root',str(ROOT),'--policy',str(ROOT/'working/whole-player/cold-run-policy.json'),'run','--reserve-gib',str(cold),'--','python3',str(MAIN/'tools/working-retention.py'),'--root',str(MAIN),'run','--reserve-gib',str(main),'--','env','-C',str(ROOT),*cmd]
  logfile=out/(label+'.log');start=time.time_ns()
  with logfile.open('xb')as log:run=subprocess.run(guard,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
  q['steps'].append(dict(stage=label,command=guard,returncode=run.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(logfile)))
  print(json.dumps(dict(case=args.case,stage=label,returncode=run.returncode)),flush=True)
  require(run.returncode==0 or allow_pixel_negative,'Case step failed/refused: '+label)
  return run.returncode
 try:
  import os
  maintenance=Path('/home/hoff/swdev/magic_and_mayhem_mod/working/retention/broader-player-capacity-20261110/compact-continuations-06.log')
  finished=json.loads(maintenance.read_text().splitlines()[-1]);require(not Path('/proc/189091').exists()and finished.get('success')is True and finished.get('completed')==32,'Complete all owned source maintenance before any new acquisition')
  execute('acquire',['python3',driver,'acquire','--plan',args.plan,'--case',args.case],cold=6,main=2)
  acquisition=case['report'];stem=acquisition.removesuffix('-acquisition.json');raw=stem+'-comparison-01.json';compared=stem+'-policy-comparison-01.json';kind='loss'if phase['fault_policy']=='quiescent-native-offscreen-target-loss'else'transition'
  execute('raw-original',['python3','tools/check-broader-recovery-ram.py','--kind',kind,'--acquisition',acquisition,'--report',raw],allow_pixel_negative=True)
  with Reader(ROOT)as r:
   baseline=r.json(raw);acquired=r.json(acquisition)
   raw_baseline_complete(baseline,acquired)
  execute('native-policy',['python3','tools/check-broader-policy-recovery.py','--kind',kind,'--acquisition',acquisition,'--raw-comparison',raw,'--oracle-build',analysis['oracle_build']['record'],'--controls',analysis['controls']['record'],'--report',compared])
  validation=P+args.case+'-validation-01.json';checker='tools/check-broader-copy-hardware-case.py'if case['provider']=='nvidia'else'tools/check-broader-copy-player-case.py'
  execute('validate',['python3',checker,'--acquisition',acquisition,'--comparison',compared,'--report',validation],cold=.1,main=.2)
  execute('expose-policy',['python3','tools/expose-broader-policy-comparison.py','--source',compared,'--alias',stem+'-comparison-policy-01.json','--report',P+args.case+'-legacy-policy-binding-01.json'],cold=.05,main=.2)
  execute('admit',['python3',driver,'admit','--plan',args.plan,'--case',args.case,'--validation',validation],cold=.1,main=.2)
  execute('retain-execution',['python3','working/broader-player/handoff-current-fault-policy-01.py','--validation',validation],cold=.05,main=1)
  execute('retain-held-top-files',['python3','tools/retire-broader-held-top-files.py','retire','--handoff',P+args.case+'-handoff-01.json','--report',P+args.case+'-top-files-handoff-01.json'],cold=.05,main=.5)
  execute('retain-RAM-capture',['python3','tools/retire-broader-capture-copy.py','--validation',validation,'--report',P+args.case+'-RAM-handoff-01.json'],cold=.05,main=.5)
  with Reader(ROOT)as r:
   q.update(acquisition=dict(record=acquisition,sha256=r.digest(acquisition)),validation=dict(record=validation,sha256=r.digest(validation)))
  q['success']=True
 except Exception as e:q['error']=repr(e)
 finally:
  for step in q['steps']:
   file=Path(step['log_path']);raw=file.read_bytes();name=P+args.case+'-coordinator-'+args.attempt+'-'+step['stage']+'.bin';step['log']=dict(record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw))
  q['retention_operator']=dict(path='working/broader-player/handoff-current-fault-policy-01.py',sha256=hashlib.sha256((ROOT/'working/broader-player/handoff-current-fault-policy-01.py').read_bytes()).hexdigest())
  h=write_report(ROOT,P+args.case+'-coordinator-'+args.attempt+'.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],case=args.case,error=q.get('error'),sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
