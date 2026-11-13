#!/usr/bin/env python3
"""Freeze current loss/restart phases under the additional broader-task budget."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess

from coverage_claims import required_sources
from coverage_evidence import Reader, write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_surface_loss import linked_sources
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/broader-player-acceptance-results-20261110/'
SURFACE = 'research/runtime/surface-loss-restore-results-20261109/'
TRANSITION = 'research/runtime/rendering-transition-recovery-results-20261107/'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kind', choices=('loss', 'transition'), required=True)
    parser.add_argument('--report', required=True)
    parser.add_argument('--global-plan', required=True)
    parser.add_argument('--controls', required=True)
    parser.add_argument('--phase-key', default='bp10')
    parser.add_argument('--provider-adapter', action='store_true')
    parser.add_argument('--capture-ram', action='store_true')
    parser.add_argument('--supersedes')
    args = parser.parse_args()
    require(args.phase_key.isalnum() and args.phase_key.startswith('bp'), 'Safe owned phase key required')
    prefix = SURFACE if args.kind == 'loss' else TRANSITION
    require(args.report.startswith(prefix) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned phase plan required')
    with Reader(ROOT) as reader:
        global_plan = reader.json(args.global_plan)
        controls = reader.json(args.controls)
        require(global_plan['success'] is True and controls['success'] is True
                and controls['sources_stable'] is True
                and all(reader.digest(n) == h for n, h in controls['sources'].items()),
                'Fresh native controls required before any original start')
        built = reader.json('research/runtime/broader-player-acceptance-results-20261110/build-current-04.json')
        build = (ROOT/'working/build/qt-shell').resolve()
        compiler = linked_sources(build, ROOT, 'mnm-qt-shell')
        oracle = linked_sources(build, ROOT, 'mnm-acceptance-world-history')
        dependencies = subprocess.check_output(['g++', '-std=c++17', '-m32', '-MM',
            'tests/acceptance-world-reference.cpp'], cwd=ROOT, text=True)
        for name in shlex.split(dependencies.replace('\\\n', ' ')):
            p = ROOT/name
            if p.is_file() and not name.endswith(':'):
                oracle.add(p.relative_to(ROOT).as_posix())
        runtime = native_runtime_source_paths(ROOT)
        reg = reader.json('research/runtime/coverage/register.json')
        bid = 'NP.surface-loss-restore-acceptance' if args.kind == 'loss' else 'NP.transition-recovery-acceptance'
        behavior = next(b for b in reg['behaviors'] if b['id'] == bid)
        names = required_sources(behavior) | {'tools/prepare-broader-worker-phase.py', 'tools/run-broader-player-case.py', 'tools/broader_player_requests.py','tools/run-broader-recovery.py','tools/broader_native_host.py','tools/broader_capture_storage.py','tools/broader_worker_source_budget.py','tools/run-broader-worker-source-case.py','tools/check-broader-worker-player-case.py','tools/check-broader-worker-hardware-case.py'}
        sources = {n: reader.digest(n) for n in sorted(names)}

        def binary(key, path):
            value = dict(built[key], path=path)
            require(hashlib.sha256((ROOT/path).read_bytes()).hexdigest() == value['sha256']
                    and reader.digest(value['record']) == value['sha256'],
                    'Exact tested player/oracle changed')
            return value

        cases = []
        reports = {}
        for software in (False, True):
            for low in (True, False):
                label = 'software' if software else 'default'
                size = [640, 480] if low else [800, 600]
                case_id = args.phase_key+'-'+args.kind+'-'+label+str(size[0])+'-01'
                report = prefix+case_id+'-acquisition.json'
                collector = ('tools/run-rendering-surface-loss.py' if args.kind == 'loss' else
                             'tools/run-rendering-recovery-closeout.py' if low else
                             'tools/run-rendering-transition.py')
                command = ['python3', collector, '--plan', args.report, '--report', report]
                if low:
                    command.append('--low')
                if software:
                    command.append('--software')
                provider = 'mesa' if software else 'nvidia'
                if args.provider_adapter:
                    command = ['python3','tools/run-broader-recovery.py','--kind',args.kind,'--provider',provider,'--plan',args.report,'--report',report]
                    if low: command.append('--low')
                    if software: command.append('--software')
                cases.append(dict(id=case_id, report=report, command=command,provider=provider if args.provider_adapter else 'mesa',
                    environment={'ATSPI_DBUS_IMPLEMENTATION': 'dbus-daemon'},
                    profile='player-software' if software else 'player-default',
                    size=size, reserved_original_games=2))
                reports[report] = dict(software=software, size=size)
        plan = dict(schema=1, kind='native_surface_loss_prospective_plan' if args.kind == 'loss'
                    else 'native_transition_recovery_prospective_plan', success=True, original_game_runs=0,
            capture_storage=dict(enabled=args.capture_ram,policy='owned-tmpfs-diagnostic-capture-exact-copy-v1'),
            supersedes_prospective_plan=dict(record=args.supersedes,sha256=reader.digest(args.supersedes))if args.supersedes else None,
            fault_policy='quiescent-native-offscreen-target-loss' if args.kind == 'loss' else 'drained-Mini-producer-invalid',
            compiler_sources={n: reader.digest(n) for n in sorted(compiler)},
            runtime_sources={n: reader.digest(n) for n in sorted(runtime)}, sources=sources,
            player_binary=binary('player', 'working/build/qt-shell/mnm-qt-shell'),
            oracle=dict(native_sources={n: reader.digest(n) for n in sorted(oracle)},
                binaries=[binary('world-history', 'working/build/qt-shell/mnm-acceptance-world-history'),
                          binary('original-reference', 'working/original-reference')]),
            current_controls=dict(record=args.controls, sha256=reader.digest(args.controls)),
            broader_plan=dict(record=args.global_plan, sha256=reader.digest(args.global_plan)),
            request_driver='tools/run-broader-worker-source-case.py',broader_cases=cases, bounds=dict(maximum_requests=4, maximum_original_games=8,
                primary_cases=4, primary_original_starts=8, maximum_repair_original_starts=0,
                stop_on_first_failure=True, successful_pilot_comparison_before_expansion=True,
                case_deadline_seconds=600),
            execution_paths=dict(schema=1, output_root='working/broader-player/'+args.phase_key+'/'+args.kind+'/acceptance',
                budget='working/broader-player/'+args.phase_key+'/'+args.kind+'/attempts.sqlite3', reports=reports),
            prior_requests=[], pilot=cases[0]['report'], original_pixels_used_as_native_inputs=False,
            newly_passed_requirements=0,
            scope='Fresh current-source actual public-player failed/fresh epochs at both sizes and profile flags. Declared native-target or producer fault only; no physical hardware/reset or arbitrary-provider equivalence, normal failed-epoch credit or old-generation continuation. The previous source phases and their capped budgets remain immutable.',
            failure_policy='Stop and retain every unknown charge on any acquisition/comparison/validation failure; repair needs a new exact review and prospective plan within the cumulative cap.')
        if args.kind == 'transition':
            target = ROOT/'working/broader-player/rendering-transition-fault'
            require(target.is_file(), 'Build and retain the exact transition fault injector before freezing its phase')
            raw = target.read_bytes()
            name = args.report.removesuffix('.json')+'-fault.bin'
            plan['fault_binary'] = dict(path=str(target.relative_to(ROOT)), record=name,
                sha256=write_report(ROOT, name, raw), bytes=len(raw))
        plan['source_preimages'] = []
        for n in sorted(names):
            previous = subprocess.run(['git', 'show', 'HEAD:'+n], cwd=ROOT, capture_output=True)
            if previous.returncode == 0 and hashlib.sha256(previous.stdout).hexdigest() == sources[n]:
                continue
            raw = (ROOT/n).read_bytes()
            name = args.report.removesuffix('.json')+'-source-'+hashlib.sha256(raw).hexdigest()+'.bin'
            plan['source_preimages'].append(dict(path=n, record=name,
                sha256=write_report(ROOT, name, raw)))
    sha = write_report(ROOT, args.report, (json.dumps(plan, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, record=args.report, sha256=sha, primary_cases=len(cases))), flush=True)


if __name__ == '__main__':
    raise SystemExit(main())
