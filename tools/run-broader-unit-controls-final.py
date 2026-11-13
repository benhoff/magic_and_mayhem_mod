#!/usr/bin/env python3
"""Run isolated broader-player tooling controls; never start the original game."""
import argparse,json,re,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract
from broader_player_sources import python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh isolated tooling control report required')
 names=sorted(v.relative_to(ROOT).as_posix()for v in(ROOT/'tests').glob('test-broader-*.py'))+['tests/test-continuation-input-binding.py','tests/test-owned-session-journal.py','tests/test-binding-continuation-budget.py','tests/test-history-continuation-budget.py','tests/test-command-history-launch.py','tests/test-namespace-continuation-budget.py','tests/test-counter-namespace.py','tests/test-history-capture-contract.py','tests/test-world-preferences-artifact.py','tests/test-delegated-plan-binding.py','tests/test-delegated-plan-budget.py'];require(len(names)==48,'Review the isolated test inventory before changing it')
 with Reader(ROOT)as r:sources={n:r.digest(n)for n in python_sources(ROOT,names+['tools/run-broader-unit-controls-final.py','tools/owned-menu-game-runner.py'])}
 results=[];count=0
 for name in names:
  command=[sys.executable,name];v=subprocess.run(command,cwd=ROOT,capture_output=True);log=v.stdout+v.stderr;matches=re.findall(rb'Ran (\d+) tests? in ',log);count+=sum(int(x)for x in matches);results.append(dict(command=command,returncode=v.returncode,reported_tests=sum(int(x)for x in matches),log=log.decode(errors='replace')))
  print(json.dumps(dict(test=name,returncode=v.returncode)),flush=True)
  if v.returncode:break
 with Reader(ROOT)as r:stable=all(r.digest(n)==h for n,h in sources.items())
 q=dict(schema=1,kind='broader_isolated_tooling_controls',success=count==324 and len(results)==len(names)and all(v['returncode']==0 for v in results)and stable,original_game_runs=0,tests=count,files=len(results),sources=sources,sources_stable=stable,results=results,current_whole_player_proved=False,supported_player_accepted=False,scope='Isolated Python planning/binding/admission/provenance/retention/aggregate controls, including synthetic paired archive/restore fixtures. No original-game acquisition, native production execution, pixel replay, current journey or overall acceptance follows.')
 with Reader(ROOT)as r:
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']=='NP.owned-session-journal-publication');sc=next(v for v in reg['scenarios']if v['id']=='native.owned-session-journal-publication');q['claims']=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={sc['id']:scenario_contract(sc)})]
 q['new_continuation_controls']=dict(binding_budget=9,history_budget=9,history_launch=9,namespace_budget=9,counter_namespace=5,actual_capture_policy=4,recorded_preferences_identity=7,delegated_plan_binding=7,delegated_plan_budget=9)
 q['publication_controls']=dict(tests=8,legacy_JSON_bytes_preserved=True,existing_receipts_never_replaced=True,partial_final_JSON_visible=False,consumer_assertions_or_game_deadlines_changed=False,live_failure_cause_proved=False)
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,tests=count,files=len(results))),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
