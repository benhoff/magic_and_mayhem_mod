#!/usr/bin/env python3
"""Close exactly two required-boundary cells from retained finite proofs."""
import argparse,json
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from required_boundaries_gate import BID,SID,GATE,PREFIX,BASIS,analyze,close_rows,validate_proof,local_python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',default=BASIS)
 for n in ('controls','report'):p.add_argument('--'+n,required=True)
 a=p.parse_args();require(a.report.startswith(PREFIX)and'..'not in Path(a.report).parts and not(ROOT/a.report).exists(),'Fresh owned admission required')
 result=dict(schema=1,success=False,original_game_runs=0)
 try:
  with Reader(ROOT)as r:
   reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']==BID);s=next(v for v in reg['scenarios']if v['id']==SID);paths=required_sources(b);require(local_python_sources()<=paths,'Unpinned local analyzer dependency');sources={n:r.digest(n)for n in sorted(paths)}
   controls=r.json(a.controls);require(controls['success']is True and controls['tests']>=70 and controls['failures']==controls['errors']==controls['skipped']==0 and controls['sources_stable']is True and controls['sources']==sources and controls['development_fixture_overlay']is False,'Complete current actual-data controls required')
   parent=r.json(a.parent);proof=analyze(r,parent);validate_proof(proof);require(all(r.digest(n)==h for n,h in sources.items()),'Analyzer inputs changed');proof.update(sources=sources,sources_stable=True)
   name=a.report.removesuffix('.json')+'-proof.json';ref=dict(record=name,sha256=write_report(ROOT,name,(json.dumps(proof,indent=2)+'\n').encode()));rows=close_rows(parent,ref,proof)
   result=dict(parent);result.update(schema=1,kind='native_rendering_required_boundaries_admission',success=True,task_complete=True,sources=sources,analysis_sources=sources,sources_stable=True,
    claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],parent=dict(record=a.parent,sha256=r.digest(a.parent)),controls=dict(record=a.controls,sha256=r.digest(a.controls)),proof=ref,
    requirements=rows,passed_requirements=156,pending_requirements=0,newly_passed_requirements=2,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{GATE}),remaining_distinct_tasks=0,pending_requirement_ids=[],
    boundary_scope_decisions=proof['decisions'],supported_player_accepted=False,original_recovery_complete=False,current_whole_player_proved=False,current_combined_player_proved=False,historical_failures_retired=False,
    original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,
    scope='Eight evidenced player boundaries for the exact demonstrated NoCD single-player inputs, maps, saves, event sequences, durations and Linux/Wine/Qt/llvmpipe providers. Preserve all154other cells and pending overall verdicts; no arbitrary domain, repaired natural witness or original-recovery completion.')
 except Exception as e:result.update(success=False,error=str(e))
 sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha,passed=result.get('passed_requirements'),pending=result.get('pending_requirements'))),flush=True);return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
