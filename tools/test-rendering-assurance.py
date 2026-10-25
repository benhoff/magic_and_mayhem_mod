#!/usr/bin/env python3
"""Renew bounded rendering comparisons with prospective claims and SQLite results.

Child JSON files are bounded interchange for existing comparators. Their verified
payloads and command diagnostics are imported into the immutable SQLite result.
No live replacement or complete renderer acceptance follows from this suite.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, evidence_digest, write_report

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SURFACES = ['NR.surface-backend.owned', 'NR.surface-backend.formats-pitch',
            'NR.surface-backend.ownership-recovery', 'NR.surface-copy.clipping']
PHASES = {
    'surfaces': (SURFACES, ['NR.surface-backend.owned.offline',
        'NR.surface-backend.formats-pitch.offline',
        'NR.surface-backend.ownership-recovery.offline', 'NR.surface-copy.clipping.backend']),
    'panels': (['RS.panel-pixel-domain'], ['native-panel-domain-20261010']),
    'fade': (['RS.fade-loop-state'], []),
    'word': (['RS.word-raster'], ['word-sprite-fixtures-20261007']),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--phase', choices=PHASES, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.absolute()
    if out.exists() or args.report.exists() or not out.is_relative_to(ROOT/'working'):
        parser.error('Require new checkout-local working output and evidence paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain rendering assurance attempts and diagnostics.\n')
    env = dict(os.environ, TMPDIR=str(ROOT/'working/tmp'), PYTHONDONTWRITEBYTECODE='1',
               LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4', QT_QPA_PLATFORM='xcb',
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    scenarios = {s['id']: s for s in register['scenarios']}
    selected, scenario_ids = PHASES[args.phase]
    paths = set().union(*(required_sources(behaviors[b]) for b in selected))
    paths.update(p for sid in scenario_ids for p in scenarios[sid]['tests'])
    paths.update(['tools/test-rendering-assurance.py', 'tools/coverage_claims.py',
                  'tools/coverage_evidence.py', 'tools/original-manifest.sh'])
    # Pin the complete linked renderer project before compilation. Generated .d
    # files independently check that no compiled project dependency was omitted.
    for directory in ('renderer', 'assets', 'protocols/include', 'reconstruction/rendering'):
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*')
                     if p.is_file() and (p.suffix in ('.cpp', '.hpp', '.h', '.c', '.inc', '.S', '.cmake')
                                         or p.name == 'CMakeLists.txt'))
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tools').glob('check-*surface*.py'))
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tests/fixtures/surfaces').iterdir()
                 if p.is_file())
    paths.update(['tests/render-surface-backend-test.cpp', 'tests/render-surface-formats-test.cpp',
                  'tests/render-surface-ownership-test.cpp', 'tests/render-surface-copy-fixtures.cpp',
                  'tests/surface-retry-model.cpp', 'tests/resource-fixtures.hpp'])
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=bid, contract_sha256=behavior_contract(behaviors[bid], builds),
                   scenarios={sid: scenario_contract(scenarios[sid]) for sid in scenario_ids
                              if bid in scenarios[sid]['behaviors']}) for bid in selected]
    declaration = out/'claims.json'
    declaration.write_text(json.dumps(dict(schema=1, claims=claims, sources=sources), indent=2)+'\n')
    report = dict(schema=1, success=False, phase=args.phase, sources=sources, claims=claims,
                  scope='Bounded current-source rendering component comparisons; retained standalone '
                        'driver fixtures remain separate from unchanged original executable comparisons. '
                        'No new game journey, live bypass, hardware or whole-renderer acceptance.',
                  original_sha256=BUILD, commands=[], results={}, inputs={},
                  live_replacement=False, complete_replacement=False,
                  artifacts=out.relative_to(ROOT).as_posix())

    def run(label, command, timeout=300):
        result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                capture_output=True, timeout=timeout)
        raw = result.stdout+b'\n'+result.stderr
        (out/(label+'.log')).write_bytes(raw)
        report['commands'].append(dict(label=label, argv=[str(x) for x in command],
            returncode=result.returncode, diagnostic=raw.decode(errors='replace')))
        print(label, result.returncode, flush=True)
        result.check_returncode()
        return result.stdout.decode()

    def import_result(name, path):
        data = json.loads(path.read_bytes())
        if data.get('success') is not True:
            raise ValueError(name+': child comparison did not pass')
        report['results'][name] = data
        report['inputs'][path.relative_to(ROOT).as_posix()] = sha(path)

    verified = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        verified = True
        if args.phase == 'surfaces':
            build = ROOT/'working/build/rendering-assurance'
            run('configure', ['cmake', '-S', ROOT/'renderer', '-B', build,
                              '-DCMAKE_BUILD_TYPE=Debug'])
            targets = ['render-surface-backend-test', 'render-surface-formats-test',
                       'render-surface-ownership-test', 'render-surface-copy-fixtures']
            run('build', ['cmake', '--build', build, '--target', *targets, '-j2'], 600)
            dependencies = set()
            for dep in build.rglob('*.o.d'):
                for token in shlex.split(dep.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                    path = Path(token).resolve()
                    if path.is_absolute() and path.is_relative_to(ROOT) and path.is_file():
                        relative = path.relative_to(ROOT).as_posix()
                        if not relative.startswith('working/'):
                            dependencies.add(relative)
            if dependencies-sources.keys():
                raise ValueError('Unbound compiler dependencies: '+repr(sorted(dependencies-sources.keys())))
            report['compiled_project_sources'] = sorted(dependencies)
            for label, script, extra in [
                ('backend', 'check-native-surface-backend.py', ['--combined']),
                ('formats', 'check-native-surface-formats.py', []),
                ('ownership', 'check-native-surface-ownership.py', []),
                ('clipping', 'check-native-surface-clipping.py', []),
            ]:
                path = out/(label+'.json')
                command = [sys.executable, '-B', ROOT/'tools'/script, '--build', build,
                           '--report', path, *extra]
                if label in ('backend', 'ownership'):
                    command += ['--executable', build/('render-surface-'+label+'-test')]
                run(label, ['xvfb-run', '-a', '-s', '-screen 0 1280x1024x24', *command])
                import_result(label, path)
        elif args.phase == 'panels':
            output = run('panels', [sys.executable, '-B', ROOT/'tools/test-panel-domain.py',
                                   '--claims', declaration])
            for line in output.splitlines():
                if line.startswith(str(ROOT/'working/tests/panel-domain/run-')):
                    import_result('panels', Path(line)/'report.json')
                    break
            else:
                raise ValueError('Panel child did not identify its retained report')
        elif args.phase == 'fade':
            entries = []
            for w, h in ((1, 1), (3, 2), (16, 3), (31, 9)):
                for stride in (w, w+3):
                    for fmt in (0, 1):
                        values = [(i*1273+fmt*31)&65535 for i in range(stride*h)]
                        entries.append(struct.pack('<4I', w, h, stride, fmt)
                                       +struct.pack('<'+'H'*len(values), *values))
            corpus = out/'fade.bin'
            corpus.write_bytes(b'MNMFADE1'+struct.pack('<I', len(entries))+b''.join(entries))
            report['inputs'][corpus.relative_to(ROOT).as_posix()] = sha(corpus)
            output = run('fade', [sys.executable, '-B', ROOT/'tools/test-fade-loop.py',
                                 '--claims', declaration, '--corpus', corpus])
            for line in output.splitlines():
                if line.startswith(str(ROOT/'working/tests/fade-loop/run-')):
                    import_result('fade', Path(line)/'report.json')
                    break
            else:
                raise ValueError('Fade child did not identify its retained report')
        else:
            path = out/'word.json'
            run('word', [sys.executable, '-B', ROOT/'tools/test-word-sprites.py', '--report', path])
            import_result('word', path)
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = verified
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = all(sha(ROOT/p) == h for p, h in report['inputs'].items())
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        # Preserve child provenance as payload hashes; old comparators sometimes
        # fingerprint SQLite pointer text, so that identity is kept only inside
        # their verbatim child report rather than used as current source proof.
        with Reader(ROOT):
            report['retained_oracle_payloads'] = {p: evidence_digest(ROOT/p) for child in report['results'].values()
                for p in child.get('sources', {}) if p.startswith('research/')}
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k: report.get(k) for k in ('success', 'phase', 'error', 'sources_stable')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
