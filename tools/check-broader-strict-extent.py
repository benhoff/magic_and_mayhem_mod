#!/usr/bin/env python3
"""Add observed native extent to the unchanged strict functional admission."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_strict_extent import observe
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('acquisition','report','analysis-plan'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh strict actual-context validation required')
 with Reader(ROOT)as r:
  try:
   plan=r.json(args.analysis_plan);require(plan['success']and plan['original_game_runs']==0 and all(r.digest(n)==h for n,h in plan['sources'].items()),'Changed strict extent controls');a=r.json(args.acquisition);require(a['broader_phase']==plan['strict_phase'],'Foreign strict source phase')
   filename=ROOT/'tools/check-broader-strict.py';spec=importlib.util.spec_from_file_location('broader_unchanged_strict_admission',filename);base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base);q=base.assess(r,args.acquisition);q['actual_context']=observe(ROOT,a,plan['actual_native_frame_size']);q['actual_context_analysis']=dict(record=args.analysis_plan,sha256=r.digest(args.analysis_plan));q['sources'].update(plan['sources']);q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items());q['success']&=q['sources_stable']
  except(ValueError,KeyError,OSError,TypeError)as e:q=dict(schema=1,kind='current_broader_strict_campaign_validation',success=False,original_game_runs=0,error=str(e),acquisition=dict(record=args.acquisition,sha256=r.digest(args.acquisition)))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
