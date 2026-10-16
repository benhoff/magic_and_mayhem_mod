#!/usr/bin/env python3
"""Source-bound owner visibility, coupled original encounters and portable saves."""
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
SCOPE = ('Owner visibility52b540/508ca0/5150a0/515270/4e14a0 simulation projection; '
         'local player-1, no observation points; ordinary installed type17 native '
         'perception to knowledge/danger/navigation with empty attenuation columns. '
         'Four32x32x23 paired scenarios, two seeds,2304 ticks, native64/sanitizers '
         'and CMD1v11 portable continuation. UI discovery, commander reveal, neutral '
         'owners, fog points, column production, fullWorld cadence/campaign/live excluded.')


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def validate_reuse_state(old, declaration, installed):
    if old['scope'] != SCOPE or old['source_executable_sha256'] != PIN or old['claims'] != declaration['claims']:
        raise ValueError('Cached scope/build/claims differ')
    if old['success'] or not all(old.get(k) for k in ['sources_unchanged', 'build_inputs_unchanged', 'original_manifest_verified_before_after']):
        raise ValueError('Recovery requires the incomplete, stable engine stage')
    if old['installed_inputs'] != installed or set(old['source_sha256']) != set(declaration['sources']):
        raise ValueError('Cached inputs/source closure differ')
    changed = {p for p, h in old['source_sha256'].items() if declaration['sources'][p] != h}
    if changed != {'tools/test-owner-visibility-integration.py'}:
        raise ValueError('Only the non-engine recovery driver may change')


def validate_artifacts(previous, artifacts):
    for p, value in artifacts.items():
        target = (previous / p).resolve()
        if not target.is_relative_to(previous) or sha(target) != value:
            raise ValueError('Changed cached artifact: ' + p)


