#!/usr/bin/env python3
"""Pinned original/native owner knowledge, danger integration and portable saves."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SCOPE = ('Pinned4181d0 modes1/2 creature memory with absent task/notification lists; '
         'owner/type/distance matching, anonymous keys, markers, enabled-owner slices, '
         'signed age>1024 expiry and danger-consumed snapshot projection. '
         'Eight synthetic lifecycle domains, four installed32x32x23 ordinary-type17 '
         'paired journeys with two seeds, native64/sanitizer and portable CMD1v10. '
         'Visibility inputs, one brain per frame and empty task pointers are explicit; '
         'geometric visibility, strategic consequences, dual-brain World/campaign '
         'and live replacement excluded.')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def reuse_engine_run(args, declaration, run, report):
    """Complete the exact v9-assertion failure; never rebind changed engine code.

    This is component evidence composition, not a new original execution. The
    original report, fingerprints and artifacts remain immutable. Only the
    independent legacy wire-version assertion and this recovery driver may
    differ; all engine/paired-test sources and scope declarations must match.
    """
    previous = args.reuse_engine_run.resolve()
    raw = previous / 'report.json'
    old = json.loads(raw.read_text())
    if old['scope'] != SCOPE or old['source_executable_sha256'] != PIN or old['claims'] != declaration['claims']:
        raise ValueError('Cached engine run has different scope/build/claims')
    if old['success'] or any(not old.get(k) for k in [
            'sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged',
            'original_manifest_verified_before_after']):
        raise ValueError('Recovery requires the preserved incomplete source-stable run')
    if old['installed_inputs'] != report['installed_inputs']:
        raise ValueError('Cached installed inputs differ')
    if set(old['source_sha256']) != set(declaration['sources']):
        raise ValueError('Cached source closure differs')
    changed = {p for p, value in old['source_sha256'].items()
               if declaration['sources'][p] != value}
    allowed = {'tools/test-owner-knowledge.py', 'tests/native-command-action-bodies-session-test.cpp'}
    if changed != allowed:
        raise ValueError('Recovery allows only its driver and the v9-to-v10 assertion')
    for p in changed:
        if sha(previous / 'retained-sources' / p) != old['source_sha256'][p]:
            raise ValueError('Missing exact superseded source: ' + p)
    legacy = 'tests/native-command-action-bodies-session-test.cpp'
    before = (previous / 'retained-sources' / legacy).read_bytes()
    if before.count(b'bytes[4]==9') != 1 or before.replace(b'bytes[4]==9', b'bytes[4]==10') != (ROOT / legacy).read_bytes():
        raise ValueError('Legacy assertion has additional changes')
    for p, value in old['artifacts_sha256'].items():
        target = (previous / p).resolve()
        if not target.is_relative_to(previous) or sha(target) != value:
            raise ValueError('Changed cached artifact: ' + p)
    core = (previous / 'native64-core.stdout').read_text()
    if ('full action wire version missing' not in core or
            '80% tests passed, 1 tests failed out of 5' not in core or
            len(list(previous.glob('sanitized-core.stdout'))) != 0):
        raise ValueError('Cached failure is not the admitted legacy assertion')
    if len(old.get('journeys', [])) != 4 * len(args.seeds):
        raise ValueError('Cached journey set incomplete')
    restores = 0
    for scenario in range(4):
        for seed in args.seeds:
            name = f'scenario-{scenario}-seed-{seed}'
            directory = previous / name
            oracle = json.loads((previous / (name + '.stdout')).read_text())
            if not oracle['success'] or oracle['ticks'] != args.ticks:
                raise ValueError('Cached original journey incomplete')
            for tag in ['native64', 'sanitized']:
                row = json.loads((previous / (tag + '-' + name + '.stdout')).read_text())
                if (not row['success'] or row['ticks'] != args.ticks or
                        (previous / (tag + '-' + name + '.snapshot')).read_bytes() != (directory / 'final.snapshot').read_bytes() or
                        (previous / (tag + '-' + name + '.trace')).read_bytes() != (directory / 'trace.txt').read_bytes()):
                    raise ValueError('Cached native replay differs')
                restores += row['restores']
        for end in sorted({1, 2, 4, 16, 17, 64, 65, 129, 193, 256, 257, 513, 1024, 1089, 1153, 1408, 1409, 1792, 1793, args.ticks}):
            row = json.loads((previous / f'resume-{scenario}-{end}.stdout').read_text())
            if not row['success']:
                raise ValueError('Cached fresh process failed')
            restores += 1
        if ((previous / f'resume-{scenario}-{args.ticks}.snapshot').read_bytes() !=
                (previous / f'scenario-{scenario}-seed-{args.seeds[0]}' / 'final.snapshot').read_bytes()):
            raise ValueError('Cached fresh process final state differs')
    for key in ['helper', 'state_helper', 'danger_regression']:
        if not old[key]['success']:
            raise ValueError('Cached component failed: ' + key)
    for key in ['helper', 'helper_checks', 'state_helper', 'journeys', 'journey_workers', 'danger_regression']:
        report[key] = old[key]
    report['reused_engine_run'] = dict(
        record=str(raw), sha256=sha(raw), original_execution_repeated=False,
        unchanged_engine_and_paired_sources=True, verified_artifacts=len(old['artifacts_sha256']),
        changed_non_engine_sources=sorted(changed),
        retained_prior_sources={p: old['source_sha256'][p] for p in sorted(changed)},
        scope='Original/native helpers, eight paired journeys, native64/sanitizer replays and80 fresh checkpoints reused only after complete source/scope/input/artifact verification. Core suites, installed regressions and CLI completion execute anew.')
    builds = {tag: args.build_root.resolve() / tag for tag in ['original32', 'native64', 'sanitized']}
    for tag in ['native64', 'sanitized']:
        run(tag + '-rebuild-version-assertion', ['cmake', '--build', builds[tag], '--target',
            'native-command-action-bodies-session-test', '-j2'])
    sources = {p: sha(ROOT / p) for p in old['build_inputs_sha256']}
    return builds, sources, restores, old['journeys']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--install', type=Path, required=True)
    parser.add_argument('--build-root', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--ticks', type=int, default=2304)
    parser.add_argument('--seeds', type=int, nargs='+', default=[17, 91])
    parser.add_argument('--reuse-engine-run', type=Path,
                        help='Complete the exact retained v9 assertion failure; engine sources must be unchanged')
    args = parser.parse_args()
    if args.reuse_engine_run and not args.claims:
        parser.error('Recovery requires a prospective source/scope declaration')
    if args.ticks < 2304 or args.ticks > 8192:
        parser.error('The complete journey requires2304..8192 ticks')
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
        if args.reuse_engine_run:
            builds, sources, restores, journeys = reuse_engine_run(args, declaration, run, report)
        else:
            dependencies = SourceFileLoader('danger_dependencies', str(ROOT / 'tools/test-installed-encounter.py')).load_module()
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
                    'regional-maintenance-test.cpp', 'regional-maintenance-encounter-test.cpp',
                    'regional-danger-test.cpp', 'native-regional-danger-test.cpp', 'regional-danger-encounter-test.cpp',
                    'owner-knowledge-test.cpp', 'native-owner-knowledge-test.cpp'})
                closure.update({'tools/test-owner-knowledge.py', 'tools/export-owner-knowledge.py',
                                'tools/test-regional-danger.py', 'tools/export-regional-danger.py',
                                'tools/test-regional-maintenance.py', 'tools/export-regional-repair.py',
                                'tools/test-regional-graph.py', 'tools/export-regional-graph.py',
                                'tools/test-regional-navigation.py', 'tools/export-regional-navigation.py',
                                'tools/ghidra/ExportPersistenceProgression.java'})
                sources.update({p: sha(ROOT / p) for p in closure})
                if args.claims and closure - declaration['sources'].keys():
                    raise ValueError('Undeclared build inputs: ' + str(sorted(closure - declaration['sources'].keys())))
                targets = ['owner-knowledge-test', 'native-owner-knowledge-test', 'owner-knowledge-encounter-test',
                           'regional-danger-test', 'native-regional-danger-test', 'native-encounter',
                           'native-command-session-test', 'native-command-behavior-session-test',
                           'native-command-action-session-test', 'native-command-action-bodies-session-test',
                           'native-creature-providers-test', 'native-encounter-test']
                run(tag + '-build', ['cmake', '--build', build, '--target', *targets, '-j2'])
            helpers = [json.loads(run(tag + '-knowledge-helper',
                [build / 'owner-knowledge-test', *([pe] if tag == 'original32' else []), args.install], env=env))
                for tag, build in builds.items()]
            if not all(row == helpers[0] for row in helpers):
                raise ValueError('Knowledge helper architecture/sanitizer mismatch')
            report['helper'] = helpers[0]
            report['helper_checks'] = helpers[0]['checks']
            state = [json.loads(run(tag + '-knowledge-state',
                [builds[tag] / 'native-owner-knowledge-test', args.install], env=env))
                for tag in ['native64', 'sanitized']]
            if state[0] != state[1]:
                raise ValueError('Knowledge state architecture/sanitizer mismatch')
            report['state_helper'] = state[0]
            run('static', [sys.executable, ROOT / 'tools/export-owner-knowledge.py', '--output', out / 'static'])
            def journey(case):
                scenario, seed = case
                name = f'scenario-{scenario}-seed-{seed}'
                directory = out / name
                row = json.loads(run(name, [builds['original32'] / 'owner-knowledge-encounter-test',
                                           pe, args.install, scenario, seed, args.ticks, directory], env=env))
                restored = 0
                for tag in ['native64', 'sanitized']:
                    snapshot, trace = out / (tag + '-' + name + '.snapshot'), out / (tag + '-' + name + '.trace')
                    result = json.loads(run(tag + '-' + name,
                        [builds[tag] / 'owner-knowledge-encounter-test', directory / 'initial.snapshot',
                         scenario, args.ticks, snapshot, trace, 16], env=env))
                    if snapshot.read_bytes() != (directory / 'final.snapshot').read_bytes() or trace.read_bytes() != (directory / 'trace.txt').read_bytes():
                        raise ValueError('Native/sanitized journey differs: ' + name)
                    restored += result['restores']
                return dict(scenario=scenario, seed=seed, **row), restored

            # Each oracle and replay has private address space and unique output paths.
            # Builds and discovery finish before these workers start; installed inputs
            # are read-only, and the outer storage guard covers every child process.
            with ThreadPoolExecutor(max_workers=3) as pool:
                results = list(pool.map(journey, [(scenario, seed) for scenario in range(4) for seed in args.seeds]))
            journeys, restores = [v[0] for v in results], sum(v[1] for v in results)
            report['journey_workers'] = 3

            def checkpoints(scenario):
                directory = out / f'scenario-{scenario}-seed-{args.seeds[0]}'
                previous, last, restored = directory / 'initial.snapshot', 0, 0
                for end in sorted({1, 2, 4, 16, 17, 64, 65, 129, 193, 256, 257, 513, 1024, 1089, 1153, 1408, 1409, 1792, 1793, args.ticks}):
                    name = f'resume-{scenario}-{end}'
                    snapshot, trace = out / (name + '.snapshot'), out / (name + '.trace')
                    run(name, [builds['native64'] / 'owner-knowledge-encounter-test', previous,
                               scenario, end - last, snapshot, trace, 0], env={**env, 'PATH': '/nonexistent'})
                    previous, last = snapshot, end
                    restored += 1
                if previous.read_bytes() != (directory / 'final.snapshot').read_bytes():
                    raise ValueError('Fresh-process continuation differs')
                return restored

            with ThreadPoolExecutor(max_workers=3) as pool:
                restores += sum(pool.map(checkpoints, range(4)))
            report['journeys'] = journeys
            if any(not v['danger_frames'] or not v['row_changes'] for v in journeys):
                raise ValueError('Published danger witness absent')
            if not all(v['hidden_frames'] and v['remembered_frames'] for v in journeys if v['scenario']==1):
                raise ValueError('Hidden moving threat memory witness absent')
            if not all(v['hits'] for v in journeys if v['scenario']==3) or not any(v['deaths'] for v in journeys if v['scenario']==3):
                raise ValueError('Damage/death witness absent')
            regression = []
            for tag, build in builds.items():
                regression.append(json.loads(run(tag + '-danger-regression',
                    [build / 'regional-danger-test', *([pe] if tag == 'original32' else []), args.install], env=env)))
            if not all(v == regression[0] for v in regression):
                raise ValueError('Danger producer regression differs')
            report['danger_regression'] = regression[0]
        legacy_states = []
        for tag in ['native64', 'sanitized']:
            legacy_states.append(json.loads(run(tag + '-legacy-danger-state',
                [builds[tag] / 'native-regional-danger-test', args.install], env=env)))
            run(tag + '-core', ['ctest', '--test-dir', builds[tag] / 'command-session', '-R',
                '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env=env)
            run(tag + '-installed', [builds[tag] / 'native-encounter-test', args.install], env=env)
        if legacy_states[0] != legacy_states[1]:
            raise ValueError('Legacy observation state regression differs')
        report['legacy_state_regression'] = legacy_states[0]
        cli = builds['native64'] / 'native-encounter'
        initial, edited = out / 'cli-initial.snapshot', out / 'cli-edited.snapshot'
        run('cli-initial', [cli, 'run-knowledge', args.install, 0, 0, 32, 32, 17, 0, initial], env=env)
        run('cli-edit', [cli, 'edit-terrain', initial, 16, 6, 20, 0, 16384, 16384, edited], env={**env, 'PATH': '/nonexistent'})
        whole, first, resumed = [out / ('cli-' + n + '.snapshot') for n in ['whole', 'first', 'resumed']]
        run('cli-whole', [cli, 'resume', edited, 2304, whole], env=env)
        run('cli-first', [cli, 'resume', edited, 1, first], env={**env, 'PATH': '/nonexistent'})
        run('cli-resumed', [cli, 'resume', first, 2303, resumed], env={**env, 'PATH': '/nonexistent'})
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
        if 'reused_engine_run' in report:
            prior = Path(report['reused_engine_run']['record'])
            prior_data = json.loads(prior.read_text())
            report['reused_inputs_unchanged'] = (
                sha(prior) == report['reused_engine_run']['sha256'] and
                all(sha(prior.parent / p) == value for p, value in prior_data['artifacts_sha256'].items()))
        report['artifacts_sha256'] = {str(p.relative_to(out)): sha(p) for p in out.rglob('*') if p.is_file()}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not all(report[k] for k in ['sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged']) or not report.get('reused_inputs_unchanged', True):
        raise ValueError('Inputs changed during execution')
    print(json.dumps({k: report[k] for k in ['success', 'compared_ticks', 'helper_checks', 'process_resumes', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
