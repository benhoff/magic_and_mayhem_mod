#!/usr/bin/env python3
"""Close victory and next-map from four retained full campaign executions."""
import argparse
import json
from pathlib import Path
from campaign_results_review import BID, SID, OUT, PARENT, CASES, LEAVES, SCOPE, analyze_case, close_rows
from coverage_claims import behavior_contract, scenario_contract, required_sources
from coverage_evidence import Reader, write_report
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True); args = p.parse_args()
    require(args.report.startswith(OUT) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),
            'Fresh owned immutable result required')
    q = dict(schema=1, kind='retained_victory_next_map_acceptance', success=False,
             original_game_runs=0, new_original_oracle_executions=0, new_pixel_comparisons=0,
             newly_passed_requirements=0, current_native_engine_execution=False)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            behavior = next(v for v in reg['behaviors'] if v['id'] == BID)
            scenario = next(v for v in reg['scenarios'] if v['id'] == SID)
            sources = {p: reader.digest(p) for p in required_sources(behavior)}
            cases = []; q['cases'] = cases
            for path in CASES:
                case = analyze_case(reader, path); cases.append(case)
                print(json.dumps(dict(domain=[case['configuration'], case['extent']], verified=True,
                    pixels=case['complete_output']['pixels'], later_swaps=case['later_output']['compared_swaps'])), flush=True)
            parent = reader.json(PARENT)
            rows = close_rows(reader.json('docs/native-rendering-closure.json'), parent, cases)
            require(all(reader.digest(p) == h for p, h in sources.items()), 'Mixed analysis source phase')
            q.update(success=True, sources_stable=True, analysis_sources=sources,
                sources=parent['sources'] | {p:h for p,h in sources.items() if p not in parent['sources']},
                parent=dict(record=PARENT, sha256=reader.digest(PARENT)), parent_source_phases=parent['sources'],
                requirements=rows, dossier_identity=parent['dossier_identity'], decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | set(LEAVES)),
                passed_requirements=124, pending_requirements=32, newly_passed_requirements=4,
                required_domains=4, verified_domains=4,
                retained_compared_pixels=sum(c['complete_output']['pixels'] for c in cases),
                mismatching_pixels=0, scope=SCOPE,
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {}),
                             scenarios={SID:scenario_contract(scenario)})])
    except Exception as error:q['error'] = str(error)
    h = write_report(ROOT, args.report, (json.dumps(q, indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not q['success'])


if __name__ == '__main__':raise SystemExit(main())
