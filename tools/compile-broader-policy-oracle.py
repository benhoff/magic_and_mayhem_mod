#!/usr/bin/env python3
"""Build a separately identified retained-edge oracle around unchanged PE calls."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--attempt',required=True);args=p.parse_args();assert args.attempt.isdecimal()
 output=Path('/dev/shm/mnm-broader-player-20261110')/('policy-reference-build-'+args.attempt);output.mkdir()
 source=ROOT/'tests/acceptance-world-reference.cpp';raw=source.read_text();needle='int main(int argc,char** argv)try{';assert raw.count(needle)==1
 # Only the Linux harness entry is renamed. No original call/assertion changes.
 generated=raw.replace(needle,'int broader_original_policy_main(int argc,char** argv)try{');inc=output/'policy-original-executor.inc';inc.write_text(generated)
 binary=output/'original-policy-reference';command=['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','-I'+str(ROOT/'tests'),'-I'+str(ROOT/'tools'),'-DMNM_BROADER_POLICY_EXECUTOR="'+str(inc)+'"','-MD','-MF',str(output/'dependencies.d'),str(ROOT/'tools/broader-world-policy-reference.cpp'),'-o',str(binary)]
 run=subprocess.run(command,capture_output=True);prefix=P+'policy-reference-build-'+args.attempt
 def retained(suffix,body):
  n=prefix+'-'+suffix+'.bin';return dict(record=n,sha256=write_report(ROOT,n,body),bytes=len(body))
 q=dict(schema=1,kind='native_finite_edge_oracle_build',success=run.returncode==0,original_game_runs=0,production_renderer_changed=False,command=command,log=retained('log',run.stdout+run.stderr),compiler=subprocess.check_output(['g++','--version'],text=True),generated_executor=retained('executor',generated.encode()),derivation='Exactly one host harness main entry renamed; original draw calls, AX assertions and PE instructions unchanged.')
 with Reader(ROOT)as r:
  names=['tests/acceptance-world-reference.cpp','tests/sprite-binary-reference.cpp','tests/world-raster-reference.hpp','tools/broader_finite_edge_oracle.hpp','tools/broader-world-policy-reference.cpp','tools/compile-broader-policy-oracle.py']
  q['sources']={n:r.digest(n)for n in names}
 if q['success']:q['binary']=dict(path=str(binary),**retained('binary',binary.read_bytes()))
 h=write_report(ROOT,prefix+'.json',(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h)),flush=True)
 if run.returncode:print((run.stdout+run.stderr).decode(errors='replace')[-3000:])
 return run.returncode
if __name__=='__main__':raise SystemExit(main())
