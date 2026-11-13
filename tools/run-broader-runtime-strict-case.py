#!/usr/bin/env python3
"""Reserve, validate and retain one strict current campaign; never retry."""
import argparse,hashlib,json,os,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];MAIN=Path('/home/hoff/swdev/magic_and_mayhem_mod');P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--case',required=True);a=p.parse_args();out=ROOT/'working/broader-player/strict-coordination'/a.case;out.mkdir(parents=True);(out/'.retain').touch()
 with Reader(ROOT)as r:
  phase=r.json(a.plan);case=next(v for v in phase['broader_cases']if v['id']==a.case);require(phase['success']and case['reserved_original_games']==1,'One strict original start required');ref=dict(record=a.plan,sha256=r.digest(a.plan))
  driver=phase.get('request_driver','tools/run-broader-viewport-source-case.py');require(driver in('tools/run-broader-runtime-source-case.py',),'Unknown strict request driver');analysis_name=phase.get('actual_context_analysis_plan',P+'strict-extent-analysis-plan-16.json')
 q=dict(schema=1,kind='broader_strict_case_coordinator',success=False,case=a.case,plan=ref,original_game_runs_authorized=1,automatic_retries=False,steps=[],source=dict(path='tools/run-broader-runtime-strict-case.py',sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()))
 def execute(label,cmd,cold=.1,main=.5):
  guard=['python3',str(ROOT/'tools/working-retention.py'),'--root',str(ROOT),'--policy',str(ROOT/'working/whole-player/cold-run-policy.json'),'run','--reserve-gib',str(cold),'--','python3',str(MAIN/'tools/working-retention.py'),'--root',str(MAIN),'run','--reserve-gib',str(main),'--','env','-C',str(ROOT),*cmd];log=out/(label+'.log');start=time.time_ns()
  with log.open('xb')as f:v=subprocess.run(guard,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
  q['steps'].append(dict(stage=label,command=guard,returncode=v.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(log)));print(json.dumps(dict(case=a.case,stage=label,returncode=v.returncode)),flush=True);require(v.returncode==0,'Strict failed/refused: '+label)
 try:
  import os
  maintenance=Path('/home/hoff/swdev/magic_and_mayhem_mod/working/retention/broader-player-capacity-20261110/compact-continuations-06.log')
  finished=json.loads(maintenance.read_text().splitlines()[-1]);require(not Path('/proc/189091').exists()and finished.get('success')is True and finished.get('completed')==32,'Complete all owned source maintenance before any new acquisition')
  runtime=out/'runtime';runtime.mkdir(mode=0o700)
  execute('acquire',['env','XDG_RUNTIME_DIR='+str(runtime),'ATSPI_DBUS_IMPLEMENTATION=dbus-daemon','dbus-run-session','--','python3',driver,'acquire','--plan',a.plan,'--case',a.case],cold=6,main=2)
  validation=P+a.case+'-validation-01.json';execute('validate',['python3','tools/check-broader-strict-extent.py','--analysis-plan',analysis_name,'--acquisition',case['report'],'--report',validation]);execute('admit',['python3',driver,'admit','--plan',a.plan,'--case',a.case,'--validation',validation]);execute('retain',['python3','tools/retire-broader-strict.py','--validation',validation,'--report',P+a.case+'-handoff-01.json'],main=1)
  with Reader(ROOT)as r:q['validation']=dict(record=validation,sha256=r.digest(validation))
  q['success']=True
 except Exception as e:q['error']=repr(e)
 finally:
  for row in q['steps']:
   data=Path(row['log_path']).read_bytes();name=P+a.case+'-coordinator-'+row['stage']+'.bin';row['log']=dict(record=name,sha256=write_report(ROOT,name,data))
  h=write_report(ROOT,P+a.case+'-coordinator-01.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'))),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
