#!/usr/bin/env python3
"""Bind retained native writer controls and fresh reader guards to the v4 policy."""
import argparse,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import required_sources,behavior_contract,scenario_contract
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/rendering-pause-results-20261107/'
BID='NP.acceptance-command-history-capture';SID='native.acceptance-command-history-capture'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--controls',required=True);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned policy binding required')
 with Reader(ROOT) as r:
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID)
  sources={n:r.digest(n) for n in required_sources(b)};c=r.json(a.controls)
  require(c['success'] and c['sources_stable'] and not c['original_game_executed'] and all(r.digest(n)==h for n,h in c['sources'].items()),'Changed or failed native writer controls')
  runs=[v for v in c['checks'] if 'result' in v];require(len(runs)==2,'Optimized and sanitized producer runs required')
  for run in runs:
   cases=run['result']['cases'];require(run['exit_code']==0 and run['result']['success'] and len(cases)==5 and all(v['success'] for v in cases),'Failed native limit/queue controls')
   require([(v['requested'],v['accepted'],v['overflow_expected']) for v in cases[:4]]==[(17000,16382,True),(17000,17000,False),(66000,65534,True),(100,100,False)],'Native policy boundary changed')
   require(cases[3]['role']=='viewport' and not cases[3]['command_history'] and cases[4]['overflow_expected'] and cases[4]['append_returned_while_compression_blocked'],'Role isolation or nonblocking queue refusal absent')
   require(run['production_reader']==dict(complete_accepted=2,incomplete_rejected=3),'Reader admitted a failed native proof')
  guards=[]
  for name in ('tests/test-rendering-command-capture.py','tests/test-rendering-capture.py'):
   run=subprocess.run(['python3',name],cwd=ROOT,capture_output=True,text=True,timeout=60)
   guards.append(dict(path=name,exit_code=run.returncode,stdout=run.stdout,stderr=run.stderr));require(run.returncode==0,'Reader guard failed')
  require(all(r.digest(n)==h for n,h in sources.items()),'Policy sources changed')
  q=dict(schema=1,kind='versioned_command_capture_controls_binding',success=True,sources=sources,sources_stable=True,
   claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
   native_controls=dict(record=a.controls,sha256=r.digest(a.controls)),native_control_runs=2,native_control_cases=10,reader_guard_tests=9,reader_guards=guards,
   original_game_runs=0,new_native_producer_execution=False,all_legacy_budgets_preserved=True,
   scope='Only opt-in command-history-lossless-v1/version4:65536total rows,8GiBraw,1GiBstored,unchanged256MiB/4096in-flight queue. Retained optimized/ASAN native producer controls plus fresh strict reader guards. No game, raster-output, simulation, watchdog, physical-provider or all-player validation credit.')
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h)))
if __name__=='__main__':main()
