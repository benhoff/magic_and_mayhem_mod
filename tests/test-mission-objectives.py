#!/usr/bin/env python3
"""Source-bound headless mission acceptance; no window or interactive playtest."""
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
    for name in ('assets', 'build', 'sanitized', 'claims', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    a = parser.parse_args()
    a.assets, a.build, a.sanitized, a.output = [p.resolve() for p in (a.assets, a.build, a.sanitized, a.output)]
    a.output.mkdir(parents=True, exist_ok=False)
    (a.output/'.retain').write_text('Headless objective mission execution and continuation evidence.\n')
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    assert all(sha(ROOT/p) == h for p, h in sources.items())
    report = dict(success=False, claims=declaration['claims'], sources=sources, commands=[],
                  scope='Bounded native objective graph and installed Last Stand mission; headless normal/sanitized contracts, timer and event restart boundaries, old-save compatibility and campaign reward/transition. No UI acceptance, original scripting/rules equivalence or live replacement.')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(name, args, ok=True):
        args = list(map(str, args))
        report['commands'].append(args)
        result = subprocess.run(args, cwd=ROOT, env=env, capture_output=True, text=True, timeout=900)
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
        report['normal'] = json.loads(run('normal-contract', [a.build/'mission-objectives-test', a.assets, 'emit', a.output/'boundaries']))
        run('sanitized-configure', ['cmake', '-S', ROOT/'apps/skirmish', '-B', a.sanitized, '-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS_DEBUG=-g0',
                                   '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie',
                                   '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitized-build', ['cmake', '--build', a.sanitized, '--target', 'mission-objectives-test', '-j4'])
        report['sanitized'] = json.loads(run('sanitized-contract', [a.sanitized/'mission-objectives-test', a.assets]))
        assert report['normal'] == report['sanitized']
        report['legacy_staged_contract'] = json.loads(run('legacy-staged-contract', [a.build/'staged-mission-test', a.assets]))
        report['ctest'] = run('ctest', ['ctest', '--test-dir', a.build, '--output-on-failure', '--exclude-regex', 'window'])
        cli = a.build/'mnm-map-campaign-cli'
        checkpoints = sorted((a.output/'boundaries').glob('*.mnmc'))
        assert len(checkpoints) == report['normal']['boundaries']+1
        report['restart_checks'] = []
        for i, checkpoint in enumerate(checkpoints):
            def resume(label, source, ticks):
                destination = a.output/f'cut-{i}-{label}.mnmc'
                run(f'cut-{i}-{label}', [cli, a.assets, 'resume', source, destination, ticks])
                return destination
            roundtrip = resume('roundtrip', checkpoint, 0)
            assert roundtrip.read_bytes() == checkpoint.read_bytes()
            direct = resume('direct', checkpoint, 7)
            first = resume('first', checkpoint, 1)
            split = resume('split', first, 6)
            assert direct.read_bytes() == split.read_bytes()
            report['restart_checks'].append(dict(checkpoint=str(checkpoint.relative_to(ROOT)), sha256=sha(checkpoint), continuation_sha256=sha(direct)))
        # A production-order journey must also match across a process boundary.
        idle, started = a.output/'idle.mnmc', a.output/'started.mnmc'
        run('create', [cli, a.assets, 'create-objectives', idle])
        run('start', [cli, a.assets, 'start', idle, started])
        whole, cut, split = [a.output/(name+'.mnmc') for name in ('pilot-whole', 'pilot-cut', 'pilot-split')]
        test = a.build/'mission-objectives-test'
        run('pilot-whole', [test, a.assets, 'pilot', started, whole, 1024])
        run('pilot-cut', [test, a.assets, 'pilot', started, cut, 24])
        run('pilot-split', [test, a.assets, 'pilot', cut, split, 1000])
        assert whole.read_bytes() == split.read_bytes()
        state = json.loads(run('pilot-inspect', [cli, a.assets, 'inspect', whole]))
        assert state['region'] == 'arena-finale' and state['last_result']['outcome'] == 1 and state['cure_available']
        # Saved v3 staged mission plus retained terrain and point profiles retain exact bytes.
        legacy = [ROOT/'working/tests/staged-mission-bound-20261017-v6/started.mnmc',
                  ROOT/'working/tests/staged-mission-bound-20261017-v6/reward-reopened.mnmc']
        old = ROOT/'working/tests/terrain-campaign-bound-20261016/execution'
        legacy += [sorted((old/name).rglob('*.mnmc'))[0] for name in ('terrain-window', 'legacy-creature-window', 'legacy-map-window')]
        report['legacy'] = {}
        for i, source in enumerate(legacy):
            assert source.is_file(), source
            destination = a.output/f'legacy-{i}.mnmc'
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
