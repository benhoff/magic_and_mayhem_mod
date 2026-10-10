#!/usr/bin/env python3
"""Compare guarded rolling live replies with every actual original raster and AX result."""
import argparse, hashlib, json, os, subprocess, tempfile
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--live-run',type=Path,required=True);a=p.parse_args()
 parent=ROOT/'working/tests/native-world-rolling-original';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in r['behaviors'] if x['id']=='RS.world-rolling-handoff-equivalence');s=next(x for x in r['scenarios'] if x['id']=='native-world-rolling-original-20261010');sources={n:sha(ROOT/n) for n in sorted(required_sources(b))};claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})];(out/'claims.json').write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
 report=dict(success=False,sources=sources,claims=claims,scope=b['scope'],original_pixels_used_as_native_inputs=False)
 try:
  live=json.loads((a.live_run/'report.json').read_text());assert live['success'] and live['mode']=='complete';capture=Path(live['experiment'])/'report.json'
  with (out/'comparison.log').open('x') as log:subprocess.run(['python3',str(ROOT/'tools/test-world-raster-batch.py'),'--capture-report',str(capture),'--claims',str(a.live_run/'claims.json')],cwd=ROOT,env=dict(os.environ,LP_NUM_THREADS='8'),stdout=log,stderr=subprocess.STDOUT,check=True,timeout=900)
  rows=[Path(x) for x in (out/'comparison.log').read_text().splitlines() if x.endswith('/report.json') and x.startswith(str(ROOT/'working/tests/world-raster-batch/run-'))];assert len(rows)==1
  result=json.loads(rows[0].read_text());assert result['success'] and result['sources_stable'] and result['inputs_stable'] and result['original_manifest_verified_before_after']
  report.update(comparison=result,live_report_sha256=sha(a.live_run/'report.json'),comparison_report=str(rows[0]),comparison_report_sha256=sha(rows[0]),original_manifest_verified_before_after=True,success=True)
 except Exception as e:report['error']=str(e)
 finally:
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
