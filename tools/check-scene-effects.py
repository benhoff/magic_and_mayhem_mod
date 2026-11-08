#!/usr/bin/env python3
"""Verify retained visibility transitions and keep the four semantic rows open."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from scene_effects import ROOT, PREFIX, BID, SID, analyze

DEPENDENCIES = ('tools/scene_effects.py', 'tools/check-scene-effects.py',
                'tests/test-scene-effects.py', 'tools/close-rendering-animation-auxiliary.py',
                'tools/close-rendering-first-world.py', 'tools/close-rendering-blend-direct.py',
                'tools/close-rendering-destination-history.py', 'tools/coverage_evidence.py',
                'tools/coverage_claims.py', 'tools/rendering_closeout.py',
                'tools/rendering_acceptance.py', 'tools/rendering_menu_acceptance.py',
                'tools/rendering_pilot.py', 'tools/rendering_capture.py',
                'tools/inspect-canvas-producers.py', 'tools/resident_capture.py',
                'docs/native-rendering-closure.json')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    path = Path(args.report)
    if not args.report.startswith(PREFIX) or '..' in path.parts or path.suffix != '.json' or (ROOT/path).exists():
        parser.error('New owned immutable report required')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        behaviors = [b for b in register['behaviors'] if b['id'] == BID]
        names = set(DEPENDENCIES)
        if behaviors:
            names.update(required_sources(behaviors[0]))
        sources = {n: reader.digest(n) for n in sorted(names)}
        try:
            result = analyze(reader)
            if any(reader.digest(n) != h for n, h in sources.items()):
                raise ValueError('Scene analysis source changed during execution')
            result.update(analysis_sources=sources, analysis_sources_stable=True)
            if behaviors:
                scenario = next(s for s in register['scenarios'] if s['id'] == SID)
                result['claims'] = [dict(behavior=BID, contract_sha256=behavior_contract(
                    behaviors[0], {v['id']: v for v in register['builds']}), scenarios={SID: scenario_contract(scenario)})]
        except Exception as error:
            result = dict(schema=1, kind='retained_scene_occlusion_output', success=False, error=str(error),
                          analysis_sources=sources, original_game_runs=0, newly_passed_requirements=0,
                          scope='Failed retained scene admission. No partial row promotion or historical hash refresh.')
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(record=args.report, sha256=digest, **{k: result.get(k) for k in
                     ('success', 'error', 'passed_requirements', 'pending_requirements', 'changed_world_points',
                      'matched_viewport_pixels', 'remaining_task_rows')})), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
