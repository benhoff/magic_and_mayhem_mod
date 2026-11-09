#!/usr/bin/env python3
"""Recheck a finite input/media gate cohort and admit exactly its two cells."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from input_media_gate import BID, SID, GATE, PREFIX, analyze, close_rows
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent', required=True)
    p.add_argument('--report', required=True)
    p.add_argument('--controls', required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(), 'Fresh owned gate report required')
    result = dict(schema=1, success=False, original_game_runs=0)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            b = next(v for v in reg['behaviors'] if v['id'] == BID)
            s = next(v for v in reg['scenarios'] if v['id'] == SID)
            sources = {n: reader.digest(n) for n in sorted(required_sources(b))}
            controls = reader.json(a.controls)
            require(controls['success'] is True and controls['tests'] == 38
                    and all(reader.digest(n) == h for n, h in controls['sources'].items()),
                    'Current actual-data gate controls are required')
            parent = reader.json(a.parent)
            proof = analyze(reader, parent)
            proof['sources'] = sources
            proof['source_snapshots'] = []
            for name in ('tools/input_media_gate.py', 'tools/check-rendering-input-media-gates.py', 'tests/test-input-media-gates.py'):
                raw = (ROOT/name).read_bytes()
                record = a.report.removesuffix('.json') + '-source-' + Path(name).name + '.bin'
                proof['source_snapshots'].append(dict(path=name, record=record, sha256=write_report(ROOT, record, raw)))
            proof['sources_stable'] = all(reader.digest(n) == h for n, h in sources.items())
            require(proof['sources_stable'], 'Gate analyzer changed during execution')
            proof_name = a.report.removesuffix('.json') + '-proof.json'
            proof_ref = dict(record=proof_name, sha256=write_report(ROOT, proof_name, (json.dumps(proof, indent=2)+'\n').encode()))
            result = dict(parent)
            result.update(schema=1, kind='input_media_gate_admission', success=True, task_complete=True,
                sources=sources, analysis_sources=sources, sources_stable=True,
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(b, {v['id']:v for v in reg['builds']}),
                             scenarios={SID:scenario_contract(s)})],
                parent=dict(record=a.parent, sha256=reader.digest(a.parent)),
                controls=dict(record=a.controls, sha256=reader.digest(a.controls)), proof=proof_ref,
                requirements=close_rows(parent, proof_ref, proof),
                passed_requirements=parent['passed_requirements']+2,
                pending_requirements=parent['pending_requirements']-2,
                newly_passed_requirements=2, closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {GATE}),
                original_game_runs=0, new_pixel_comparisons=0, current_whole_player_proved=False,
                historical_failures_retired=False, scope=proof['scope'])
    except Exception as error:
        result.update(success=False, error=str(error))
    digest = write_report(ROOT, a.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=a.report,
                         sha256=digest, passed=result.get('passed_requirements'), pending=result.get('pending_requirements'))), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
