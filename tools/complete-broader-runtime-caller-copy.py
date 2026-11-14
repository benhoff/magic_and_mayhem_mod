#!/usr/bin/env python3
"""Validate the retained first caller case and copy/restore evidence; remove nothing."""
import argparse,hashlib,json,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import seal,scan,safe,idle,MARKER
from broader_player_sources import python_sources
from broader_player_runtime_caller_matrix import P,FIRST,REVIEW
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.parse_args();report=P+FIRST+'-coordinator-02.json';require(not(ROOT/report).exists(),'Never replace a caller coordinator');step_results=[]
 with Reader(ROOT)as reader:
  ref=lambda n:dict(record=n,sha256=reader.digest(n));require(reader.digest(REVIEW['record'])==REVIEW['sha256'],'Caller review changed');review=reader.json(REVIEW['record']);execution_ref=ref(P+'runtime29-caller-retained-analysis-execution-02.json');execution=reader.json(execution_ref['record']);require(execution['success']and execution['plan']==REVIEW and execution['original_game_runs']==0 and execution['new_public_player_executions']==0,'Incomplete retained analysis');comparison=execution['comparison'];comp=reader.json(comparison['record']);require(comp['success']and comp['comparison_processes_stopped'],'Caller comparison failed/still running');acquisition=review['acquisition'];acquired=reader.json(acquisition['record']);require(acquired['success']and acquired['private_wine_stopped']and acquired['prefix_pool']['returned'],'Caller execution still active');sources={n:reader.digest(n)for n in python_sources(ROOT,['tools/complete-broader-runtime-caller-copy.py'])}
 with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;require(dict(db.execute('select *from requests where case_id=?',(FIRST,)).fetchone())==review['request_row'],'Acquired caller request changed')
 validation=P+FIRST+'-validation-01.json';require(not(ROOT/validation).exists(),'Fresh standard validation required')
 for stage,command in [('validate',['python3','tools/check-broader-relative-world.py','--acquisition',acquisition['record'],'--comparison',comparison['record'],'--report',validation]),('admit',['python3','tools/run-broader-runtime-source-case.py','admit','--plan',review['phase']['record'],'--case',FIRST,'--validation',validation])]:
  run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True);log_name=P+FIRST+'-copy-only-'+stage+'.bin';log_ref=dict(record=log_name,sha256=write_report(ROOT,log_name,(run.stdout+run.stderr).encode()));step_results.append(dict(stage=stage,command=command,returncode=run.returncode,log=log_ref));print(json.dumps(dict(stage=stage,returncode=run.returncode)),flush=True);require(run.returncode==0,'Standard caller validation/admission refused: '+stage)
 with Reader(ROOT)as reader:
  vr=dict(record=validation,sha256=reader.digest(validation));v=reader.json(validation);require(v['success']and v['sources_stable']and v['lifetime'],'Incomplete caller lifetime validation');source_bytes={n:reader.read_bytes(n)for n in v['sources']};require(all(hashlib.sha256(body).hexdigest()==v['sources'][n]for n,body in source_bytes.items()),'Caller source payload differs')
 paths=[Path(acquired['temporary_root']),Path(acquired['session']),ROOT/'working/caller-state-comparisons'/Path(comparison['record']).stem];require(len(set(paths))==3,'Caller copy roots overlap');plans=[]
 for path in paths:
  safe(ROOT,path);idle(path);require(not(path/MARKER).exists(),'Caller copy source already replaced');members,identity,allocated=scan(path);plans.append(dict(path=str(path),members=members,allocated_bytes=allocated))
 plan=dict(schema=1,kind='validated_caller_copy_only_custody_plan',success=True,original_game_runs=0,validation=vr,paths=plans,removal_authorized=False);plan_name=P+FIRST+'-handoff-01-plan.json';plan_ref=dict(record=plan_name,sha256=write_report(ROOT,plan_name,(json.dumps(plan,indent=2)+'\n').encode()))
 with configured(ROOT)as pair:
  with pair.batch():source_manifest=pair.root('caller_copy_only_analysis_sources',FIRST,dict(schema=1,sources={n:dict(sha256=pair.bytes(body),size=len(body))for n,body in source_bytes.items()}))
  results=[seal(ROOT,path,pair,retire=False,restore_budget=8*1024**3)for path in paths]
 require(all(not r['retired']and r['real_restore_verified']and r['replicas_verified']==2 for r in results),'Copy-only verification incomplete')
 for path in paths:require(path.exists()and not(path/MARKER).exists(),'Original caller path was replaced')
 handoff=dict(schema=1,kind='admitted_broader_domain_handoff',success=True,original_game_runs=0,validation=vr,plan=plan_ref,analysis_sources=source_manifest,results=results,source_trees_held=True,retirement_performed=False,scope='Non-destructive copies and real restores of the validated first caller capture. Every original tree and pin remains in place; explicit approval is required before a later distinct retirement. No rejected archival action is invoked.')
 handoff_name=P+FIRST+'-handoff-01.json';write_report(ROOT,handoff_name,(json.dumps(handoff,indent=2)+'\n').encode())
 ram_report=P+FIRST+'-RAM-handoff-01.json';run=subprocess.run(['python3','tools/retain-broader-controller-ram.py','--validation',validation,'--report',ram_report],cwd=ROOT,capture_output=True,text=True);ln=P+FIRST+'-copy-only-RAM-log.bin';write_report(ROOT,ln,(run.stdout+run.stderr).encode());require(run.returncode==0,'Held controller RAM custody refused')
 require(all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items()),'Copy-only coordinator source changed')
 q=dict(schema=1,kind='broader_domain_case_coordinator',success=True,case=FIRST,family='caller',group='world',plan=review['phase'],validation=vr,acquisition=acquisition,previous_failed_coordinator=review['failed_coordinator'],retained_analysis_plan=REVIEW,retained_analysis_execution=execution_ref,original_game_runs_added=0,new_acquisition_performed=False,source_trees_held=True,retirement_performed=False,automatic_retries=False,steps=step_results,sources=sources,source=dict(path='tools/complete-broader-runtime-caller-copy.py',sha256=sources['tools/complete-broader-runtime-caller-copy.py']),scope='Standard validation and admission plus two-copy real-restore custody only. Every original execution/comparison/RAM tree remains in place. This distinct coordinator preserves the failed01record and performs no new game or destructive archival.')
 digest=write_report(ROOT,report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=digest,trees=len(results),originals_held=True)),flush=True)
 return 0
if __name__=='__main__':raise SystemExit(main())
