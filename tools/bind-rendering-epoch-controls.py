#!/usr/bin/env python3
"""Bind actual optimized/sanitized epoch recorders and retained legacy guards."""
import argparse,json,subprocess
from pathlib import Path
from coverage_claims import required_sources,behavior_contract,scenario_contract
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/rendering-transition-recovery-results-20261107/'
BID='NP.capture-epoch-bindings';SID='native.capture-epoch-bindings'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh epoch control binding required')
 with Reader(ROOT) as r:
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID);sources={n:r.digest(n) for n in required_sources(b)};inputs=[]
  for name in ('epoch-controls-02.json','legacy-writer-01.json'):
   record=PREFIX+name;q=r.json(record);require(q['success'] and q['sources_stable'] and all(r.digest(n)==h for n,h in q['sources'].items()),'Native controls source phase differs');inputs.append(dict(record=record,sha256=r.digest(record)))
   if name.startswith('epoch'):
    require(q['native_cases']==18 and q['original_game_runs']==0,'Incomplete epoch controls')
    runs=[v for v in q['checks'] if v['stage']=='execute'];require({v['mode'] for v in runs}=={'optimized','sanitized'} and len(runs)==2,'Missing native producer variants')
    for run in runs:require(run['exit_code']==0 and run['result']['success'] and len(run['result']['checks'])==9 and run['reader']==dict(complete=4,refused=2,producer_ids=[101,202,303]),'Failed actual epoch writer or reader controls')
   else:
    require(q['original_game_executed'] is False,'Legacy controls started an original game');runs=[v for v in q['checks'] if 'result' in v];require(len(runs)==2,'Missing native legacy variants')
    for run in runs:require(run['exit_code']==0 and run['result']['success'] and len(run['result']['cases'])==5 and all(v['success'] for v in run['result']['cases']) and run['production_reader']==dict(complete_accepted=2,incomplete_rejected=3),'Legacy budget/queue refusal changed')
  guards=[]
  for name in ('tests/test-rendering-command-capture.py','tests/test-rendering-capture.py'):
   run=subprocess.run(['python3',name],cwd=ROOT,capture_output=True,text=True,timeout=60);guards.append(dict(path=name,exit_code=run.returncode,stdout=run.stdout,stderr=run.stderr));require(run.returncode==0,'Strict reader guard failed')
  require(all(r.digest(n)==h for n,h in sources.items()),'Epoch policy source drift')
  q=dict(schema=1,kind='native_epoch_capture_controls_binding',success=True,sources=sources,sources_stable=True,claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],native_inputs=inputs,native_epoch_cases=18,native_legacy_cases=10,reader_guard_tests=9,reader_guards=guards,original_game_runs=0,new_native_execution=False,scope=b['scope'])
  print(write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
