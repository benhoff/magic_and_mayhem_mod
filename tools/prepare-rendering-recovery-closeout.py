#!/usr/bin/env python3
"""Freeze two Low recovery requests after a no-game startup control repair."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from coverage_claims import required_sources
from coverage_evidence import Reader, write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-transition-recovery-results-20261107/'


def main():
    with Reader(ROOT) as reader:
        old_name = PREFIX + 'input-plan-09.json'
        plan = reader.json(old_name)
        require(all(reader.digest(n) == h for n, h in plan['compiler_sources'].items()),
                'Historical tested host compiler phase changed')
        runtime = {n: reader.digest(n) for n in sorted(native_runtime_source_paths(ROOT))}
        changed = {n: dict(before=h, after=runtime.get(n))
                   for n, h in plan['runtime_sources'].items() if runtime.get(n) != h}
        added = sorted(set(runtime) - set(plan['runtime_sources']))
        require(set(changed) == {'runtime/menu/campaign_mini_observe.h',
                                'runtime/menu/drawing_frame_input.h',
                                'runtime/menu/observer.c', 'runtime/render/input_polling.h'}
                and added == ['runtime/menu/hud_acceptance.h', 'runtime/render/ui_key_observation.h']
                and not set(plan['runtime_sources']) - set(runtime),
                'Unexpected guest phase delta')
        require(all(hashlib.sha256(subprocess.check_output(['git', 'show', 'HEAD:' + n], cwd=ROOT)).hexdigest() == h
                    for n, h in runtime.items()), 'Uncommitted guest/runtime changes')
        for binary in [plan['player_binary'], plan['fault_binary'], *plan['oracle']['binaries']]:
            require(hashlib.sha256((ROOT / binary['path']).read_bytes()).hexdigest() == binary['sha256'],
                    'Tested binary differs')
        partial_name = PREFIX + 'transition-partial-01.json'
        partial = reader.json(partial_name)
        require(partial['success'] and partial['collection_stopped'] and partial['original_game_runs'] == 9
                and len(partial['attempts']) == 11 and not partial['acceptance_closed'],
                'Historical stopped attempts must remain visible')
        failures = []
        for name in ('default640-02', 'default640-04'):
            record = PREFIX + name + '-acquisition.json'
            failed = reader.json(record)
            require(not failed['success'] and failed['original_game_runs'] == 1
                    and not failed['states'] and not failed['epochs_observed']
                    and 'Did not receive a reply' in failed['error'],
                    'Startup control failure differs')
            failures.append(dict(record=record, sha256=reader.digest(record)))
        controls = []
        for name in ('test-rendering-transition-startup.py', 'test-rendering-transition-confirmation.py'):
            completed = subprocess.run([sys.executable, str(ROOT / 'tests' / name), '-v'],
                                       cwd=ROOT, capture_output=True, check=True)
            raw = completed.stdout + completed.stderr
            record = PREFIX + 'closeout08-' + name.removesuffix('.py') + '.log'
            controls.append(dict(record=record, sha256=write_report(ROOT, record, raw)))
        failed_name = PREFIX + 'closeout08-default640-01-acquisition.json'
        failed = reader.json(failed_name)
        require(not failed['success'] and failed['original_game_runs'] == 0
                and 'SIGABRT' in failed['error'] and not failed.get('states'),
                'Expected zero-game accessibility setup failure')
        for index in range(3):
            env = dict(os.environ, ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
            env.pop('AT_SPI_BUS_ADDRESS', None)
            control = subprocess.run(['dbus-run-session', '--', sys.executable, '-c',
                "import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();assert Atspi.get_desktop(0) is not None;print('private dbus-daemon accessibility bus ready')"],
                cwd=ROOT, env=env, capture_output=True, check=True, timeout=20)
            record = PREFIX + 'closeout08-bus-control-%02d.log' % (index + 1)
            controls.append(dict(record=record, sha256=write_report(ROOT, record, control.stdout + control.stderr)))
        behavior = next(v for v in reader.json('research/runtime/coverage/register.json')['behaviors']
                        if v['id'] == 'NP.transition-recovery-acceptance')
        plan.update(previous_plan=dict(record=old_name, sha256=reader.digest(old_name)),
                    runtime_sources=runtime,
                    guest_phase_review=dict(changed=changed, added=added,
                        reason='Four later guest HUD/input observation changes and two added observation headers are present in committed source. Collect only the missing Low domains under this exact guest phase. Historical High domains keep their original runtime hashes and are not current combined-guest execution. No unchanged-native-source waiver is granted for guest changes.'),
                    sources={n: reader.digest(n) for n in sorted(required_sources(behavior))},
                    historical_stopped_budget=dict(record=partial_name, sha256=reader.digest(partial_name),
                        requests=11, original_game_runs=9, collection_stopped=True),
                    startup_control_review=dict(failures=failures, controls=controls,
                        reason='The added pre-admission read made synchronous AT-SPI RPCs to the Qt thread while Launch stages the original runner. The Low controller now polls only owned session admissions and stable original seqlocks during startup; existing child-exit and finite readiness deadlines remain. No-game controls verify pending Launch makes no accessibility RPC and rejects unpublished/unowned state. This repairs that controller hazard; it does not assert the original runner cannot fail to start.',
                        native_sources_unchanged=True, original_game_runs=0),
                    bounds=dict(maximum_requests=3, maximum_original_games=4,
                                primary_cases=2, primary_original_starts=4,
                                repair_cases=0, zero_game_preparations=1, stop_on_first_failure=True,
                                successful_pilot_comparison_before_expansion=True),
                    domains=[dict(software=v, size=[640, 480]) for v in (False, True)],
                    prior_requests=[dict(record=failed_name, sha256=reader.digest(failed_name))],
                    setup_repair=dict(failed_acquisition=dict(record=failed_name, sha256=reader.digest(failed_name)),
                        previous_plan=dict(record=PREFIX+'closeout08-input-plan-01.json', sha256=reader.digest(PREFIX+'closeout08-input-plan-01.json')),
                        reason='The host accessibility launcher selected dbus-broker --scope user, whose systemd user activation failed inside the private dbus-run-session. Set its supported ATSPI_DBUS_IMPLEMENTATION=dbus-daemon explicitly. Three disposable no-game private registry controls pass. Preserve the stopped zero-game request, explicitly add one setup-only request to the new plan, and keep the original-start cap at four; no runtime retry or High recollection is admitted.'),
                    new_native_capture_code=False,
                    owned_base=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                    repair_phase=dict(maximum_new_requests=2, maximum_new_original_starts=4,
                                      no_800_recollection=True, no_further_expansion=True))
        record = PREFIX + 'closeout08-input-plan-02.json'
        print(write_report(ROOT, record, (json.dumps(plan, indent=2) + '\n').encode()), flush=True)


if __name__ == '__main__':
    main()
