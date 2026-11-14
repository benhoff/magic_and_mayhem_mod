#!/usr/bin/env python3
"""Compare a retained current interaction capture using an explicit new plan."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
import coverage_evidence
from coverage_evidence import Reader, write_report
from broader_interaction_analysis_binding import bind_plan, select_plan_source, FIELDS
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]
SUPPORT = ('tools/broader_interaction_analysis_binding.py', 'tools/compare-broader-current-interaction.py')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('acquisition', 'analysis-plan', 'report'):
        parser.add_argument('--' + key, required=True)
    args = parser.parse_args()
    require(not (ROOT / args.report).exists(), 'Fresh immutable analysis result required')
    legacy_path = ROOT / 'tools/check-rendering-interaction.py'
    with Reader(ROOT) as reader:
        ref = lambda n: dict(record=n, sha256=reader.digest(n))
        acquired = reader.json(args.acquisition)
        selected = reader.json(args.analysis_plan)
        binding = selected['analysis_input_binding']
        phase_ref = acquired['broader_phase']
        require(binding['phase'] == phase_ref and reader.digest(phase_ref['record']) == phase_ref['sha256'],
                'Different pre-acquisition phase')
        phase = reader.json(phase_ref['record'])
        require(any(c['report'] == args.acquisition for c in phase['broader_cases']), 'Foreign acquisition')
        build_ref = phase['current_player_build']
        require(binding['build'] == build_ref and reader.digest(build_ref['record']) == build_ref['sha256'],
                'Different tested build')
        legacy_ref = binding['legacy_plan']
        require(reader.digest(legacy_ref['record']) == legacy_ref['sha256'], 'Historical plan changed')
        legacy = reader.json(legacy_ref['record'])
        binary_digest = lambda n: hashlib.sha256((ROOT / n).read_bytes()).hexdigest()
        derived = bind_plan(legacy, phase, acquired, reader.json(build_ref['record']), reader.digest, binary_digest)
        require({k: v for k, v in selected.items() if k != 'analysis_input_binding'} == derived,
                'Analysis plan changed beyond declared execution/oracle identities')
        require(binding['fields'] == list(FIELDS) and binding['analysis_only'] is True,
                'Unreviewed analysis substitution')
        controls_ref = binding['controls']
        require(reader.digest(controls_ref['record']) == controls_ref['sha256'], 'Control receipt changed')
        controls = reader.json(controls_ref['record'])
        require(controls['success'] and controls['tests'] >= 12
                and all(reader.digest(n) == h for n, h in controls['sources'].items()), 'Analysis controls stale')
        support = {n: reader.digest(n) for n in SUPPORT}
        require(binding['sources'] == support and controls['legacy_checker_sha256'] == reader.digest(legacy_path),
                'Analysis implementation changed')
        source = legacy_path.read_text()
        adapted = select_plan_source(source, args.analysis_plan)
        adapted_name = args.report.removesuffix('.json') + '-selected-analyser.bin'
        adapted_ref = dict(record=adapted_name, sha256=write_report(ROOT, adapted_name, adapted.encode()))
        analysis_ref = ref(args.analysis_plan)
        acquisition_ref = ref(args.acquisition)
    original_write = coverage_evidence.write_report
    original_argv = sys.argv

    def writer(root, path, body):
        if str(path) == args.report:
            result = json.loads(body)
            require(result['prospective_plan'] == analysis_ref and result['acquisition'] == acquisition_ref,
                    'Comparer consumed another analysis plan or capture')
            result['analysis_input_binding'] = dict(plan=analysis_ref, legacy_plan=legacy_ref,
                phase=phase_ref, build=build_ref, controls=controls_ref, selected_analyser=adapted_ref,
                changed_expression='main.plan_name only', changed_fields=list(FIELDS),
                historical_payloads_changed=False, source_waivers=False, new_original_game_runs=0,
                semantic_guards_changed=False)
            result['sources'].update(support)
            with Reader(ROOT) as reader:
                stable = all(reader.digest(n) == h for n, h in result['sources'].items())
            result['sources_stable'] &= stable
            result['success'] &= stable
            body = (json.dumps(result, indent=2) + '\n').encode()
        return original_write(root, path, body)

    coverage_evidence.write_report = writer
    try:
        namespace = dict(__name__='bound_interaction_analysis', __file__=str(legacy_path))
        exec(compile(adapted, str(legacy_path), 'exec'), namespace)
        sys.argv = [str(legacy_path), '--acquisition', args.acquisition, '--report', args.report]
        return namespace['main']()
    finally:
        coverage_evidence.write_report = original_write
        sys.argv = original_argv


if __name__ == '__main__':
    raise SystemExit(main())
