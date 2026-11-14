#!/usr/bin/env python3
"""Finish one of two exact B06 hotspot retained analyses without another game."""
import argparse,hashlib,json,sqlite3,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import seal
from rendering_pilot import require
from broader_player_copy_inputs_matrix import specs,P,BINARY
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--case',required=True,choices=['default640','default800']);a=p.parse_args()
 cid='bp27-world-hotspot-'+a.case+'-01';fresh=a.case=='default640';out=ROOT/'working/broader-player/retained-hotspot-analysis'/cid;out.mkdir(parents=True,exist_ok=True);(out/'.retain').touch()
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));spec=next(v for v in specs(r)if v['case']['id']==cid);case=spec['case'];ar=ref(case['report']);capture=r.json(ar['record']);comparison=case['comparison'].removesuffix('-01.json')+('-02.json'if fresh else'-03.json');c=r.json(comparison)
  require(c['success']and c['sources_stable']and c['acquisition']==ar and c['comparison_processes_stopped']and c['comparison']['mismatching_pixels']==0 and all(r.digest(n)==h for n,h in c['sources'].items()),'Current complete retained analysis required')
  require(capture['player_binary']['sha256']==BINARY and all(r.digest(n)==h for n,h in capture['sources'].items()),'Original capture/Native source changed')
  with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
   db.row_factory=sqlite3.Row;row=dict(db.execute('select *from requests where case_id=?',(cid,)).fetchone())
  require(row['state']==('acquired'if fresh else'admitted'),'Wrong retained request state');oldco=ref(P+cid+'-coordinator-01.json');old=r.json(oldco['record']);require(old['success']is(not fresh),'Unexpected historical coordinator state')
  oldv=ref(row['validation'])if row['validation']else None
  source_names=['tools/cursor_hotspot_analysis.py','tests/test-cursor-hotspot.py','tools/broader_player_copy_matrix.py','tools/broader_player_hotspot_analysis_matrix.py','tests/test-broader-analysis-revalidation.py','tools/resume-broader-hotspot-analysis.py'];sources={n:r.digest(n)for n in source_names}
 validation=P+cid+'-validation-02.json';handoff=P+cid+('-handoff-01.json'if fresh else'-handoff-02.json');co_name=P+cid+'-coordinator-02.json';steps=[]
 def run(stage,command):
  start=time.time_ns();log=out/(stage+'.log')
  with log.open('xb')as f:result=subprocess.run(command,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
  steps.append(dict(stage=stage,command=command,returncode=result.returncode,start_ns=start,finish_ns=time.time_ns(),log_path=str(log)))
  print(json.dumps(dict(case=cid,stage=stage,returncode=result.returncode)),flush=True);require(result.returncode==0,'Retained analysis step failed: '+stage)
 error=None;negative=None
 try:
  run('validate',['python3','tools/check-broader-relative-world.py','--acquisition',ar['record'],'--comparison',comparison,'--report',validation])
  if fresh:run('admit',['python3','tools/run-broader-copy-source-case.py','admit','--plan',spec['plan'],'--case',cid,'--validation',validation])
  run('retain-execution',['python3','tools/retire-broader-domain-case.py','--validation',validation,'--report',handoff])
  if fresh:
   run('retain-RAM',['python3','tools/retire-broader-capture-copy.py','--validation',validation,'--report',P+cid+'-RAM-handoff-01.json'])
   # Preserve the negative comparison tree separately; never overwrite its report.
   path=ROOT/'working/interaction-comparisons'/Path(case['comparison']).stem
   with configured(ROOT)as pair:negative=seal(ROOT,path,pair,retire=True,restore_budget=2*1024**3)
  else:
   with Reader(ROOT)as r:
    original_RAM=r.json(P+cid+'-RAM-handoff-01.json');require(original_RAM['success'],'Original RAM custody missing')
 except Exception as exc:error=repr(exc)
 with Reader(ROOT)as r:
  for step in steps:
   n=P+cid+'-retained-analysis-'+step['stage']+'.bin';step['log']=dict(record=n,sha256=write_report(ROOT,n,Path(step['log_path']).read_bytes()))
  co=dict(schema=1,kind='broader_retained_hotspot_analysis_coordinator',success=error is None,error=error,case=cid,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=1,automatic_retries=False,steps=steps,source=dict(path='tools/resume-broader-hotspot-analysis.py',sha256=sources['tools/resume-broader-hotspot-analysis.py']),sources=sources,original_coordinator=oldco,original_validation=oldv,negative_comparison_tree=negative)
  if error is None:co.update(validation=dict(record=validation,sha256=r.digest(validation)),acquisition=ar,handoff=dict(record=handoff,sha256=r.digest(handoff)))
  ch=write_report(ROOT,co_name,(json.dumps(co,indent=2)+'\n').encode())
  if error is None:
   b=dict(schema=1,kind='broader_retained_hotspot_analysis_revalidation',success=True,case=cid,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=1,Native_ELF_sha256=BINARY,request_identity={k:row[k]for k in('case_id','report','plan','reserved_games','started_ns','acquisition_sha256')},ledger_validation=validation if fresh else row['validation'],acquisition=ar,plan=dict(record=spec['plan'],sha256=r.digest(spec['plan'])),source_phase_waivers=[],original_coordinator=oldco,original_coordinator_success=not fresh,original_validation=oldv,current_validation=co['validation'],current_coordinator=dict(record=co_name,sha256=ch),current_handoff=co['handoff'],raw_capture_and_Native_inputs_unchanged=True,historical_failed_comparison_preserved=True,sources=sources,diagnosis=dict(record=P+'B06-hotspot-readiness-diagnosis-01.json',sha256=r.digest(P+'B06-hotspot-readiness-diagnosis-01.json')),scope='Only fresh retained-data analysis and its current validation/retention supersede the earlier analysis. Acquisition, original request/start/charge, Native binary/runtime, prospective action/clock/three-generation domain and every old report remain unchanged. A captured same-World complete swap must precede the original readiness deadline; selection never uses pixel agreement. No automatic new game or Native-source waiver.')
   n=P+cid+'-analysis-revalidation-01.json';h=write_report(ROOT,n,(json.dumps(b,indent=2)+'\n').encode());print(json.dumps(dict(success=True,case=cid,sha256=h)),flush=True)
  else:print(json.dumps(dict(success=False,case=cid,error=error,coordinator_sha256=ch)),flush=True)
 return int(error is not None)
if __name__=='__main__':raise SystemExit(main())