def reuse_engine(args, declaration, report, builds, sources):
    import re
    import shlex
    previous = args.reuse_engine_run.resolve()
    raw = previous / 'report.json'
    retained = ROOT / 'research/runtime/owner-visibility-engine-stage-ns77-20261016.json'
    if raw.read_bytes() != retained.read_bytes():
        raise ValueError('Cached report differs from the preserved engine-stage record')
    old = json.loads(raw.read_text())
    validate_reuse_state(old, declaration, report['installed_inputs'])
    archive = json.loads((ROOT / 'research/runtime/owner-visibility-retained-sources-ns77-20261016.json').read_text())
    driver = 'tools/test-owner-visibility-integration.py'
    if hashlib.sha256(archive['sources'][driver].encode()).hexdigest() != old['source_sha256'][driver]:
        raise ValueError('Superseded driver bytes missing')
    core = (previous / 'native64-core.stdout').read_text()
    diagnostics = (previous / 'native64-core.stderr').read_text()
    if ('Unable to find executable' not in diagnostics or
            any(not re.search(r'Test\s+#'+str(i)+r': [^\n]+Passed', core) for i in range(1,6))):
        raise ValueError('Cached failure is not unbuilt inherited CTest discovery')
    validate_artifacts(previous, old['artifacts_sha256'])
    if len(old.get('journeys', [])) != 8 or old.get('fresh_process_restores') != 80:
        raise ValueError('Cached engine journeys/checkpoints incomplete')
    restores = 0
    input_names = list(report['installed_inputs'])
    for scenario in range(4):
        for seed in [17, 91]:
            name = f'scenario-{scenario}-seed-{seed}'
            directory = previous / name
            oracle = json.loads((previous / (name+'.stdout')).read_text())
            if not oracle['success'] or oracle['ticks'] != 2304:
                raise ValueError('Cached original journey incomplete')
            # Bind actual consumed/embedded asset bytes, not just a later reread.
            data = (directory / 'initial.snapshot').read_bytes()
            at = data.index(b'MNMENC01') + 8 + 16
            for key in input_names:
                n = int.from_bytes(data[at:at+4], 'little'); at += 4
                if hashlib.sha256(data[at:at+n]).hexdigest() != old['installed_inputs'][key]:
                    raise ValueError('Embedded installed input differs: '+key)
                at += n
            for tag in ['native64', 'sanitized']:
                row = json.loads((previous / (tag+'-'+name+'.stdout')).read_text())
                if (not row['success'] or row['ticks'] != 2304 or
                        (previous / (tag+'-'+name+'.snapshot')).read_bytes() != (directory/'final.snapshot').read_bytes() or
                        (previous / (tag+'-'+name+'.trace')).read_bytes() != (directory/'trace.txt').read_bytes()):
                    raise ValueError('Cached native/sanitized replay differs')
                restores += row['restores']
        if ((previous / f'resume-{scenario}-2304.snapshot').read_bytes() !=
                (previous / f'scenario-{scenario}-seed-17/final.snapshot').read_bytes()):
            raise ValueError('Cached cold continuation differs')
    if restores != old['journey_restores']:
        raise ValueError('Cached restore accounting differs')
    for key in ['helper', 'native-owner-visibility-test', 'native-owner-knowledge-test', 'native-regional-danger-test', 'native-encounter-test']:
        if not old[key]['success']:
            raise ValueError('Cached suite incomplete: '+key)
    # The failed driver checked its computed closure but persisted it only on
    # success. Recover that list from immutable preprocessor logs and unchanged
    # CMake source declarations; every path must match the prospective closure.
    closure = {'tools/test-installed-encounter.py', driver, 'tools/test-owner-visibility.py',
               'tools/export-owner-visibility.py', 'tools/export-regional-navigation.py',
               'tools/ghidra/ExportPersistenceProgression.java'}
    for log in previous.glob('*-dependencies-*.stdout'):
        for token in shlex.split(log.read_text().replace('\\\n', ' ')):
            q = Path(token)
            q = q.resolve() if q.is_absolute() else (ROOT / q).resolve()
            if q.is_file() and q.is_relative_to(ROOT) and not q.is_relative_to(ROOT/'working'):
                closure.add(q.relative_to(ROOT).as_posix())
    for tag in ['original32', 'native64', 'sanitized']:
        builds[tag] = args.build_root.resolve()/tag
        for makefile in builds[tag].rglob('CMakeFiles/Makefile.cmake'):
            for token in re.findall(r'"([^"]+)"', makefile.read_text()):
                q = Path(token).resolve()
                if q.is_file() and q.is_relative_to(ROOT) and not q.is_relative_to(ROOT/'working'):
                    closure.add(q.relative_to(ROOT).as_posix())
    if closure - declaration['sources'].keys():
        raise ValueError('Cached compiler/configure closure contains undeclared inputs')
    sources.update({p: sha(ROOT/p) for p in closure})
    for key in ['helper', 'journeys', 'journey_restores', 'fresh_process_restores',
                'native-owner-visibility-test', 'native-owner-knowledge-test', 'native-regional-danger-test', 'native-encounter-test']:
        report[key] = old[key]
    report['reused_engine_run'] = dict(record=retained.relative_to(ROOT).as_posix(), sha256=sha(retained),
        original_execution_repeated=False, unchanged_engine_and_paired_sources=True,
        verified_artifacts=len(old['artifacts_sha256']), embedded_input_sets_verified=8,
        compiler_closure_reconstructed_from_retained_logs=True,
        boundary='Complete original/native engine helpers, eight journeys, native64/sanitizer replays,80 cold checkpoints and four native/sanitizer state/regression suites retained. Only the validation driver changes; five intended CTests and CLI completion execute anew.')
    return previous, old

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ['output', 'install', 'build-root', 'claims']:
        parser.add_argument('--' + key, type=Path, required=True)
    parser.add_argument('--reuse-engine-run', type=Path, help='Complete the exact retained unbuilt-CTest failure after strict source/input/artifact checks')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').write_text('NS77 source-bound visibility evidence; preserve raw comparisons.\n')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT / p) != v for p, v in declaration['sources'].items()):
        raise ValueError('Declared source changed before execution')
    report = dict(success=False, scope=SCOPE, source_executable_sha256=PIN,
                  claims=declaration['claims'], source_sha256=declaration['sources'],
                  original_claims=[c for c in declaration['claims'] if c['behavior'] in {'AI.owner-visibility-production', 'AI.fine-coordinate-facing'}],
                  wine_required=False, live_replacement=False, commands=[])
    builds, sources = {}, {}

    def run(name, command, env=None, expected=0):
        command = [str(v) for v in command]
        report['commands'].append(command)
        row = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                             timeout=900, env=env)
        (out / (name + '.stdout')).write_text(row.stdout)
        (out / (name + '.stderr')).write_text(row.stderr)
        if row.returncode != expected or 'ERROR: AddressSanitizer' in row.stderr or 'runtime error:' in row.stderr:
            raise RuntimeError(name + ' failed: ' + str(row.returncode) + '\n' + row.stdout[-1500:] + row.stderr[-3000:])
        return row.stdout

    env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1',
           'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}
    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != PIN:
            raise ValueError('Original executable differs')
        inputs = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani',
                  'Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd']
        report['installed_inputs'] = {p: sha(args.install / p) for p in inputs}
        reused = None
        if args.reuse_engine_run:
            reused = reuse_engine(args, declaration, report, builds, sources)
            for tag in ['native64', 'sanitized']:
                run(tag+'-rebuild-completion', ['cmake', '--build', builds[tag], '--target',
                    'native-encounter', 'native-command-session-test', 'native-command-behavior-session-test',
                    'native-command-action-session-test', 'native-command-action-bodies-session-test',
                    'native-creature-providers-test', '-j2'])
        else:
            dependencies = SourceFileLoader('visibility_dependencies', str(ROOT / 'tools/test-installed-encounter.py')).load_module()
            for tag, options in [
                ('original32', ['-DCMAKE_CXX_FLAGS=-m32 -O1 -g', '-DCMAKE_EXE_LINKER_FLAGS=-m32 -no-pie', '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=ON']),
                ('native64', ['-DCMAKE_BUILD_TYPE=', '-DCMAKE_CXX_FLAGS=-O1 -g']),
                ('sanitized', ['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
                build = args.build_root.resolve() / tag
                builds[tag] = build
                run(tag + '-configure', ['cmake', '-S', ROOT / 'apps/native-encounter', '-B', build,
                                         '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', *options])
                closure = dependencies.dependencies(build, run, tag, extra={
                    'owner-visibility-test.cpp', 'native-owner-visibility-test.cpp',
                    'regional-danger-encounter-test.cpp', 'native-regional-danger-test.cpp',
                    'owner-knowledge-test.cpp', 'native-owner-knowledge-test.cpp'})
                closure.update({'tools/test-owner-visibility-integration.py', 'tools/test-owner-visibility.py',
                                'tools/export-owner-visibility.py', 'tools/export-regional-navigation.py',
                                'tools/ghidra/ExportPersistenceProgression.java'})
                sources.update({p: sha(ROOT / p) for p in closure})
                if closure - declaration['sources'].keys():
                    raise ValueError('Undeclared compiler/configure inputs: ' + str(sorted(closure - declaration['sources'].keys())))
                targets = ['owner-visibility-encounter-test']
                if tag != 'original32':
                    targets += ['native-owner-visibility-test', 'native-owner-knowledge-test',
                                'native-regional-danger-test', 'native-encounter', 'native-encounter-test',
                                'native-command-session-test', 'native-command-behavior-session-test',
                                'native-command-action-session-test', 'native-command-action-bodies-session-test',
                                'native-creature-providers-test']
                run(tag + '-build', ['cmake', '--build', build, '--target', *targets, '-j2'])
            run('helpers', [sys.executable, ROOT / 'tools/test-owner-visibility.py',
                            '--claims', args.claims, '--output', out / 'helpers'])
            report['helper'] = json.loads((out / 'helpers/report.json').read_text())
            run('static', [sys.executable, ROOT / 'tools/export-owner-visibility.py', '--output', out / 'static'])

            def journey(case):
                scenario, seed = case
                name = f'scenario-{scenario}-seed-{seed}'
                directory = out / name
                original = json.loads(run(name, [builds['original32'] / 'owner-visibility-encounter-test',
                    pe, args.install, scenario, seed, 2304, directory], env=env))
                restores = 0
                for tag in ['native64', 'sanitized']:
                    snapshot, trace = out / (tag + '-' + name + '.snapshot'), out / (tag + '-' + name + '.trace')
                    result = json.loads(run(tag + '-' + name, [builds[tag] / 'owner-visibility-encounter-test',
                        directory / 'initial.snapshot', scenario, 2304, snapshot, trace, 16], env=env))
                    if snapshot.read_bytes() != (directory / 'final.snapshot').read_bytes() or trace.read_bytes() != (directory / 'trace.txt').read_bytes():
                        raise ValueError('Native/sanitized replay differs: ' + name)
                    restores += result['restores']
                return dict(scenario=scenario, seed=seed, **original), restores

            with ThreadPoolExecutor(max_workers=3) as pool:
                results = list(pool.map(journey, [(s, seed) for s in range(4) for seed in [17, 91]]))
            report['journeys'] = [r[0] for r in results]
            report['journey_restores'] = sum(r[1] for r in results)

            def checkpoints(scenario):
                directory = out / f'scenario-{scenario}-seed-17'
                previous, last, count = directory / 'initial.snapshot', 0, 0
                for end in [1, 2, 12, 13, 16, 64, 65, 96, 128, 129, 256, 257, 513, 1024, 1089, 1408, 1409, 1792, 1793, 2304]:
                    name = f'resume-{scenario}-{end}'
                    snapshot, trace = out / (name + '.snapshot'), out / (name + '.trace')
                    run(name, [builds['native64'] / 'owner-visibility-encounter-test', previous,
                               scenario, end - last, snapshot, trace, 0], env={**env, 'PATH': '/nonexistent'})
                    previous, last, count = snapshot, end, count + 1
                if previous.read_bytes() != (directory / 'final.snapshot').read_bytes():
                    raise ValueError('Cold continuation differs')
                return count

            with ThreadPoolExecutor(max_workers=3) as pool:
                report['fresh_process_restores'] = sum(pool.map(checkpoints, range(4)))
            for suite in ['native-owner-visibility-test', 'native-owner-knowledge-test', 'native-regional-danger-test', 'native-encounter-test']:
                results = [json.loads(run(tag + '-' + suite, [builds[tag] / suite, args.install], env=env)) for tag in ['native64', 'sanitized']]
                if results[0] != results[1]:
                    raise ValueError('Native/sanitized suite differs: ' + suite)
                report[suite] = results[0]
        for tag in ['native64', 'sanitized']:
            run(tag + '-core', ['ctest', '--test-dir', builds[tag] / 'command-session', '-R', '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env=env)
        exe = builds['native64'] / 'native-encounter'
        linkage = run('linkage', ['ldd', exe])
        if any(s in linkage.lower() for s in ['wine', 'libqt']):
            raise ValueError('Unexpected runtime dependency')
        report['runtime_linkage'] = linkage
        run('cli-whole', [exe, 'run-visibility', args.install, 0, 0, 32, 32, 17, 2304, out / 'cli-whole.snapshot'])
        run('cli-initial', [exe, 'run-visibility', args.install, 0, 0, 32, 32, 17, 129, out / 'cli-initial.snapshot'])
        run('cli-resume', [exe, 'resume', out / 'cli-initial.snapshot', 2175, out / 'cli-resumed.snapshot'], env={**env, 'PATH': '/nonexistent'})
        if (out / 'cli-whole.snapshot').read_bytes() != (out / 'cli-resumed.snapshot').read_bytes():
            raise ValueError('CLI cold resume differs')
        if reused:
            validate_artifacts(reused[0], reused[1]['artifacts_sha256'])
            report['reused_inputs_unchanged'] = True
        report['build_inputs_sha256'] = sources
        report['installed_inputs_unchanged'] = all(sha(args.install / p) == v for p, v in report['installed_inputs'].items())
        report.update(success=True, paired_ticks=8 * 2304, native_passed=True, sanitizer_passed=True,
                      complete_original_execution=True, cli_resume_identical=True, core_suites_passed=True)
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT / p) == v for p, v in declaration['sources'].items())
        report['build_inputs_unchanged'] = all(sha(ROOT / p) == v for p, v in sources.items())
        if not report['sources_unchanged'] or not report['build_inputs_unchanged'] or not report.get('installed_inputs_unchanged', False):
            report['success'] = False
        report['artifacts_sha256'] = {p.relative_to(out).as_posix(): sha(p) for p in out.rglob('*') if p.is_file() and p != out / 'report.json'}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not report['success']:
        raise ValueError('Incomplete or changed-source execution')
    print(json.dumps({k: report[k] for k in ['success', 'paired_ticks', 'fresh_process_restores', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
