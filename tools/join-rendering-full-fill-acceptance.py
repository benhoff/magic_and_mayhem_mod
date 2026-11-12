#!/usr/bin/env python3
"""Join unchanged retained full-fill analysis to a newer accepted row inventory."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('full_fill_operations', ROOT/'tools/close-rendering-full-fill.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def join(reader, analysis, parent, plan):
    require(analysis['success'] and analysis['analysis_sources_stable']
            and analysis['actual_cases'] == 4
            and analysis['full_fill_witnesses'] == 12
            and not analysis['current_native_engine_execution'], 'Incomplete Full-fill component analysis')
    require(all(reader.digest(n) == h for n, h in analysis['analysis_sources'].items()),
            'Full-fill component analysis source changed')
    for case in analysis['cases']:
        for ref in case['pair'].values():
            require(reader.digest(ref['record']) == ref['sha256'], 'Changed accepted case input')
        require(any(row['state'] == 'passed' and row['configuration'] == case['domain'][0]
                    and any(v.get('execution') == case['pair'] for v in row.get('admitted_execution_evidence', []))
                    for row in parent['requirements']), 'Full-fill case missing from accepted parent cohort')
    rows = m.close_rows(plan, parent, analysis['cases'])
    return dict(analysis, requirements=rows, decisions=parent['decisions'],
                scope='Only full-fill for default/software at actual640/800 in four already admitted storage/menu contexts while the same imported World surface and generation remain live through UPDATE and GPU COPY output. Exact natural arguments, caller/fence/live-handle identity, complete argument-derived UPDATE bytes and twelve independent recorded complete actual GPU plane witnesses. Preserve every other accepted row, historical native phase, failure, eight unpaired terminal fills and both decisions. No clipping, whole caller ABI, new game/native execution or fixture claim.',
                sources=parent['sources'] | {n: h for n, h in analysis['sources'].items() if n not in parent['sources']},
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {m.LEAF}),
                passed_requirements=parent['passed_requirements']+2,
                pending_requirements=154-parent['passed_requirements'],
                case_execution_repeated=False,
                component_analysis_sources=analysis['analysis_sources'])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('analysis', 'parent', 'report'): p.add_argument('--'+name, required=True)
    args = p.parse_args()
    require(args.report.startswith(m.PREFIX) and not (ROOT/args.report).exists(), 'New owned report required')
    q = dict(schema=1, success=False)
    try:
        with Reader(ROOT) as r:
            before = m.sources(r)
            q = join(r, r.json(args.analysis), r.json(args.parent), r.json('docs/native-rendering-closure.json'))
            require(before == m.sources(r), 'Join sources changed')
            q.update(kind='retained_full_fill_acceptance_join', analysis_sources=before,
                     component_analysis=dict(record=args.analysis, sha256=r.digest(args.analysis)),
                     parent=dict(record=args.parent, sha256=r.digest(args.parent)))
    except Exception as e: q = dict(schema=1, success=False, error=str(e))
    h = write_report(ROOT, args.report, (json.dumps(q, indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'], error=q.get('error'), sha256=h)))
    return int(not q['success'])


if __name__ == '__main__': raise SystemExit(main())
