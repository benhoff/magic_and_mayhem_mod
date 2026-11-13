#!/usr/bin/env python3
"""Use the unchanged case coordinator with one explicit current comparison dispatch."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
import coverage_evidence
from coverage_evidence import Reader, write_report
from broader_interaction_dispatch import dispatch_source
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]
P = 'research/runtime/broader-player-acceptance-results-20261110/'
CASES = {'bp40-world-interaction-default800-01', 'bp40-world-interaction-software640-01',
         'bp40-world-interaction-software800-01'}
SUPPORT = ('tools/run-broader-current-interaction-case.py', 'tools/compare-broader-current-interaction-complete.py',
           'tools/broader_interaction_dispatch.py')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('plan', 'case', 'analysis-plan', 'current-analysis-plan', 'chunk-controls', 'dispatch-controls'):
        parser.add_argument('--' + key, required=True)
    args = parser.parse_args()
    require(args.case in CASES, 'Only the three unrequested declared interaction contexts')
    with Reader(ROOT) as reader:
        ref = lambda n: dict(record=n, sha256=reader.digest(n))
        phase = reader.json(args.plan)
        current_plan = reader.json(args.current_analysis_plan)
        require(phase['family'] == 'interaction'
                and current_plan['analysis_input_binding']['phase'] == ref(args.plan), 'Different interaction phase')
        case = next(v for v in phase['broader_cases'] if v['id'] == args.case)
        controls = reader.json(args.dispatch_controls)
        require(controls['success'] and controls['tests'] >= 10
                and all(reader.digest(n) == h for n, h in controls['sources'].items()), 'Current dispatch controls required')
        sources = {n: reader.digest(n) for n in SUPPORT}
        legacy = ROOT / 'tools/run-broader-discovery-domain-case.py'
        require(controls['original_coordinator_sha256'] == reader.digest(legacy), 'Original coordinator changed')
        selected = dispatch_source(legacy.read_text())
        operator_name = P + args.case + '-selected-coordinator.bin'
        operator = dict(record=operator_name, sha256=write_report(ROOT, operator_name, selected.encode()))
        control_ref, current_ref, chunk_ref = ref(args.dispatch_controls), ref(args.current_analysis_plan), ref(args.chunk_controls)
    report = P + args.case + '-coordinator-01.json'
    original_write = coverage_evidence.write_report
    original_argv = sys.argv

    def comparison_command():
        return ['python3', 'tools/compare-broader-current-interaction-complete.py', '--acquisition', case['report'],
                '--analysis-plan', args.current_analysis_plan, '--chunk-controls', args.chunk_controls,
                '--report', case['comparison']]

    def writer(root, path, body):
        if str(path) == report:
            result = json.loads(body)
            result['sources'] = result.get('sources', {}) | sources
            result['current_analysis_dispatch'] = dict(plan=current_ref, source_chunk_controls=chunk_ref,
                controls=control_ref, selected_coordinator=operator,
                changed_expression='comparison command only', acquisition_controller_changed=False,
                capture_policy_changed=False, actions_or_deadlines_changed=False, automatic_retries=False)
            with Reader(ROOT) as reader:
                stable = all(reader.digest(n) == h for n, h in result['sources'].items())
            result['success'] &= stable
            if not stable: result['dispatch_source_error'] = 'Current coordinator source changed'
            body = (json.dumps(result, indent=2) + '\n').encode()
        return original_write(root, path, body)

    coverage_evidence.write_report = writer
    try:
        namespace = dict(__name__='bound_current_interaction_coordinator', __file__=str(legacy),
                         _bound_comparison_command=comparison_command)
        exec(compile(selected, str(legacy), 'exec'), namespace)
        sys.argv = [str(legacy), '--plan', args.plan, '--case', args.case, '--analysis-plan', args.analysis_plan]
        return namespace['main']()
    finally:
        coverage_evidence.write_report = original_write
        sys.argv = original_argv


if __name__ == '__main__': raise SystemExit(main())
