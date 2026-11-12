#!/usr/bin/env python3
"""Admit only the two bounded fault/lifetime gate cells from retained bytes."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from fault_lifetime_gate import BID, SID, GATE, PREFIX, analyze, close_rows
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent', required=True)
    p.add_argument('--report', required=True)
    p.add_argument('--controls', required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts
            and not (ROOT/a.report).exists(), 'Fresh owned gate report required')
    result = dict(schema=1, success=False, original_game_runs=0)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            b = next(v for v in reg['behaviors'] if v['id'] == BID)
            s = next(v for v in reg['scenarios'] if v['id'] == SID)
            sources = {n: reader.digest(n) for n in sorted(required_sources(b))}
            snapshots = []
            for name in ('tools/fault_lifetime_gate.py', 'tools/check-rendering-fault-lifetime-gates.py', 'tests/test-fault-lifetime-gates.py'):
                record = a.report.removesuffix('.json')+'-source-'+Path(name).name+'.bin'
                snapshots.append(dict(path=name, record=record, sha256=write_report(ROOT, record, (ROOT/name).read_bytes())))
            result['source_snapshots'] = snapshots
            controls = reader.json(a.controls)
            require(controls['success'] is True and controls['tests'] >= 30
                    and controls['failures'] == controls['errors'] == controls['skipped'] == 0
                    and controls['sources_stable'] is True and controls['development_fixture_overlay'] is False
                    and controls['sources'] == sources
                    and all(reader.digest(n) == h for n, h in controls['sources'].items()),
                    'Current complete actual-data gate controls required')
            parent = reader.json(a.parent)
            proof = analyze(reader, parent)
            proof.update(sources=sources, source_snapshots=snapshots)
            require(all(reader.digest(n) == h for n, h in sources.items()), 'Gate analyzer changed during execution')
            proof['sources_stable'] = True
            proof_name = a.report.removesuffix('.json')+'-proof.json'
            proof_ref = dict(record=proof_name, sha256=write_report(ROOT, proof_name, (json.dumps(proof, indent=2)+'\n').encode()))
            rows = close_rows(parent, proof_ref, proof)
            pending_ids = sorted({v['id'] for v in rows if v['state'] == 'pending'})
            result = dict(parent)
            result.update(schema=1, kind='fault_lifetime_gate_admission', success=True, task_complete=True,
                sources=sources, analysis_sources=sources, sources_stable=True, source_snapshots=snapshots,
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(b, {v['id']:v for v in reg['builds']}),
                             scenarios={SID:scenario_contract(s)})],
                parent=dict(record=a.parent, sha256=reader.digest(a.parent)),
                controls=dict(record=a.controls, sha256=reader.digest(a.controls)), proof=proof_ref,
                requirements=rows, remaining_distinct_tasks=len(pending_ids), pending_requirement_ids=pending_ids,
                passed_requirements=parent['passed_requirements']+2,
                pending_requirements=parent['pending_requirements']-2,
                newly_passed_requirements=2, closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {GATE}),
                original_game_runs=0, new_pixel_comparisons=0, current_whole_player_proved=False,
                physical_hardware_reset=False, historical_failures_retired=False, scope=proof['scope'])
    except Exception as error:
        result.update(success=False, error=str(error))
    digest = write_report(ROOT, a.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=a.report,
                         sha256=digest, passed=result.get('passed_requirements'), pending=result.get('pending_requirements'))), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
