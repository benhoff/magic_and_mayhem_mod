#!/usr/bin/env python3
"""Review two map-transition obligations without creating capture copies."""
import argparse
import json
from pathlib import Path

from campaign_transition_review import (BID, SID, OUT, PARENT, CASES, SCOPE,
    REPORT_LIMIT, COMPRESSED_LIMIT, DECODED_LIMIT, review_case, coverage_frontier, read_reference)
from coverage_claims import behavior_contract, scenario_contract, required_sources
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(OUT) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned immutable report required')
    result = dict(schema=1, kind='compact_retained_campaign_transition_review', success=False,
                  original_game_runs=0, new_pixel_comparisons=0, newly_passed_requirements=0,
                  current_native_engine_execution=False, capture_copies_created=0,
                  expanded_research_reports_created=0, scope=SCOPE)
    try:
        with Reader(ROOT) as reader:
            register = reader.json('research/runtime/coverage/register.json')
            behavior = next(b for b in register['behaviors'] if b['id'] == BID)
            scenario = next(s for s in register['scenarios'] if s['id'] == SID)
            sources = {p: reader.digest(p) for p in sorted(required_sources(behavior))}
            result['analysis_sources'] = sources
            parent_ref = dict(record=PARENT, sha256=reader.digest(PARENT))
            parent = read_reference(reader, parent_ref)
            require(parent['success'] and parent['passed_requirements'] == 112
                    and parent['pending_requirements'] == 44, 'Acceptance parent changed')
            cases = []
            result['cases'] = cases
            for acquisition, comparison in CASES:
                case = review_case(reader, acquisition, comparison)
                cases.append(case)
                print(json.dumps(dict(configuration=case['configuration'], extent=case['extent'],
                                      nominal_candidate=case['nominal_candidate'], verified=True)), flush=True)
            require(len({c['identity'] for c in cases}) == len(cases), 'Duplicate sessions')
            require(all(reader.digest(p) == h for p, h in sources.items()), 'Mixed analysis source phase')
            result.update(success=True, sources_stable=True, analysis_sources=sources,
                parent=parent_ref, passed_requirements=parent['passed_requirements'],
                pending_requirements=parent['pending_requirements'], decisions=parent['decisions'],
                frontier=coverage_frontier(cases),
                claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior,
                                 {b['id']: b for b in register['builds']}),
                             scenarios={SID: scenario_contract(scenario)})],
                storage=dict(report_limit_bytes=REPORT_LIMIT, compressed_read_limit_per_case=COMPRESSED_LIMIT,
                             decoded_ram_limit_per_case=DECODED_LIMIT,
                             existing_input_bytes_read=sum(c['compressed_bytes_read'] for c in cases),
                             retained_capture_bytes_added=0, retained_installations_added=0,
                             retained_prefixes_added=0,
                             policy='Reuse hash-pinned SQLite objects; references and compact witnesses only. '
                                    'RAM decoding is temporary. Preserve all historical input objects and failures.'))
    except Exception as exc:
        result['error'] = str(exc)
    raw = (json.dumps(result, indent=2)+'\n').encode()
    require(len(raw) <= REPORT_LIMIT, 'Compact report budget exceeded')
    digest = write_report(ROOT, args.report, raw)
    print(json.dumps(dict(success=result['success'], error=result.get('error'), report=args.report,
                          sha256=digest, report_bytes=len(raw), newly_passed_requirements=0)), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
