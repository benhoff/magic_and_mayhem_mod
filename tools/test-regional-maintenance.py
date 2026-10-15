#!/usr/bin/env python3
"""Pinned original/native dynamic navigation, incremental saves and regressions."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SCOPE = ('Pinned540430 terrain signatures/mode selection,540140 priority ring,5404a0/'
         '540820/5416d0 incremental connection maintenance and53c070 local reconnection; '
         'original32/native32 paired installed32x32x23 ordinary-type17 journeys, '
         'native64/sanitizer continuation and native terrain-edit/CMD1v8 policies. '
         'Four helper fixtures cover boundary0/6, partitions, retained actor routes, '
         'interleaved regional searches, all64 signatures, queue merge/full/wrap and '
         'checkpoint/admission failures. Four journeys open/close the sole crossing, '
         'including rapid reversal; a declared move order at tick1536 tests use of '
         'the repaired graph after an earlier failed move. World danger, startup '
         'finalization, arbitrary species, full World/campaign and live replacement excluded.')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--install', type=Path, required=True)
    parser.add_argument('--build-root', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--ticks', type=int, default=2048)
    parser.add_argument('--seeds', type=int, nargs='+', default=[17, 91])
    args = parser.parse_args()
    if args.ticks < 2048 or args.ticks > 8192:
        parser.error('The complete journey requires2048..8192 ticks')
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / p) != v for p, v in declaration['sources'].items()):
        raise ValueError('Declared source changed before execution')
    report = dict(success=False, source_executable_sha256=PIN, scope=SCOPE,
                  source_sha256=declaration['sources'], claims=declaration['claims'],
                  commands=[], wine_required=False, live_replacement=False)
    sources, builds = {}, {}

    def run(name, command, expected=0, env=None):
        command = [str(v) for v in command]
        report['commands'].append(command)
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                timeout=900, env=env)
        (out / (name + '.stdout')).write_text(result.stdout)
        (out / (name + '.stderr')).write_text(result.stderr)
        if result.returncode != expected or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise RuntimeError(name + ' failed: ' + str(result.returncode) + '\n' + result.stdout[-1500:] + result.stderr[-3000:])
        return result.stdout

    env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1',
           'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}
    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != PIN:
            raise ValueError('Original executable hash differs')
        inputs = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani',
                  'Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd']
        report['installed_inputs'] = {p: sha(args.install / p) for p in inputs}
        dependencies = SourceFileLoader('maintenance_dependencies', str(ROOT / 'tools/test-installed-encounter.py')).load_module()
        for tag, options in [
            ('original32', ['-DCMAKE_CXX_FLAGS=-m32 -O1 -g', '-DCMAKE_EXE_LINKER_FLAGS=-m32 -no-pie', '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=ON']),
            ('native64', ['-DCMAKE_BUILD_TYPE=', '-DCMAKE_CXX_FLAGS=-O1 -g']),
            ('sanitized', ['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = args.build_root.resolve() / tag
            builds[tag] = build
            run(tag + '-configure', ['cmake', '-S', ROOT / 'apps/native-encounter', '-B', build,
                                     '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', *options])
            closure = dependencies.dependencies(build, run, tag, extra={
                'regional-navigation-test.cpp', 'regional-encounter-test.cpp',
                'regional-map-admission-test.cpp', 'regional-graph-test.cpp',
                'regional-maintenance-test.cpp', 'regional-maintenance-encounter-test.cpp'})
            closure.update({'tools/test-regional-maintenance.py', 'tools/export-regional-repair.py',
                            'tools/test-regional-graph.py', 'tools/export-regional-graph.py',
                            'tools/test-regional-navigation.py', 'tools/export-regional-navigation.py',
                            'tools/ghidra/ExportPersistenceProgression.java'})
            sources.update({p: sha(ROOT / p) for p in closure})
            if args.claims and closure - declaration['sources'].keys():
                raise ValueError('Undeclared build inputs: ' + str(sorted(closure - declaration['sources'].keys())))
            targets = ['regional-maintenance-test', 'regional-maintenance-encounter-test',
                       'regional-navigation-test', 'regional-encounter-test', 'regional-map-admission-test',
                       'regional-graph-test', 'regional-graph-encounter-test', 'native-encounter',
                       'native-command-session-test', 'native-command-behavior-session-test',
                       'native-command-action-session-test', 'native-command-action-bodies-session-test',
                       'native-creature-providers-test', 'native-encounter-test']
            run(tag + '-build', ['cmake', '--build', build, '--target', *targets, '-j2'])
        helpers = []
        for scenario in range(4):
            rows = [json.loads(run(tag + '-maintenance-' + str(scenario),
                                  [build / 'regional-maintenance-test', *([pe] if tag == 'original32' else []),
                                   args.install, scenario], env=env)) for tag, build in builds.items()]
            if not all(row == rows[0] for row in rows):
                raise ValueError('Maintenance helper architecture/sanitizer mismatch: ' + str(scenario))
            helpers.append(dict(scenario=scenario, **rows[0]))
        report['helpers'] = helpers
        report['helper_checks'] = sum(v['checks'] for v in helpers)
        run('static', [sys.executable, ROOT / 'tools/export-regional-repair.py', '--output', out / 'static'])
        journeys, restores = [], 0
        for scenario in range(4):
            for seed in args.seeds:
                name = f'scenario-{scenario}-seed-{seed}'
                directory = out / name
                row = json.loads(run(name, [builds['original32'] / 'regional-maintenance-encounter-test',
                                           pe, args.install, scenario, seed, args.ticks, directory], env=env))
                journeys.append(dict(scenario=scenario, seed=seed, **row))
                for tag in ['native64', 'sanitized']:
                    snapshot, trace = out / (tag + '-' + name + '.snapshot'), out / (tag + '-' + name + '.trace')
                    result = json.loads(run(tag + '-' + name,
                        [builds[tag] / 'regional-maintenance-encounter-test', directory / 'initial.snapshot',
                         scenario, args.ticks, snapshot, trace, 16], env=env))
                    if snapshot.read_bytes() != (directory / 'final.snapshot').read_bytes() or trace.read_bytes() != (directory / 'trace.txt').read_bytes():
                        raise ValueError('Native/sanitized journey differs: ' + name)
                    restores += result['restores']
            directory = out / f'scenario-{scenario}-seed-{args.seeds[0]}'
            previous, last = directory / 'initial.snapshot', 0
            for end in sorted({1, 2, 4, 16, 17, 65, 129, 193, 257, 385, 513, 641, 1024, 1536, 1537, args.ticks}):
                name = f'resume-{scenario}-{end}'
                snapshot, trace = out / (name + '.snapshot'), out / (name + '.trace')
                run(name, [builds['native64'] / 'regional-maintenance-encounter-test', previous,
                           scenario, end - last, snapshot, trace, 0], env={**env, 'PATH': '/nonexistent'})
                previous, last = snapshot, end
                restores += 1
            if previous.read_bytes() != (directory / 'final.snapshot').read_bytes():
                raise ValueError('Fresh-process continuation differs')
        report['journeys'] = journeys
        if any(not v['arrivals'] or not v['graph_changes'] or not v['terrain_events'] or not v['repair_ticks'] or v['reorders'] != 1 for v in journeys):
            raise ValueError('Dynamic journey branch/arrival witness absent')
        if not sum(v['active_route_edits'] for v in journeys):
            raise ValueError('No edit overlapped a retained actor route')
        graph_regression = []
        for fixture in range(11):
            rows = [json.loads(run(tag + '-static-graph-' + str(fixture),
                [build / 'regional-graph-test', *([pe] if tag == 'original32' else []), args.install, fixture], env=env))
                for tag, build in builds.items()]
            if not all(v == rows[0] for v in rows):
                raise ValueError('Static graph regression differs')
            graph_regression.append(rows[0])
        report['static_graph_regression'] = graph_regression
        navigation = []
        for tag, build in builds.items():
            navigation.append(json.loads(run(tag + '-navigation-regression',
                [build / 'regional-navigation-test', *([pe] if tag == 'original32' else []), args.install], env=env)))
            run(tag + '-admission', [build / 'regional-map-admission-test'], env=env)
        if not all(v == navigation[0] for v in navigation):
            raise ValueError('Regional navigation regression differs')
        report['navigation_regression'] = navigation[0]
        for scenario in range(5):
            run('static-journey-' + str(scenario), [builds['original32'] / 'regional-graph-encounter-test',
                pe, args.install, scenario, 17, 1024, out / ('static-journey-' + str(scenario))], env=env)
        for tag in ['native64', 'sanitized']:
            run(tag + '-core', ['ctest', '--test-dir', builds[tag] / 'command-session', '-R',
                '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env=env)
            run(tag + '-installed', [builds[tag] / 'native-encounter-test', args.install], env=env)
        cli = builds['native64'] / 'native-encounter'
        initial, edited = out / 'cli-initial.snapshot', out / 'cli-edited.snapshot'
        run('cli-initial', [cli, 'run-regional', args.install, 0, 0, 32, 32, 17, 0, initial], env=env)
        run('cli-edit', [cli, 'edit-terrain', initial, 16, 6, 20, 0, 16384, 16384, edited], env={**env, 'PATH': '/nonexistent'})
        whole, first, resumed = [out / ('cli-' + n + '.snapshot') for n in ['whole', 'first', 'resumed']]
        run('cli-whole', [cli, 'resume', edited, 256, whole], env=env)
        run('cli-first', [cli, 'resume', edited, 1, first], env={**env, 'PATH': '/nonexistent'})
        run('cli-resumed', [cli, 'resume', first, 255, resumed], env={**env, 'PATH': '/nonexistent'})
        if whole.read_bytes() != resumed.read_bytes():
            raise ValueError('CLI edited checkpoint continuation differs')
        refused = out / 'cli-refused.snapshot'
        run('cli-refusal', [cli, 'edit-terrain', edited, 32, 0, 0, 0, 16384, 0, refused], expected=1, env=env)
        if refused.exists():
            raise ValueError('Refused edit published a checkpoint')
        report.update(success=True, native_passed=True, sanitizer_passed=True,
                      process_resume_identical=True, process_resumes=restores,
                      cli_edit_resume_identical=True, original_target_text_unchanged=True,
                      compared_ticks=sum(v['ticks'] for v in journeys))
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT / p) == v for p, v in declaration['sources'].items())
        report['build_inputs_sha256'] = sources
        report['build_inputs_unchanged'] = all(sha(ROOT / p) == v for p, v in sources.items())
        report['installed_inputs_unchanged'] = all(sha(args.install / p) == v for p, v in report.get('installed_inputs', {}).items())
        report['artifacts_sha256'] = {str(p.relative_to(out)): sha(p) for p in out.rglob('*') if p.is_file()}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not all(report[k] for k in ['sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged']):
        raise ValueError('Inputs changed during execution')
    print(json.dumps({k: report[k] for k in ['success', 'compared_ticks', 'helper_checks', 'process_resumes', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
