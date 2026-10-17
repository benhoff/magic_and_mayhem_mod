#!/usr/bin/env python3
"""Source-bound installed terrain, footprint, combat and restart validation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
MAIN = Path(subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', '--path-format=absolute', '--git-common-dir'], text=True).strip()).parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--build-root', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--assets', type=Path, required=True)
    p.add_argument('--window', action='store_true', help='Validate the actual Qt encounter and legacy map/arena windows')
    a = p.parse_args()
    a.output = a.output.resolve()
    a.output.mkdir(parents=True, exist_ok=False)
    a.assets = a.assets.resolve()
    a.build_root = a.build_root.resolve()
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    assert all(sha(ROOT/k) == v for k, v in sources.items())
    report = dict(success=False, claims=declaration['claims'], source_sha256=sources,
                  scope='Native sealed12x12 Plains Crocodile2x2x2 encounter: shared terrain/occupancy/health, melee, Cure and TTD-aware Sphere, owned campaign/profile/RNG/ANI continuation, authored placement and cadence. Optional real Qt controls and fresh-window/CLI identical campaign bytes, actual retreat/pursuit and fresh player/AI summons. Full map entities, original AI/search/speed, wizard species, physical audio and live replacement excluded.',
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  qt_version=subprocess.check_output(['pkg-config', '--modversion', 'Qt6Core'], text=True).strip(), commands=[])

    def run(name, argv, env=None):
        argv = list(map(str, argv))
        report['commands'].append(argv)
        result = subprocess.run(argv, cwd=ROOT, env=env, capture_output=True, text=True, timeout=600)
        (a.output/(name+'.stdout')).write_text(result.stdout)
        (a.output/(name+'.stderr')).write_text(result.stderr)
        assert result.returncode == 0, (name, result.stdout[-2000:], result.stderr[-2000:])
        assert 'runtime error:' not in result.stderr and 'ERROR: AddressSanitizer' not in result.stderr
        return result.stdout

    run('original-before', [MAIN/'tools/original-manifest.sh', 'verify'])
    names = ['Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd', 'Realms/Celtic/Plains/Terrain.spr', 'CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani', 'CFG/Encrypted/effectani.cfg']
    inputs = [a.assets/k for k in names]
    inputs += sorted(p for p in (a.assets/'Sprites').iterdir() if re.fullmatch(r'effects\d+\.ani', p.name, re.I))
    report['installed_input_sha256'] = {p.relative_to(a.assets).as_posix(): sha(p) for p in inputs}
    try:
        for name, flags in [('normal', []), ('sanitized', ['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = a.build_root/name
            run(name+'-configure', ['cmake', '-S', 'apps/skirmish', '-B', build, '-DCMAKE_BUILD_TYPE=Debug', *flags])
            targets = [] if name == 'normal' else ['--target', 'terrain-battle-test', 'terrain-creature-test', 'terrain-campaign-test', 'map-sphere-test', 'map-cure-test', 'creature-battle-test', 'sphere-battle-test', 'cure-battle-test', 'battle-effects-test', 'skirmish-test', 'native-health-rules-test', 'campaign-arena-test']
            run(name+'-build', ['cmake', '--build', build, '-j4', *targets])
            if name == 'normal':
                dependencies = set()
                for dep in build.rglob('*.o.d'):
                    for token in dep.read_text().replace('\\\n', ' ').split()[1:]:
                        path = Path(token)
                        path = (path if path.is_absolute() else build/path).resolve()
                        if path.is_file() and path.is_relative_to(ROOT):
                            dependencies.add(path.relative_to(ROOT).as_posix())
                report['compiler_dependencies'] = sorted(dependencies)
                report['undeclared_compiler_dependencies'] = sorted(dependencies-set(sources))
                assert not report['undeclared_compiler_dependencies'], report['undeclared_compiler_dependencies']
            env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
            report[name] = {}
            for target in ['terrain-battle-test', 'terrain-creature-test', 'terrain-campaign-test', 'map-sphere-test', 'map-cure-test', 'creature-battle-test']:
                result = json.loads(run(name+'-'+target, [build/target, a.assets], env))
                assert result['success']
                report[name][target] = result
            assert report[name]['terrain-creature-test']['continuations'] == 100
            assert report[name]['terrain-creature-test']['melee_hits'] > 0
            assert report[name]['terrain-creature-test']['moves'] > 0
            assert report[name]['terrain-campaign-test']['campaign_continuations'] == 20
            assert report[name]['terrain-campaign-test']['terrain_retired']
            assert report[name]['terrain-campaign-test']['bad_footprint_refused']
            assert report[name]['terrain-campaign-test']['next_retry']
            filters = ['-R', 'native-(sphere-battle|battle-effects|cure-battle|skirmish-contract|health-rules-contract|campaign-arena-contract)$'] if name == 'sanitized' else []
            result = run(name+'-ctest', ['ctest', '--test-dir', build, '--output-on-failure', *filters], env)
            match = re.search(r'100% tests passed[^\n]*out of (\d+)', result)
            assert match
            report[name+'_ctests'] = int(match[1])
            report[name+'_installed_cure'] = json.loads(run(name+'-installed-cure', [build/'cure-battle-test', 'installed', a.assets], env))
            assert report[name+'_installed_cure']['success']
        assert report['normal'] == report['sanitized']
        normal = a.build_root/'normal'
        run('map-presentation', [normal/'map-presentation-test', a.assets])
        if a.window:
            report['windows'] = {}
            jobs = [('terrain', 'tests/test-terrain-battle-window.py', [normal/'campaign-window-test', normal/'mnm-map-campaign-cli', normal/'mnm-campaign-cli', normal/'terrain-campaign-test', '--assets', a.assets]),
                    ('legacy-creature', 'tests/test-creature-battle-window.py', [normal/'campaign-window-test', normal/'mnm-campaign-cli', normal/'creature-battle-test', '--assets', a.assets]),
                    ('legacy-map', 'tests/test-map-sphere-window.py', [normal/'campaign-window-test', normal/'mnm-map-campaign-cli', normal/'map-sphere-test', a.assets])]
            for name, script, args in jobs:
                out = a.output/(name+'-window')
                run(name+'-window', ['python3', ROOT/script, *args, '--output', out])
                details = json.loads((out/'report.json').read_text())
                assert details['success']
                report['windows'][name] = details
            out = a.output/'profile-switch-window'
            run('profile-switch-window', ['python3', ROOT/'tests/test-campaign-profile-switch.py', normal/'campaign-window-test', normal/'mnm-campaign-cli', normal/'mnm-map-campaign-cli', normal/'creature-battle-test', a.assets, out])
            details = json.loads((out/'report.json').read_text())
            assert details['success']
            report['windows']['profile-switch'] = details
        # Compiler-emitted dependency files verify that declaration covers all
        # repository source/header bytes actually consumed by this build.
        dependencies = set()
        for dep in normal.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n', ' ').split()[1:]:
                path = Path(token)
                path = (path if path.is_absolute() else normal/path).resolve()
                if path.is_file() and path.is_relative_to(ROOT):
                    dependencies.add(path.relative_to(ROOT).as_posix())
        report['compiler_dependencies'] = sorted(dependencies)
        report['undeclared_compiler_dependencies'] = sorted(dependencies-set(sources))
        assert not report['undeclared_compiler_dependencies'], report['undeclared_compiler_dependencies']
        report['binaries'] = {name: {target: sha(a.build_root/name/target) for target in ['terrain-battle-test', 'terrain-creature-test', 'terrain-campaign-test', 'creature-battle-test']} for name in ['normal', 'sanitized']}
        report['success'] = True
    finally:
        run('original-after', [MAIN/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT/k) == v for k, v in sources.items())
        report['installed_inputs_unchanged'] = all(sha(a.assets/k) == v for k, v in report['installed_input_sha256'].items())
        (a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    assert report['sources_unchanged'] and report['installed_inputs_unchanged']
    print(json.dumps(dict(success=True, result=report['normal'])))


if __name__ == '__main__':
    main()
