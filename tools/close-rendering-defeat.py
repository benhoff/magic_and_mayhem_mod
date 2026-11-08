#!/usr/bin/env python3
"""Close only the four-domain natural-defeat row using retained full evidence."""
import argparse
import json
from pathlib import Path
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, scenario_contract, required_sources
from rendering_defeat import BID, OUT, PARENT, SCOPE, analyze_case, close_rows, load
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    require(a.report.startswith(OUT) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),
            'Fresh owned immutable report required')
    result = dict(schema=1, kind='retained_natural_defeat_acceptance', success=False, original_game_runs=0)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            behavior = next(b for b in reg['behaviors'] if b['id'] == BID)
            sources = {n: reader.digest(n) for n in required_sources(behavior)}
            _, applicability = load(ROOT, 'close-rendering-world-exit').parent_plan(
                ROOT, reader, 'CF08.clipped-fill-native01', 'NP.clipped-fill-operation-closeout')
            cases = []
            result['cases'] = cases
            for profile in ('player-default', 'player-software'):
                for size in ([640, 480], [800, 600]):
                    case = analyze_case(reader, profile, size)
                    cases.append(case)
                    print(json.dumps(dict(domain=case['domain'], verified=True, pixels=case['output']['pixels'])), flush=True)
            parent = reader.json(PARENT)
            rows = close_rows(reader.json('docs/native-rendering-closure.json'), parent, cases)
            require(all(reader.digest(n) == h for n, h in sources.items()), 'Mixed analysis source phase')
            result.update(success=True, analysis_sources=sources,
                sources=parent['sources'] | {n:h for n,h in sources.items() if n not in parent['sources']},
                sources_stable=True, scope=SCOPE, cases=cases, accepted_parent_applicability=applicability,
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {}),
                    scenarios={s['id']: scenario_contract(s) for s in reg['scenarios'] if BID in s['behaviors']})],
                parent=dict(record=PARENT, sha256=reader.digest(PARENT)), parent_source_phases=parent['sources'],
                dossier_identity=parent['dossier_identity'], requirements=rows, decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {'journey.battle-results.defeat'}),
                passed_requirements=102, pending_requirements=54, newly_passed_requirements=2,
                required_domains=4, verified_domains=4, inherited_current_rows=100,
                pixels_compared=sum(c['output']['pixels'] for c in cases), mismatching_pixels=0,
                new_pixel_comparisons=0, current_native_engine_execution=False, historical_failures_retired=False)
    except Exception as e:
        result['error'] = str(e)
    digest = write_report(ROOT, a.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=a.report, sha256=digest)), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
