#!/usr/bin/env python3
"""Admit the two retained visibility rows with fresh adverse and UI controls."""
import argparse
from copy import deepcopy
import json
import os
from pathlib import Path
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_pilot import require
from scene_effects import ROOT, PREFIX, BID, SID, checked, close_rows


def join(reader, analysis_name):
    analysis = reader.json(analysis_name)
    require(analysis.get('success') is True and analysis['analysis_sources_stable'] is True,
            'Failed or unstable visibility analysis')
    sources = {n: reader.digest(n) for n in analysis['analysis_sources']}
    changed = {n: dict(recorded=h, current=sources[n]) for n, h in analysis['analysis_sources'].items()
               if h != sources[n]}
    # Only the default retained positive-report selector changed after this
    # analysis. Controls execute afresh below; the analyzer/readers stay exact.
    require(set(changed) <= {'tests/test-scene-effects.py'}, 'Scene analyzer/source drift needs retained-data reanalysis')
    parent = checked(reader, analysis['parent'])
    corpus = checked(reader, analysis['corpus'])
    plan = reader.json('docs/native-rendering-closure.json')
    for case in analysis['cases']:
        acquired = checked(reader, case['pair']['acquisition'])
        compared = checked(reader, case['pair']['comparison'])
        require(acquired['success'] and compared['success'] and compared['acquisition'] == case['pair']['acquisition']
                and acquired['session_identity'] == case['identity'], 'Changed actual scene execution')
        originals = [c for c in corpus['executions'] if c['pair'] == case['pair']]
        require(len(originals) == 1 and all(case[k] == originals[0][k] for k in ('domain', 'identity', 'scenario', 'provider'))
                and acquired['configuration']['software_flag'] is (case['domain'][0] == 'player-software')
                and all(s['fields'][5:7] == case['domain'][1] for s in case['transition']['states']),
                'Relabelled scene profile, domain or primary admission')
    rows = close_rows(plan, parent, analysis['cases'])
    require(rows == analysis['requirements'] and analysis['decisions'] == parent['decisions'],
            'Visibility join changed recorded facts or decisions')
    register = reader.json('research/runtime/coverage/register.json')
    behavior = next(b for b in register['behaviors'] if b['id'] == BID)
    scenario = next(s for s in register['scenarios'] if s['id'] == SID)
    sources.update({n: reader.digest(n) for n in required_sources(behavior)})
    sources.update({n: reader.digest(n) for n in (
        'tests/test-coverage-ui.py', 'tests/test-coverage-ui-client.js',
        'tools/coverage_rendering_artifacts.py', 'tools/coverage_ui.py',
        'apps/coverage-ui/app.js', 'apps/coverage-ui/importance.json')})
    result = deepcopy(analysis)
    result.update(kind='retained_scene_occlusion_admission', analysis_sources=sources,
                  analysis=dict(record=analysis_name, sha256=reader.digest(analysis_name)),
                  source_phase_review=dict(changed=changed, current_controls_only=True,
                      reason='Only the adverse test file changed: its positive-report default, accepted refusal exception classes and actual UI projection check. Re-execute current controls; source/WORD/viewport analysis and every native execution hash remain exact.'),
                  claims=[dict(behavior=BID, contract_sha256=behavior_contract(
                      behavior, {v['id']: v for v in register['builds']}), scenarios={SID: scenario_contract(scenario)})])
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--analysis', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    path = Path(args.report)
    require(args.report.startswith(PREFIX) and '..' not in path.parts and path.suffix == '.json'
            and not (ROOT/path).exists(), 'New owned immutable admission required')
    with Reader(ROOT) as reader:
        controls = []
        result = {}
        try:
            result = join(reader, args.analysis)
            env = dict(os.environ, MNM_SCENE_EFFECTS_REPORT=args.analysis)
            for name in ('tests/test-scene-effects.py', 'tests/test-coverage-ui.py', 'tests/test-coverage-ui-client.js'):
                command = ['node', name] if name.endswith('.js') else [sys.executable, '-B', name]
                check = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=120)
                controls.append(dict(command=command, exit_code=check.returncode, stdout=check.stdout, stderr=check.stderr))
                require(check.returncode == 0, 'Scene/UI controls failed: '+check.stderr)
            require(all(reader.digest(n) == h for n, h in result['analysis_sources'].items()), 'Join sources changed during controls')
            result['controls'] = controls
        except Exception as error:
            result.update(schema=1, kind='retained_scene_occlusion_admission', success=False, error=str(error),
                          controls=controls, claims=[], original_game_runs=0, newly_passed_requirements=0,
                          scope='Failed visibility admission; preserve the previous analysis and all six task-row boundaries.')
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(record=args.report, sha256=digest, **{k: result.get(k) for k in
          ('success', 'error', 'passed_requirements', 'pending_requirements', 'remaining_task_rows')})), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
