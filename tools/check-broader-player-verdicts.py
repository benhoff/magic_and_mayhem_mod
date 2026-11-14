#!/usr/bin/env python3
"""Adjudicate only a completed current finite conjunction; keep recovery independent."""
import argparse,json
from pathlib import Path
from broader_player_verdicts import issue
from broader_player_acceptance import checked
from broader_player_sources import python_sources
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['acceptance','report']:p.add_argument('--'+n,required=True)
 a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh verdict record required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));ar=ref(a.acceptance);current=checked(r,ar);require(all(r.digest(n)==h for n,h in current['sources'].items()),'Current conjunction sources changed');boundary=checked(r,current['required_boundary_impact']);parent_ref=boundary['retained_finite_master'];parent=checked(r,parent_ref);old_ref=parent['decision_record'];prior=checked(r,old_ref);decision=issue(current,parent,prior)
  sources=current['sources']|{n:r.digest(n)for n in python_sources(ROOT,['tools/check-broader-player-verdicts.py'])};require(all(r.digest(n)==h for n,h in sources.items()),'Verdict source drift')
  q=dict(schema=1,kind='broader_current_independent_rendering_verdicts',success=True,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_acceptance=ar,finite_requirement_parent=parent_ref,finite_requirement_rows=parent['requirements'],finite_requirement_rows_sha256=fingerprint(parent['requirements']),all156parent_rows_preserved_exactly=True,previous_decision_record=old_ref,scope_amendments=parent['scope_amendments'],historical_failures_retired=False,sources=sources,sources_stable=True,claims=current['claims'],**decision)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,decisions=q['decisions'],original_game_runs=0)),flush=True)
if __name__=='__main__':main()
