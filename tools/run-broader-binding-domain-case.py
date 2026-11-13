#!/usr/bin/env python3
"""Execute one bounded current case through comparison, admission and retention."""
import argparse,hashlib,json,sqlite3,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];MAIN=Path('/home/hoff/swdev/magic_and_mayhem_mod')
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--case',required=True);p.add_argument('--analysis-plan');args=p.parse_args()
 out=ROOT/'working/broader-player/domain-coordination'/args.case;out.mkdir(parents=True);(out/'.retain').touch()
 with Reader(ROOT)as r:
  phase=r.json(args.plan);case=next(v for v in phase['broader_cases']if v['id']==args.case);require(phase['success']and phase['original_game_runs']==0 and case['reserved_original_games']==1,'Unbounded or failed domain plan')
  driver=phase.get('request_driver','tools/run-broader-player-case.py');require(driver in('tools/run-broader-binding-source-case.py',),'Unknown request driver')
  plan_ref=dict(record=args.plan,sha256=r.digest(args.plan))
 with sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3')as db:require(db.execute('select 1 from requests where case_id=?',(args.case,)).fetchone()is None,'Never repeat a requested domain automatically')
 family=phase['family'];group='menu'if family in('menu','intros','skip')else'component'if family in('clipping','stretch')else'world'
 q=dict(schema=1,kind='broader_domain_case_coordinator',success=False,case=args.case,family=family,group=group,plan=plan_ref,original_game_runs_authorized=1,automatic_retries=False,steps=[],source=dict(path='tools/run-broader-binding-domain-case.py',sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()))
 if group=='world':
  require(args.analysis_plan is not None,'Declared World analysis model required')
  with Reader(ROOT)as r:
   analysis=r.json(args.analysis_plan);require(analysis['success']and analysis['global_plan']==phase['broader_plan']and analysis['ignored_pixels']==0,'Unbound World analysis plan');q['analysis_plan']=dict(record=args.analysis_plan,sha256=r.digest(args.analysis_plan))
 def execute(label,command,*,cold=.5,main=1,allow_pixel_negative=False):
  guard=['python3',str(ROOT/'tools/working-retention.py'),'--root',str(ROOT),'--policy',str(ROOT/'working/whole-player/cold-run-policy.json'),'run','--reserve-gib',str(cold),'--','python3',str(MAIN/'tools/working-retention.py'),'--root',str(MAIN),'run','--reserve-gib',str(main),'--','env','-C',str(ROOT),*command]
  log=out/(label+'.log');start=time.time_ns()
  with log.open('xb')as stream:result=subprocess.run(guard,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
  q['steps'].append(dict(stage=label,command=guard,returncode=result.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(log)))
  print(json.dumps(dict(case=args.case,stage=label,returncode=result.returncode)),flush=True);require(result.returncode==0 or allow_pixel_negative,'Domain failed/refused: '+label)
 try:
  import os
  maintenance=Path('/home/hoff/swdev/magic_and_mayhem_mod/working/retention/broader-player-capacity-20261110/compact-continuations-06.log')
  finished=json.loads(maintenance.read_text().splitlines()[-1]);require(not Path('/proc/189091').exists()and finished.get('success')is True and finished.get('completed')==32,'Complete all owned source maintenance before any new acquisition')
  execute('acquire',['python3',driver,'acquire','--plan',args.plan,'--case',args.case],cold=6,main=2)
  acquisition=case['report'];validation=P+args.case+'-validation-01.json'
  if group!='menu':
   execute('compare',case['comparison_command'],cold=1,main=1,allow_pixel_negative=group=='world')
   comparison=case['comparison'];checker='tools/check-broader-'+group+'.py'
   if group=='world':
    with Reader(ROOT)as r:baseline=r.json(comparison)
    if not baseline['success']:
     require(baseline['sources_stable']and baseline['comparison_processes_stopped']and baseline['original_manifest_verified_before_after']and baseline.get('comparison',{}).get('mismatching_pixels',0)>0,'Unexpected incomplete World baseline failure')
     comparison=case['comparison'].removesuffix('-comparison-01.json')+'-comparison-policy-01.json'
     execute('native-policy',['python3','tools/compare-broader-world-policy.py','--plan',args.plan,'--case',args.case,'--raw-comparison',case['comparison'],'--oracle-build',analysis['oracle_build']['record'],'--controls',analysis['controls']['record'],'--report',comparison],cold=1,main=1)
     checker='tools/check-broader-policy-world.py'
   execute('validate',['python3',checker,'--acquisition',acquisition,'--comparison',comparison,'--report',validation],cold=.1,main=.2)
  else:execute('validate',['python3','tools/check-broader-viewport-menu.py','--acquisition',acquisition,'--report',validation],cold=.1,main=.5)
  execute('admit',['python3',driver,'admit','--plan',args.plan,'--case',args.case,'--validation',validation],cold=.1,main=.2)
  execute('retain-execution',['python3','tools/retire-broader-domain-case.py','--validation',validation,'--report',P+args.case+'-handoff-01.json'],cold=.05,main=1)
  execute('retain-RAM-capture',['python3','tools/retire-broader-capture-copy.py','--validation',validation,'--report',P+args.case+'-RAM-handoff-01.json'],cold=.05,main=.5)
  with Reader(ROOT)as r:q.update(validation=dict(record=validation,sha256=r.digest(validation)),acquisition=dict(record=acquisition,sha256=r.digest(acquisition)))
  q['success']=True
 except Exception as e:q['error']=repr(e)
 finally:
  for step in q['steps']:
   data=Path(step['log_path']).read_bytes();name=P+args.case+'-coordinator-'+step['stage']+'.bin';step['log']=dict(record=name,sha256=write_report(ROOT,name,data),bytes=len(data))
  h=write_report(ROOT,P+args.case+'-coordinator-01.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],case=args.case,error=q.get('error'),sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
