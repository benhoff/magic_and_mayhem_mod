#!/usr/bin/env python3
"""Assess stretch/overlap gaps from the retained eight-case COPY census."""
import argparse
from collections import Counter
import json
from pathlib import Path
import tempfile

from coverage_evidence import Reader, write_report
from native_copy_routes import decode, validate
from rendering_copy_readiness import summarize
from rendering_menu_acceptance import artifact
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
CANDIDATES = 'research/runtime/rendering-copy-results-20261106/candidate-census-01.json'
STATIC = 'research/runtime/native-copy-route-static-20261017.json'
PARENT = 'research/runtime/rendering-recovery-four-results-20261108/master-admission-01.json'
PREFIX = 'research/runtime/rendering-two-copy-results-20261108/'
BID = 'NP.copy-required-branch-readiness'


def assess(reader):
    inventory = reader.json(CANDIDATES)
    require(inventory['success'] and len(inventory['executions']) == 8, 'Changed eight-case candidate inventory')
    cases = []
    for candidate in inventory['executions']:
        pair = candidate['pair']
        for binding in pair.values():
            require(reader.digest(binding['record']) == binding['sha256'], 'Changed acquisition/comparison')
        acquired = reader.json(pair['acquisition']['record'])
        with tempfile.TemporaryDirectory(prefix='copy-readiness-', dir='/tmp') as directory:
            experiment = Path(directory)
            capture = experiment/'lock-capture'
            capture.mkdir()
            for name in ('copy-routes-v2.bin', 'gpu-copy-census-v1.bin'):
                (capture/name).write_bytes(artifact(reader, acquired, name))
            validated = validate(experiment)
            header, rows = decode(capture/'copy-routes-v2.bin')
        require(header[2] == 2 and validated['verified'], 'Incomplete version2 original route census')
        cases.append(dict(domain=candidate['domain'], scenario=candidate['scenario'],
                          pair=pair, route_validation=validated, **summarize(rows)))
    counts = Counter()
    for case in cases:
        counts.update(case['counts'])
    static = reader.json(STATIC)
    explicit = next(value for value in static['selected_wrappers'].values() if value['entry'] == 0x58cbc0)
    require(static['success'] and len(explicit['callers']) == 11, 'Changed original explicit-RECT caller inventory')
    parent = reader.json(PARENT)
    targets = [row for row in parent['requirements'] if row['id'] in
               ('operation.copy-stretched', 'operation.copy-alias-overlap')]
    require(len(targets) == 4 and all(row['state'] == 'pending' for row in targets), 'Changed target row states')
    require({(c['domain'][0], tuple(c['domain'][1])) for c in cases} ==
            {(p, s) for p in ('player-default', 'player-software') for s in ((640, 480), (800, 600))},
            'Missing retained profile/actual-size domain')
    return dict(schema=1, kind='retained_copy_required_branch_assessment', success=True,
                original_game_runs=0, newly_passed_requirements=0, acceptance_ready=False,
                passed_requirements=parent['passed_requirements'], pending_requirements=parent['pending_requirements'],
                parent=dict(record=PARENT, sha256=reader.digest(PARENT)), cases=cases,
                total_counts=dict(counts), target_rows=targets,
                inputs=[dict(record=name, sha256=reader.digest(name)) for name in (CANDIDATES, STATIC)],
                original_explicit_rectangle_callers=explicit['callers'],
                remaining_gaps=[
                    'Prepared geometry only: unprepared calls may contain default/partial rectangles; unknown geometry is retained, not classified as unscaled.',
                    'Tracked identity does not establish all physical/COM aliases. Zero aliases in these active intervals does not prove global unreachability.',
                    'The production bridge rejects stretched geometry and CPU self-copy before command publication; GPU planning also refuses stretch. CPU facade support does not prove production command output.',
                    'Eleven original explicit-RECT direct call candidates are static leads, not naturally executed stretched calls or complete indirect reachability.',
                    'Retained supplied original opaque/keyed overlap corpora establish their own Wine/backend comparisons, not required natural callers in both player profiles.',
                    'Close only after caller-bound actual arguments, authority/lifetime, result and complete independent destination evidence; preserve all accepted rows and both final decisions.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh owned report required')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == BID)
        names = {n for role in ('implementation', 'tests', 'validation_dependencies') for n in behavior[role]}
        before = {n: reader.digest(n) for n in sorted(names)}
        result = assess(reader)
        require(before == {n: reader.digest(n) for n in names}, 'Assessment sources changed')
        result.update(analysis_sources=before, sources_stable=True)
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, sha256=sha, counts=result['total_counts'],
                          newly_passed_requirements=0)))


if __name__ == '__main__':
    main()
