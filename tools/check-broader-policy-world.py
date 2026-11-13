#!/usr/bin/env python3
"""Require native edge certificates plus all unchanged specialized World checks."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_world_policy import assess as policy_assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('acquisition','comparison','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(not(ROOT/args.report).exists(),'Fresh policy World validation required')
 with Reader(ROOT)as r:
  try:
   c=r.json(args.comparison);edge=policy_assess(r,c);filename=ROOT/'tools/check-broader-world.py';spec=importlib.util.spec_from_file_location('broader_unmodified_world_admission',filename);base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
   q=base.assess(r,args.acquisition,args.comparison);q['native_edge_policy']=edge;q['sources'].update(edge['oracle_sources']|{n:r.digest(n)for n in('tools/check-broader-policy-world.py','tools/broader_world_policy.py')});q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items());q['success']&=q['sources_stable']
  except(ValueError,KeyError,TypeError,OSError)as e:q=dict(schema=1,kind='current_broader_world_case_validation',success=False,error=str(e),original_game_runs=0,acquisition=dict(record=args.acquisition,sha256=r.digest(args.acquisition)))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
