#!/usr/bin/env python3
"""Create a source-bound UI projection only from completed independent verdicts."""
import argparse,json
from pathlib import Path
from broader_player_dashboard import KIND,PROJECTION_KIND,READER,validate
from broader_player_sources import python_sources
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ('acceptance','decisions','report'):p.add_argument('--'+key,required=True)
 args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh owned dashboard projection required')
 with Reader(ROOT)as reader:
  def checked(ref):
   require(reader.digest(ref['record'])==ref['sha256'],'Dashboard parent identity changed')
   return reader.json(ref['record'])
  ref=lambda n:dict(record=n,sha256=reader.digest(n))
  ar,dr=ref(args.acceptance),ref(args.decisions);current,verdict=checked(ar),checked(dr)
  parent_ref=checked(current['required_boundary_impact'])['retained_finite_master'];parent=checked(parent_ref)
  sources=dict(current['sources'])
  for n,h in verdict['sources'].items():
   require(n not in sources or sources[n]==h,'Mixed acceptance/decision source phases');sources[n]=h
  for n in python_sources(ROOT,['tools/project-broader-player-dashboard.py',READER]):
   h=reader.digest(n);require(n not in sources or sources[n]==h,'Projection implementation changed after acceptance');sources[n]=h
  q=dict(schema=1,kind=KIND,projection_kind=PROJECTION_KIND,success=True,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_acceptance=ar,independent_decisions=dr,finite_requirement_parent=parent_ref,requirements=parent['requirements'],proof=parent['proof'],decisions=verdict['decisions'],scope=current['scope'],sources=sources)
  validate(q,checked);require(all(reader.digest(n)==h for n,h in sources.items()),'Only current completed verdicts may be projected')
  q['sources_stable']=True
  q['compatibility_scope']='The existing UI consumes the unchanged 156 inherited requirement rows, eight unchanged boundary dispositions and the newly completed independent player decisions. The projection is a fresh descendant report with a distinct hash; all original proof bytes and source phases remain pinned. No previous admission payload or verdict is rewritten.'
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,decisions=q['decisions'])),flush=True)
 return 0
if __name__=='__main__':raise SystemExit(main())
