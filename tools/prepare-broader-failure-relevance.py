#!/usr/bin/env python3
"""Inspect historical startup source impact before current normal acquisitions."""
import argparse,json,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh historical relevance preflight required')
 command=[sys.executable,'tools/evidence-applicability.py','plan','--evidence','RN31.quick-watchdog','--behavior','NP.rendering-normal-acceptance'];v=subprocess.run(command,cwd=ROOT,capture_output=True);require(v.returncode==0,'Startup source impact failed');name=args.report.removesuffix('.json')+'-plan.bin';h=write_report(ROOT,name,v.stdout+v.stderr)
 with Reader(ROOT)as r:
  old='research/runtime/rendering-normal-results-20261031/quick-acquisition-03.json';a=r.json(old);require(a['success']is False and a['configuration']['skip_intros']is False,'Historical startup failure changed')
  q=dict(schema=1,kind='broader_historical_failure_relevance_preflight',success=True,planning_only=True,original_game_runs=0,source_impact=dict(command=command,record=name,sha256=h),historical_startup=dict(record=old,sha256=r.digest(old)),historical_startup_success=False,required_current_coupling='Both naturally completed intros followed by original Quick selection, first independently compared World and advancing current source generations in the same process, both profiles/extents; no skipped movie or separate movie proof substitutes.',strict_obligation='Exact10and30second two-phase B05stress on both current providers, original Mini/readiness/callback/resume/normal-Quit and zero-readback/forward/retirement assertions.',fps_obligation='B06instrumented17.253776873910518FPS and19FPSfailure remain diagnostic/historical; no fixed FPS completion gate.',causal_claims_authorized=False,historical_failures_retired=False,current_acceptance_proved=False,sources={n:r.digest(n)for n in('tools/prepare-broader-failure-relevance.py','tools/evidence-applicability.py','docs/native-rendering-closure.json')})
  sha=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=sha,original_game_runs=0)))
if __name__=='__main__':main()
