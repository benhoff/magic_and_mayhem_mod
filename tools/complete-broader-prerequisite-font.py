#!/usr/bin/env python3
"""Validate and retain the existing font pilot after its reviewed control rebinding."""
import hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
CASE='bp32-world-font-default800-01'
def main():
 name=P+CASE+'-coordinator-02.json';require(not(ROOT/name).exists(),'Fresh successor coordinator required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));review=ref(P+'prerequisite-font-retained-analysis-review-01.json');plan=r.json(review['record']);execution=ref(P+'prerequisite-font-retained-analysis-execution-01.json');e=r.json(execution['record']);require(plan['success']and e['success']and e['review']==review and e['request_rows_unchanged']and e['original_game_runs']==e['new_public_player_executions']==e['new_request_reservations']==0,'Retained-data-only execution required');comparison=e['comparison'];c=r.json(comparison['record']);require(r.digest(comparison['record'])==comparison['sha256']and c['success']and c['native_font_controls']==plan['current_control']and c['comparison_processes_stopped'],'Incomplete font comparison');acquisition=plan['acquisition'];require(r.digest(acquisition['record'])==acquisition['sha256'],'Acquisition changed');sources={'tools/complete-broader-prerequisite-font.py':r.digest('tools/complete-broader-prerequisite-font.py')}
 validation=P+CASE+'-validation-01.json';steps=[]
 commands=[('validate',['python3','tools/check-broader-relative-world.py','--acquisition',acquisition['record'],'--comparison',comparison['record'],'--report',validation]),('admit',['python3','tools/run-broader-prerequisite-source-case.py','admit','--plan',plan['phase']['record'],'--case',CASE,'--validation',validation]),('retain-execution',['python3','tools/retire-broader-domain-case.py','--validation',validation,'--report',P+CASE+'-handoff-01.json']),('retain-RAM-capture',['python3','tools/retire-broader-capture-copy.py','--validation',validation,'--report',P+CASE+'-RAM-handoff-01.json'])]
 for stage,cmd in commands:
  run=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True);ln=P+CASE+'-successor-'+stage+'.bin';steps.append(dict(stage=stage,command=cmd,returncode=run.returncode,log=dict(record=ln,sha256=write_report(ROOT,ln,(run.stdout+run.stderr).encode()))));print(json.dumps(dict(stage=stage,returncode=run.returncode,output=(run.stdout+run.stderr)[-1500:])),flush=True);require(run.returncode==0,'Stop after failed font successor step: '+stage)
 with Reader(ROOT)as r:
  vr=dict(record=validation,sha256=r.digest(validation));require(all(r.digest(n)==h for n,h in sources.items()),'Successor source changed')
 q=dict(schema=1,kind='broader_domain_case_coordinator',success=True,case=CASE,family='font',group='world',plan=plan['phase'],validation=vr,acquisition=acquisition,previous_failed_coordinator=plan['failed_coordinator'],retained_analysis_review=review,retained_analysis_execution=execution,original_game_runs_added=0,new_acquisition_performed=False,automatic_retries=False,steps=steps,sources=sources,scope='Review/admission and approved positive-only two-copy real-restore retention of the exact existing successful capture. Failed comparison and coordinator remain immutable. No new game,request or source waiver.')
 h=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h)),flush=True)
if __name__=='__main__':main()
