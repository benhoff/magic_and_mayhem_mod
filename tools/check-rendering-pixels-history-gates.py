#!/usr/bin/env python3
"""Admit exactly the two retained native pixel/history profile cells."""
import argparse
import json
import platform
import sqlite3
import subprocess
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from pixels_history_gate import BID, SID, GATE, PREFIX, analyze, close_rows
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent', required=True); p.add_argument('--report', required=True)
    p.add_argument('--controls', required=True); a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(), 'Fresh owned report required')
    result = dict(schema=1, success=False, original_game_runs=0)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            b = next(v for v in reg['behaviors'] if v['id'] == BID)
            s = next(v for v in reg['scenarios'] if v['id'] == SID)
            sources = {n: reader.digest(n) for n in sorted(required_sources(b))}
            snapshots = []
            for name in ('tools/pixels_history_gate.py', 'tools/check-rendering-pixels-history-gates.py', 'tests/test-pixels-history-gates.py'):
                target = a.report.removesuffix('.json')+'-source-'+Path(name).name+'.bin'
                snapshots.append(dict(path=name, record=target, sha256=write_report(ROOT, target, (ROOT/name).read_bytes())))
            controls = reader.json(a.controls)
            require(controls['success'] is True and controls['tests'] >= 24
                    and controls['failures'] == controls['errors'] == controls['skipped'] == 0
                    and controls['sources_stable'] is True and controls['sources'] == sources
                    and controls['development_fixture_overlay'] is False, 'Current complete actual-data controls required')
            parent = reader.json(a.parent)
            require(reader.digest(a.parent)==reader.digest(PREFIX+'basis-01.json')
                    and parent==reader.json(PREFIX+'basis-01.json'), 'Unreviewed acceptance parent')
            expected_claims=[dict(behavior=BID, contract_sha256=behavior_contract(b, {v['id']:v for v in reg['builds']}), scenarios={SID:scenario_contract(s)})]
            require(controls['claims']==expected_claims,'Control execution scope differs')
            proof = analyze(reader, parent)
            require(all(reader.digest(n) == h for n, h in sources.items()), 'Analyzer sources changed')
            proof.update(sources=sources, sources_stable=True, source_snapshots=snapshots)
            target = a.report.removesuffix('.json')+'-proof.json'
            proof_ref = dict(record=target, sha256=write_report(ROOT, target, (json.dumps(proof, indent=2)+'\n').encode()))
            rows = close_rows(parent, proof_ref, proof)
            result = dict(parent)
            result.update(schema=1, kind='pixels_history_gate_admission', success=True, task_complete=True,
                sources=sources, analysis_sources=sources, sources_stable=True, source_snapshots=snapshots,
                analysis_source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                python=platform.python_version(), sqlite=sqlite3.sqlite_version,
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(b, {v['id']:v for v in reg['builds']}), scenarios={SID:scenario_contract(s)})],
                parent=dict(record=a.parent, sha256=reader.digest(a.parent)),
                controls=dict(record=a.controls, sha256=reader.digest(a.controls)), proof=proof_ref,
                requirements=rows, passed_requirements=parent['passed_requirements']+2,
                pending_requirements=parent['pending_requirements']-2, newly_passed_requirements=2,
                pending_requirement_ids=sorted({v['id'] for v in rows if v['state']=='pending'}),
                remaining_distinct_tasks=len({v['id'] for v in rows if v['state']=='pending'}),
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {GATE}),
                original_game_runs=0, new_pixel_comparisons=0, current_whole_player_proved=False,
                historical_failures_retired=False, scope=proof['scope'])
    except Exception as error: result.update(success=False, error=str(error))
    h = write_report(ROOT, a.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=a.report, sha256=h,
                         passed=result.get('passed_requirements'), pending=result.get('pending_requirements'))), flush=True)
    return int(not result['success'])


if __name__ == '__main__': raise SystemExit(main())
