#!/usr/bin/env python3
"""Reproduce finite-edge oracle and fail-closed acceptance controls, with no game."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--attempt',required=True);args=p.parse_args();assert args.attempt.isdecimal()
 out=Path('/dev/shm/mnm-broader-player-20261110')/('policy-controls-'+args.attempt);out.mkdir();prefix=P+'finite-edge-controls-'+args.attempt;q=dict(schema=1,kind='native_finite_edge_oracle_controls',success=False,original_game_runs=0,artifacts=[])
 def keep(name,data):
  record=prefix+'-'+name+'.bin';ref=dict(record=record,sha256=write_report(ROOT,record,data),bytes=len(data));q['artifacts'].append(ref);return ref
 names=['tools/run-broader-policy-controls.py','tools/broader_finite_edge_oracle.hpp','tools/broader_policy_acceptance.py','tools/check-broader-policy-recovery.py','tests/broader-finite-edge-oracle-test.cpp','tests/test-broader-policy-recovery.py','tests/test-broader-policy-acceptance.py','tests/test-broader-player-case.py','tools/check-broader-player-case.py']
 with Reader(ROOT)as r:q['sources']={n:r.digest(n)for n in names}
 try:
  binary=out/'finite-edge-controls';commands=[['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/broader-finite-edge-oracle-test.cpp'),'-o',str(binary)],[str(binary)],*[["python3",str(ROOT/n)]for n in names if n.startswith('tests/test-')]]
  q['commands']=commands
  for i,cmd in enumerate(commands):
   run=subprocess.run(cmd,cwd=ROOT,capture_output=True,timeout=60);keep('run-%02d'%i,run.stdout+run.stderr)
   if run.returncode:raise ValueError('Control command failed: '+str(i))
   if i==1:q.update(json.loads(run.stdout))
  keep('binary',binary.read_bytes());q['guard_sha256']=hashlib.sha256((ROOT/'tools/broader_finite_edge_oracle.hpp').read_bytes()).hexdigest();q['binding_tests']=5;q['admission_tests']=9;q['case_tests']=7
  with Reader(ROOT)as r:q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items())
  q['success']=q['sources_stable']
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,prefix+'.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'))),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
