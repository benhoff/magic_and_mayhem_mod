#!/usr/bin/env python3
"""Source-bound installed creature rules in shared native battles."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--build-root', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--assets', type=Path, default=ROOT/'working/game-nocd')
    p.add_argument('--window', action='store_true')
    a = p.parse_args()
    a.output = a.output.resolve()
    a.output.mkdir(parents=True, exist_ok=False)
    a.assets = a.assets.resolve()
    a.build_root = a.build_root.resolve()
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    assert all(sha(ROOT/k) == v for k, v in sources.items())
    report = dict(success=False, claims=declaration['claims'], source_sha256=sources,
                  scope='Native same-installed-profile type17 creature rules, shared health and generation-bound actors alongside Cure/Sphere; saved RNG/ANI/stun/target continuation, 2x2 standing/edge admission over native point navigation. Wizards excluded from type17 melee; authored scheduling and wizard AI, no complete original AI/movement/encounter/live replacement.',
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  qt_version=subprocess.check_output(['pkg-config', '--modversion', 'Qt6Core'], text=True).strip(), commands=[])

    def run(name, argv, env=None):
        argv = list(map(str, argv))
        report['commands'].append(argv)
        result = subprocess.run(argv, cwd=ROOT, env=env, capture_output=True,
                                text=True, timeout=600)
        (a.output/(name+'.stdout')).write_text(result.stdout)
        (a.output/(name+'.stderr')).write_text(result.stderr)
        assert result.returncode == 0, (name, result.stdout[-3000:], result.stderr[-3000:])
        assert 'runtime error:' not in result.stderr and 'ERROR: AddressSanitizer' not in result.stderr
        return result.stdout

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    inputs = [a.assets/k for k in ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani', 'CFG/Encrypted/effectani.cfg']]
    inputs += sorted(p for p in (a.assets/'Sprites').iterdir() if re.fullmatch(r'effects\d+\.ani', p.name, re.I))
    report['installed_input_sha256'] = {p.relative_to(a.assets).as_posix(): sha(p) for p in inputs}
    try:
        for name, flags in [('normal', []), ('sanitized', ['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = a.build_root/name
            run(name+'-configure', ['cmake', '-S', 'apps/skirmish', '-B', build, '-DCMAKE_BUILD_TYPE=Debug', *flags])
            targets = [] if name == 'normal' else ['--target', 'creature-battle-test', 'sphere-battle-test', 'cure-battle-test', 'battle-effects-test', 'skirmish-test', 'native-health-rules-test', 'campaign-arena-test']
            run(name+'-build', ['cmake', '--build', build, '-j4', *targets])
            env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
            report[name] = json.loads(run(name+'-creatures', [build/'creature-battle-test', a.assets], env))
            assert report[name]['success']
            filters = ['-R', 'native-(sphere-battle|battle-effects|cure-battle|skirmish-contract|health-rules-contract|campaign-arena-contract)$'] if name == 'sanitized' else []
            result = run(name+'-ctest', ['ctest', '--test-dir', build, '--output-on-failure', *filters], env)
            match = re.search(r'100% tests passed[^\n]*out of (\d+)', result)
            assert match
            report[name+'_ctests'] = int(match[1])
            report[name+'_installed_cure'] = json.loads(run(name+'-installed-cure', [build/'cure-battle-test', 'installed', a.assets], env))
            assert report[name+'_installed_cure']['success']
        assert report['normal'] == report['sanitized']
        if a.window:
            run('window', ['python3', 'tests/test-creature-battle-window.py', a.build_root/'normal/campaign-window-test', a.build_root/'normal/mnm-campaign-cli', a.build_root/'normal/creature-battle-test', '--assets', a.assets, '--output', a.output/'window'])
            report['window'] = json.loads((a.output/'window/report.json').read_text())
            assert report['window']['success']
        report['binaries'] = {name: sha(a.build_root/name/'creature-battle-test') for name in ['normal', 'sanitized']}
        report['success'] = True
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT/k) == v for k, v in sources.items())
        report['installed_inputs_unchanged'] = all(sha(a.assets/k) == v for k, v in report['installed_input_sha256'].items())
        (a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    assert report['sources_unchanged'] and report['installed_inputs_unchanged']
    print(json.dumps(dict(success=True, normal=report['normal'], sanitized=True, window=a.window)))


if __name__ == '__main__':
    main()
