#!/usr/bin/env python3
"""Admit only the two journeys gate cells from a current retained-data join."""
import argparse
import json
import platform
import sqlite3
import subprocess
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from journeys_gate import BID, SID, GATE, PREFIX, analyze, close_rows, local_python_sources
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('parent','controls','report'): p.add_argument('--'+name,required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(), 'Fresh owned report required')
    result = dict(schema=1,success=False,original_game_runs=0)
    try:
        with Reader(ROOT,memoize=True) as r:
            reg = r.json('research/runtime/coverage/register.json')
            b = next(v for v in reg['behaviors'] if v['id'] == BID)
            s = next(v for v in reg['scenarios'] if v['id'] == SID)
            paths = required_sources(b)
            require(local_python_sources() <= paths, 'Unpinned analyzer dependency')
            sources = {n:r.digest(n) for n in sorted(paths)}
            claims = [dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})]
            controls = r.json(a.controls)
            require(controls['success'] is True and controls['tests'] >= 24
                    and controls['failures'] == controls['errors'] == controls['skipped'] == 0
                    and controls['sources_stable'] is True and controls['sources'] == sources
                    and controls['claims'] == claims and controls['development_fixture_overlay'] is False,
                    'Current complete actual-data controls required')
            require(r.digest(a.parent) == r.digest(PREFIX+'basis-01.json'), 'Unreviewed acceptance parent')
            snapshots = []
            for n in ('tools/journeys_gate.py','tools/check-rendering-journeys-gates.py','tests/test-journeys-gates.py'):
                name = a.report.removesuffix('.json')+'-source-'+Path(n).name+'.bin'
                snapshots.append(dict(path=n,record=name,sha256=write_report(ROOT,name,(ROOT/n).read_bytes())))
            parent = r.json(a.parent); proof = analyze(r,parent)
            require(all(r.digest(n) == h for n,h in sources.items()), 'Analyzer sources changed')
            proof.update(sources=sources,sources_stable=True,source_snapshots=snapshots)
            name = a.report.removesuffix('.json')+'-proof.json'
            proof_ref = dict(record=name,sha256=write_report(ROOT,name,(json.dumps(proof,indent=2)+'\n').encode()))
            rows = close_rows(parent,proof_ref,proof)
            pending = sorted({v['id'] for v in rows if v['state'] == 'pending'})
            result = dict(parent)
            result.update(schema=1,kind='journeys_gate_admission',success=True,task_complete=True,
                sources=sources,analysis_sources=sources,sources_stable=True,source_snapshots=snapshots,
                claims=claims,parent=dict(record=a.parent,sha256=r.digest(a.parent)),
                controls=dict(record=a.controls,sha256=r.digest(a.controls)),proof=proof_ref,
                analysis_source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                python=platform.python_version(),sqlite=sqlite3.sqlite_version,requirements=rows,
                journey_cells=94,journeys=14,distinct_primary_sessions=proof['distinct_primary_sessions'],
                passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,
                newly_passed_requirements=2,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{GATE}),
                pending_requirement_ids=pending,remaining_distinct_tasks=len(pending),
                original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,
                current_whole_player_proved=False,historical_failures_retired=False,scope=proof['scope'])
    except Exception as e: result.update(success=False,error=str(e))
    sha = write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha,
                         passed=result.get('passed_requirements'),pending=result.get('pending_requirements'))),flush=True)
    return int(not result['success'])


if __name__ == '__main__': raise SystemExit(main())
