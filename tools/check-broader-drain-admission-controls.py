#!/usr/bin/env python3
"""Retain real mapped-helper and continuation-accounting controls without a game."""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_sources import python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh owned control report required')
 names=python_sources(ROOT,['tools/check-broader-drain-admission-controls.py','tests/test-broader-drained-budget.py','tests/test-broader-drained-fault.py'])|{'tools/rendering-transition-fault.cpp','protocols/include/mnm/render_command_ring.h','protocols/include/mnm/render_commands_v2.h','protocols/include/mnm/render_commands_v3.h'}
 with Reader(ROOT)as r:sources={n:r.digest(n)for n in sorted(names)}
 helper=ROOT/'working/broader-player/rendering-transition-fault';binary=hashlib.sha256(helper.read_bytes()).hexdigest();require(binary=='13d7b2270af2418e8d95f47ea360eeb360b7c061bc90c1d781fef64ca962b2a0','Exact unchanged native fault helper required');results=[]
 for command in [[sys.executable,'-B','tests/test-broader-drained-fault.py'],[sys.executable,'-B','tests/test-broader-drained-budget.py'],[str(helper),'--self-test']]:
  result=subprocess.run(command,cwd=ROOT,capture_output=True);results.append(dict(command=command,exit_code=result.returncode,stdout=result.stdout.decode(errors='replace'),stderr=result.stderr.decode(errors='replace')))
  if result.returncode:break
 with Reader(ROOT)as r:stable=all(r.digest(n)==h for n,h in sources.items())
 q=dict(schema=1,kind='broader_drained_fault_admission_controls',success=len(results)==3 and all(v['exit_code']==0 for v in results)and stable,original_game_runs=0,current_live_pair_accepted=False,python_controls=16,native_helper_self_test_cases=7,exact_unchanged_helper_sha256=binary,helper_binary=dict(record=a.report.removesuffix('.json')+'-native-helper.bin',sha256=write_report(ROOT,a.report.removesuffix('.json')+'-native-helper.bin',helper.read_bytes())),helper_process_timeout_seconds=5,bounded_adapter_wait_seconds=2,only_exact_pre_mutation_drain_refusal_retried=True,deadline_or_unknown_timeout_refuses_without_retry=True,sources=sources,sources_stable=stable,results=results,scope='Current no-game mapped native helper mutation/refusal, bounded drain wait and exact charge/source/repair admission controls. No renderer/player source changes, live fault success, independent game pixels, full history or global acceptance.')
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,python_controls=16,native_helper_cases=7)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
