#!/usr/bin/env python3
"""Complete one reviewed caller context with current analysis input bindings."""
import argparse,hashlib,json,sqlite3,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_sources import python_sources
from broader_player_runtime_caller_matrix import P,FIRST,REVIEW
from broader_runtime_caller_commands import analysis_command
from broader_caller_loader_budget import phase_allowed
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
MAIN=Path('/home/hoff/swdev/magic_and_mayhem_mod')

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--plan',required=True);parser.add_argument('--case',required=True);parser.add_argument('--analysis-plan');parser.add_argument('--retained-only',action='store_true');args=parser.parse_args()
 with Reader(ROOT)as reader:
  ref=lambda n:dict(record=n,sha256=reader.digest(n));require(reader.digest(REVIEW['record'])==REVIEW['sha256'],'Caller analysis review changed');review=reader.json(REVIEW['record']);require(review['success']and all(reader.digest(n)==h for n,h in review['sources'].items()),'Caller analysis controls changed');phase=reader.json(args.plan);case=next(v for v in phase['broader_cases']if v['id']==args.case);phase_ref=ref(args.plan);require(phase_allowed(reader,phase,args.plan)and phase['family']=='caller'and case['reserved_original_games']==1,'Unreviewed caller acquisition context');sources={n:reader.digest(n)for n in python_sources(ROOT,['tools/run-broader-loader-caller-case.py'])}
  if args.retained_only:
   require(args.case==FIRST,'Retained-only completion belongs to the reviewed first caller');execution_ref=ref(P+'runtime29-caller-retained-analysis-execution-02.json');execution=reader.json(execution_ref['record']);require(execution['success']and execution['plan']==REVIEW and execution['original_game_runs']==0 and execution['new_public_player_executions']==0 and execution['request_rows_unchanged'],'Incomplete retained-only comparison');comparison=execution['comparison']['record'];require(reader.json(comparison)['success'],'Failed retained caller comparison')
  else:comparison=case['comparison'];execution_ref=None
 report=P+args.case+('-coordinator-02.json'if args.retained_only else'-coordinator-01.json');require(not(ROOT/report).exists(),'Never replace a caller coordinator')
 with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;row=db.execute('select *from requests where case_id=?',(args.case,)).fetchone()
  if args.retained_only:require(row is not None and dict(row)==review['request_row'],'Retained caller request changed')
  else:require(row is None,'Never automatically repeat a requested caller game')
 out=ROOT/'working/broader-player/caller-coordination'/args.case;out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
 q=dict(schema=1,kind='broader_domain_case_coordinator',success=False,case=args.case,family='caller',group='world',plan=phase_ref,original_game_runs_authorized=0 if args.retained_only else 1,original_game_runs_added=0 if args.retained_only else None,new_acquisition_performed=not args.retained_only,automatic_retries=False,steps=[],sources=sources,source=dict(path='tools/run-broader-loader-caller-case.py',sha256=sources['tools/run-broader-loader-caller-case.py']),retained_analysis_plan=REVIEW)
 if args.retained_only:q.update(previous_failed_coordinator=review['failed_coordinator'],retained_analysis_execution=execution_ref)
 def execute(stage,command,*,cold=.1,main=.3,allow_negative=False):
  guarded=['python3',str(ROOT/'tools/working-retention.py'),'--root',str(ROOT),'--policy',str(ROOT/'working/whole-player/cold-run-policy.json'),'run','--reserve-gib',str(cold),'--','python3',str(MAIN/'tools/working-retention.py'),'--root',str(MAIN),'run','--reserve-gib',str(main),'--','env','-C',str(ROOT),*command];log=out/(stage+'.log');start=time.time_ns()
  with log.open('xb')as stream:run=subprocess.run(guarded,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
  q['steps'].append(dict(stage=stage,command=guarded,returncode=run.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(log)));print(json.dumps(dict(case=args.case,stage=stage,returncode=run.returncode)),flush=True);require(run.returncode==0 or allow_negative,'Caller case failed/refused: '+stage)
 try:
  if not args.retained_only:
   execute('acquire',['python3','tools/run-broader-loader-source-case.py','acquire','--plan',args.plan,'--case',args.case],cold=6,main=2)
   with Reader(ROOT)as reader:q['original_game_runs_added']=reader.json(case['report'])['original_game_runs']
   execute('compare',analysis_command(case,review,comparison),cold=1,main=1,allow_negative=True)
  with Reader(ROOT)as reader:raw=reader.json(comparison)
  checker='tools/check-broader-relative-world.py'
  if not raw['success']:
   require(raw.get('sources_stable')and raw.get('comparison_processes_stopped')and raw.get('original_manifest_verified_before_after')and raw.get('comparison',{}).get('mismatching_pixels',0)>0,'Incomplete caller analysis failure');require(args.analysis_plan is not None,'Current Native edge analysis model required')
   with Reader(ROOT)as reader:analysis=reader.json(args.analysis_plan);require(analysis['success']and analysis['global_plan']==phase['broader_plan']and analysis['ignored_pixels']==0,'Unbound caller Native policy')
   policy=case['comparison'].removesuffix('-comparison-01.json')+'-comparison-policy-01.json';execute('native-policy',['python3','tools/compare-broader-runtime-caller-policy.py','--plan',args.plan,'--case',args.case,'--raw-comparison',comparison,'--oracle-build',analysis['oracle_build']['record'],'--controls',analysis['controls']['record'],'--report',policy],cold=1,main=1);comparison=policy;checker='tools/check-broader-relative-policy-world.py'
  validation=P+args.case+'-validation-01.json';execute('validate',['python3',checker,'--acquisition',case['report'],'--comparison',comparison,'--report',validation]);execute('admit',['python3','tools/run-broader-loader-source-case.py','admit','--plan',args.plan,'--case',args.case,'--validation',validation]);execute('retain-execution',['python3','tools/retire-broader-domain-case.py','--validation',validation,'--report',P+args.case+'-handoff-01.json'],cold=.05,main=1);execute('retain-RAM-capture',['python3','tools/retain-broader-controller-ram.py','--validation',validation,'--report',P+args.case+'-RAM-handoff-01.json'],cold=.05,main=.5)
  with Reader(ROOT)as reader:q.update(validation=dict(record=validation,sha256=reader.digest(validation)),acquisition=dict(record=case['report'],sha256=reader.digest(case['report'])))
  require(all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items()),'Caller coordinator source changed');q['success']=True
 except Exception as error:q['error']=repr(error)
 finally:
  for step in q['steps']:
   data=Path(step['log_path']).read_bytes();name=P+args.case+'-caller-coordinator-'+step['stage']+'.bin';step['log']=dict(record=name,sha256=write_report(ROOT,name,data),bytes=len(data))
  digest=write_report(ROOT,report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],case=args.case,error=q.get('error'),sha256=digest)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
