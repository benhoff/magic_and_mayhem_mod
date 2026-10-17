#!/usr/bin/env python3
"""Source-bound headless Crossroads chapter acceptance and process restarts."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('assets', 'build', 'sanitized', 'claims', 'output', 'scratch'):
        parser.add_argument('--'+name, type=Path, required=True)
    a = parser.parse_args()
    a.assets, a.build, a.sanitized, a.output, a.scratch = [p.resolve() for p in (a.assets, a.build, a.sanitized, a.output, a.scratch)]
    a.output.mkdir(parents=True, exist_ok=False)
    (a.output/'.retain').write_text('Crossroads chapter execution, retained transition inputs and logs.\n')
    a.scratch.mkdir(parents=True, exist_ok=False)
    (a.scratch/'.retain').write_text('Task-owned memory-backed continuation outputs; preserve during review. Durable inputs/hashes remain in the execution report.\n')
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    assert all(sha(ROOT/p) == h for p, h in sources.items())
    report = dict(success=False, claims=declaration['claims'], sources=sources, commands=[],
                  scratch=str(a.scratch), compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  scope='Native bounded campaign graph and Crossroads chapter. Headless normal/sanitized full-health opening, both exclusive routes and shared finale; actual earned Cure/Sphere delivery fixtures, graph/codec/refusal/retry contracts, fresh-process campaign transitions and old-save compatibility. No UI acceptance, original campaign/rules equivalence or live replacement.')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(name, args, ok=True):
        args = list(map(str, args))
        report['commands'].append(args)
        result = subprocess.run(args, cwd=ROOT, env=env, capture_output=True, text=True, timeout=1800)
        (a.output/(name+'.stdout')).write_text(result.stdout)
        (a.output/(name+'.stderr')).write_text(result.stderr)
        assert (result.returncode == 0) == ok, (name, result.stdout[-1500:], result.stderr[-1500:])
        assert 'runtime error:' not in result.stderr and 'ERROR: AddressSanitizer' not in result.stderr
        return result.stdout

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    installed = ['Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd', 'Realms/Celtic/Plains/Terrain.spr',
                 'CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani', 'CFG/Encrypted/effectani.cfg']
    installed += [p.relative_to(a.assets).as_posix() for p in sorted((a.assets/'Sprites').glob('*'))
                  if p.name.lower().startswith('effects') and p.suffix.lower()=='.ani']
    report['installed'] = {p: sha(a.assets/p) for p in installed}
    try:
        run('configure', ['cmake', '-S', ROOT/'apps/skirmish', '-B', a.build, '-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS_DEBUG=-g0'])
        run('build', ['cmake', '--build', a.build, '-j4'])
        report['normal_cache_sha256'] = sha(a.build/'CMakeCache.txt')
        report['core'] = json.loads(run('normal-core', [a.build/'world/campaign-graph-core-test']))
        report['normal'] = json.loads(run('normal-chapter', [a.build/'campaign-graph-test', a.assets, 'emit', a.output/'journeys']))
        run('sanitized-configure', ['cmake', '-S', ROOT/'apps/skirmish', '-B', a.sanitized, '-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS_DEBUG=-g0',
                                   '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie',
                                   '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitized-build', ['cmake', '--build', a.sanitized, '--target', 'campaign-graph-test', 'campaign-graph-core-test', '-j4'])
        report['sanitized_core'] = json.loads(run('sanitized-core', [a.sanitized/'world/campaign-graph-core-test']))
        report['sanitized'] = json.loads(run('sanitized-chapter', [a.sanitized/'campaign-graph-test', a.assets]))
        assert report['core'] == report['sanitized_core'] and report['normal'] == report['sanitized']
        report['ctest'] = run('ctest', ['ctest', '--test-dir', a.build, '--output-on-failure', '--exclude-regex', 'window'])
        cli = a.build/'mnm-map-campaign-cli'
        journeys = a.output/'journeys'
        checkpoints = sorted(journeys.glob('*.mnmc'))
        assert len(checkpoints) >= 17
        report['restart_checks'] = []
        for i, checkpoint in enumerate(checkpoints):
            def resume(label, source, ticks):
                destination = a.scratch/f'cut-{i}-{label}.mnmc'
                run(f'cut-{i}-{label}', [cli, a.assets, 'resume', source, destination, ticks])
                return destination
            roundtrip = resume('roundtrip', checkpoint, 0)
            assert roundtrip.read_bytes() == checkpoint.read_bytes()
            direct = resume('direct', checkpoint, 7)
            first = resume('first', checkpoint, 1)
            split = resume('split', first, 6)
            assert direct.read_bytes() == split.read_bytes()
            report['restart_checks'].append(dict(checkpoint=str(checkpoint.relative_to(ROOT)), sha256=sha(checkpoint), continuation_sha256=sha(direct)))
        # Public create/select/start commands must reproduce the same campaign bytes.
        idle, started = a.scratch/'idle.mnmc', a.scratch/'started.mnmc'
        run('create', [cli, a.assets, 'create-crossroads', idle])
        assert idle.read_bytes() == (journeys/'initial.mnmc').read_bytes()
        run('start-opening', [cli, a.assets, 'start', idle, started])
        assert started.read_bytes() == (journeys/'opening-start.mnmc').read_bytes()
        fork = journeys/'fork.mnmc'
        report['production_restarts'] = []
        for route, name in [(1, 'last-stand'), (2, 'watchpost')]:
            selected, active = a.scratch/f'select-{route}.mnmc', a.scratch/f'active-{route}.mnmc'
            run(f'select-{route}', [cli, a.assets, 'select', fork, selected, 'celtic', name])
            assert selected.read_bytes() == (journeys/f'route-{route}-selected.mnmc').read_bytes()
            run(f'start-route-{route}', [cli, a.assets, 'start', selected, active])
            assert active.read_bytes() == (journeys/f'route-{route}-start.mnmc').read_bytes()
            resumed = a.scratch/f'pilot-{route}.mnmc'
            run(f'pilot-route-{route}', [a.build/'campaign-graph-test', a.assets, 'pilot', journeys/f'route-{route}-mid.mnmc', resumed, 1024])
            assert resumed.read_bytes() == (journeys/f'route-{route}-win.mnmc').read_bytes()
            report['production_restarts'].append(dict(route=route, sha256=sha(resumed)))
            denied = a.scratch/f'denied-{route}.mnmc'
            run(f'locked-route-{route}', [cli, a.assets, 'select', resumed, denied, 'celtic', 'watchpost' if route == 1 else 'last-stand'], ok=False)
            assert not denied.exists()
            completed = json.loads(run(f'complete-{route}', [cli, a.assets, 'inspect', journeys/f'route-{route}-finale-win.mnmc']))
            assert completed['completed'] and completed['graph']['completed'] == [0, route, 3] and not completed['available_missions']
        denied = a.scratch/'denied-finale.mnmc'
        run('locked-finale', [cli, a.assets, 'select', idle, denied, 'celtic', 'arena-finale'], ok=False)
        assert not denied.exists()
        # Native campaign versions 1–4 retain exact saved bytes.
        legacy = [ROOT/'working/tests/staged-mission-bound-20261017-v6/started.mnmc',
                  ROOT/'working/tests/staged-mission-bound-20261017-v6/reward-reopened.mnmc',
                  ROOT/'working/tests/mission-objectives-bound-20261017-v3/boundaries/boundary-0.mnmc',
                  ROOT/'working/tests/mission-objectives-bound-20261017-v3/boundaries/finale.mnmc']
        old = ROOT/'working/tests/terrain-campaign-bound-20261016/execution'
        legacy += [sorted((old/name).rglob('*.mnmc'))[0] for name in ('terrain-window', 'legacy-creature-window', 'legacy-map-window')]
        report['legacy'] = {}
        for i, source in enumerate(legacy):
            assert source.is_file(), source
            destination = a.scratch/f'legacy-{i}.mnmc'
            args = [a.build/'mnm-campaign-cli', '--installation', a.assets] if 'legacy-creature-window' in str(source) else [cli, a.assets]
            run(f'legacy-{i}', [*args, 'resume', source, destination, 0])
            assert source.read_bytes() == destination.read_bytes()
            report['legacy'][str(source.relative_to(ROOT))] = sha(source)
        dependencies = set()
        for dep in a.build.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n', ' ').split()[1:]:
                path = Path(token)
                path = (path if path.is_absolute() else a.build/path).resolve()
                if path.is_file() and path.is_relative_to(ROOT): dependencies.add(path.relative_to(ROOT).as_posix())
        report['compiler_dependencies'] = sorted(dependencies)
        report['undeclared_dependencies'] = sorted(dependencies-set(sources))
        assert not report['undeclared_dependencies'], report['undeclared_dependencies']
        report['sources_unchanged'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_unchanged'] = all(sha(a.assets/p) == h for p, h in report['installed'].items())
        assert report['sources_unchanged'] and report['inputs_unchanged']
        report['success'] = True
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        (a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(dict(success=report['success'], report=str(a.output/'report.json'))))


if __name__ == '__main__':
    main()
