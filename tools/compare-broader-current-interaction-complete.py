#!/usr/bin/env python3
"""Run current input binding and, only when needed, retained source-chunk semantics."""
import argparse
import copy
import json
from pathlib import Path
import subprocess
from coverage_evidence import Reader, write_report
from broader_interaction_dispatch import comparison_route
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('acquisition', 'analysis-plan', 'chunk-controls', 'report'):
        parser.add_argument('--' + key, required=True)
    args = parser.parse_args()
    require(args.report.startswith('research/runtime/rendering-interaction-results-20261107/')
            and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned comparison required')
    raw_name = args.report.removesuffix('.json') + '-input-bound.json'
    require(not (ROOT / raw_name).exists(), 'Never repeat a comparison automatically')
    command = ['python3', 'tools/compare-broader-current-interaction.py', '--acquisition', args.acquisition,
               '--analysis-plan', args.analysis_plan, '--report', raw_name]
    completed = subprocess.run(command, cwd=ROOT)
    require(completed.returncode in (0, 1) and (ROOT / raw_name).exists(), 'Current input comparison setup failed')
    with Reader(ROOT) as reader:
        parent = reader.json(raw_name)
        parent_ref = dict(record=raw_name, sha256=reader.digest(raw_name))
        require(parent['acquisition'] == dict(record=args.acquisition, sha256=reader.digest(args.acquisition)),
                'Foreign current comparison')
        route = comparison_route(parent)
        sources = {n: reader.digest(n) for n in ('tools/compare-broader-current-interaction-complete.py',
                                                'tools/broader_interaction_dispatch.py')}
    if route == 'semantics':
        return subprocess.run(['python3', 'tools/analyze-broader-retained-interaction.py',
            '--acquisition', args.acquisition, '--pixels', raw_name, '--controls', args.chunk_controls,
            '--report', args.report], cwd=ROOT).returncode
    result = copy.deepcopy(parent)
    result['current_comparison_dispatch'] = dict(raw_comparison=parent_ref, route=route,
        original_game_runs=0, new_public_player_executions=0, extra_pixel_comparisons=0,
        scope='The retained complete raw result is projected without changing its success, pixels or semantic proof. Pixel disagreements remain negative for the existing explicit policy-or-refusal path.')
    result['output_directory'] = str(ROOT / 'working/interaction-comparisons' / Path(raw_name).stem)
    result['sources'].update(sources)
    with Reader(ROOT) as reader:
        stable = all(reader.digest(n) == h for n, h in result['sources'].items())
    result['sources_stable'] &= stable
    result['success'] &= stable
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], route=route, sha256=digest)), flush=True)
    return int(not result['success'])


if __name__ == '__main__': raise SystemExit(main())
