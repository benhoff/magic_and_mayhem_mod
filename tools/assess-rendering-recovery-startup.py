#!/usr/bin/env python3
"""Retain failed Low recovery admission without granting acceptance credit."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_menu_acceptance import artifact
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-transition-recovery-results-20261107/'
BID = 'NP.transition-recovery-acceptance'
SID = 'native.transition-recovery-acceptance'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT / args.report).exists(), 'Fresh owned report required')
    result = dict(schema=1, kind='rendering_recovery_startup_negative_review', success=False,
                  newly_passed_requirements=0, acceptance_closed=False, collection_stopped=True,
                  current_native_engine_validation=False, new_pixel_comparison=False)
    try:
        with Reader(ROOT) as reader:
            reg = reader.json('research/runtime/coverage/register.json')
            behavior = next(v for v in reg['behaviors'] if v['id'] == BID)
            scenario = next(v for v in reg['scenarios'] if v['id'] == SID)
            sources = {n: reader.digest(n) for n in sorted(required_sources(behavior))}
            inputs = []
            acquisitions = []
            for name in ('closeout08-default640-01', 'closeout08-default640-02'):
                record = PREFIX + name + '-acquisition.json'
                acquired = reader.json(record)
                require(not acquired['success'] and acquired['sources_stable'] and acquired['player_binary_stable']
                        and acquired['original_manifest_verified_before_after'], 'Changed/incomplete negative acquisition')
                acquisitions.append(acquired)
                inputs.append(dict(record=record, sha256=reader.digest(record), success=False,
                                   original_game_runs=acquired['original_game_runs'], error=acquired['error']))
            setup, failed = acquisitions
            require(setup['original_game_runs'] == 0 and not setup.get('observed_session_roots')
                    and 'SIGABRT' in setup['error'], 'Expected zero-game accessibility failure')
            require(failed['original_game_runs'] == 1 and failed['error'] == 'Initial Main readiness deadline'
                    and not failed['states'] and not failed['epochs_observed'] and not failed['pause_actions']
                    and failed['configuration']['actual_size'] == [640, 480]
                    and failed['configuration']['accessibility_bus'] == 'private-dbus-daemon',
                    'Wrong stopped Low execution')
            require(len(failed['observed_session_roots']) == 1, 'Ambiguous failed generation')
            session = Path(failed['observed_session_roots'][0])
            def raw(name):
                return artifact(reader, failed, str((session / name).relative_to(ROOT)))
            native = json.loads(raw('native-only-session.json'))
            resident = json.loads(raw('resident-world-report.json'))
            terminal = json.loads(raw('native-terminal-state.json'))
            events = raw('events.bin')
            frame_input = raw('frame-input.bin')
            exit_text = raw('original-process-exit.txt')
            require(native['session'] == str(session) and native['state'] == 'failed'
                    and 'GAP' in native['error'] and 'reason 2' in native['error']
                    and not native['normal_quit'] and not native['restart_requested'],
                    'Failure became nominal or different')
            require(int(resident['completed']) == 0 and resident['error'] == native['error']
                    and len(events) == len(frame_input) == 16
                    and struct.unpack_from('<II', events, 8)[1] > 0
                    and exit_text.strip() == b'0', 'Not the retained pre-menu zero-World exit')
            require(terminal['session'] == str(session) and terminal['error'] == native['error']
                    and all(v is True for v in terminal['state'].values()), 'Incomplete terminal owner observation')
            controls = []
            for test in ('test-rendering-transition-startup.py', 'test-rendering-transition-confirmation.py',
                         'test-rendering-transition.py'):
                execution = subprocess.run([sys.executable, str(ROOT / 'tests' / test), '-v'], cwd=ROOT,
                                           capture_output=True, check=True, timeout=60)
                record = args.report.removesuffix('.json') + '-' + test + '.log'
                controls.append(dict(test='tests/' + test, record=record,
                                     sha256=write_report(ROOT, record, execution.stdout + execution.stderr)))
            parent_name = 'research/runtime/rendering-clipped-fill-results-20261108/acceptance-01.json'
            parent = reader.json(parent_name)
            require(parent['success'] and parent['passed_requirements'] == 100
                    and parent['pending_requirements'] == 56, 'Accepted parent changed')
            result.update(success=True, sources=sources,
                sources_stable=all(reader.digest(n) == h for n, h in sources.items()),
                analysis_contract=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {v['id']: v for v in reg['builds']}),
                             scenarios={SID: scenario_contract(scenario)})],
                inputs=inputs, controls=controls, controls_passed=24, new_requests=2, new_original_game_runs=1,
                historical_original_game_runs=9, total_original_game_runs=10,
                original_exit_status=0, original_menu_callbacks=0, completed_worlds=0,
                native_error=native['error'], terminal_observation=terminal,
                native_summary_flags={k: native[k] for k in ('fallback', 'renderer_retired', 'force_stop_verified', 'normal_quit')},
                acceptance_parent=dict(record=parent_name, sha256=reader.digest(parent_name)),
                passed_requirements=100, pending_requirements=56, decisions=parent['decisions'],
                historical_failures_retired=False,
                source_phases=dict(current_sources='Current controller/read-only assessment and unit controls only.',
                    failed_execution=dict(record=inputs[1]['record'], source_pointer='/sources',
                        tested_controller=dict(record=PREFIX+'closeout08-tested-startup-source.py.bin',
                                               sha256=reader.digest(PREFIX+'closeout08-tested-startup-source.py.bin'))),
                    native_host='The retained player and its 406 compiler inputs remain unchanged.',
                    guest='The failed Low case uses the 1021-path committed guest phase. High cases retain their historical 1019-path guest phase; no current combined-guest success is claimed.'),
                limitation='No recovery interval, declared INVALID injection, fresh-generation admission, normal Quit or output comparison was reached in the new original run. Actual original exit0 precedes every menu callback; terminal GAP is incomplete-history retirement, not a proven raster defect. Its upstream launch cause remains unresolved. The raw final native summary retains fallback=true and renderer_retired/force_stop_verified=false while the earlier terminal observation has all nine retirement predicates true. Neither report is rewritten or admitted as successful cleanup. The private dbus-daemon preflight repair passes; the additional prompt terminal-read control has unit execution only. Both640 domains, all four requested operation rows and both final decisions remain pending.')
    except Exception as error:
        result.update(success=False, error=str(error))
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), sha256=digest,
                          newly_passed_requirements=0)), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
