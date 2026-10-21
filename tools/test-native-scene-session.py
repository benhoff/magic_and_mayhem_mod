#!/usr/bin/env python3
"""Execute source-bound native scene ownership, GPU composition and installed CLI checks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract
ROOT = Path(__file__).resolve().parents[1]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main(argv=None):
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--build-root', type=Path)
    parser.add_argument('--reuse-build', action='store_true')
    parser.add_argument('--sources-only', action='store_true')
    parser.add_argument('--sanitized', action='store_true')
    parser.add_argument('--installed-root', type=Path)
    args = parser.parse_args(argv)
    if args.sources_only and args.installed_root:
        parser.error('--installed-root requires full scene checks')
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').touch()
    (out/'tmp').mkdir()
    os.environ['TMPDIR'] = str(out/'tmp')
    build = args.build_root.resolve() if args.build_root else out/'build'
    build.mkdir(parents=True, exist_ok=args.reuse_build)
    report = dict(schema=1, success=False, original_game_executed=False,
                  scope='Owned native source projection, explicit animation clocks/stages and typed light admission; full mode includes persistent software OpenGL composition, four camera views, independent CPU pixel comparison, stale/foreign/failed-input guards and retirement/restart. Optional installed assets exercise immutable MAP/MPS/ANI/TTD/config acquisition and actual executable with a native checkpoint; no original full-scene, hardware or live equivalence claim.')
    sources = {}
    manifest_started = False

    def run(name, command, timeout=900, env=None):
        result = subprocess.run([str(v) for v in command], cwd=ROOT, capture_output=True,
                                text=True, timeout=timeout, env=env)
        (out/(name+'.log')).write_text(result.stdout+result.stderr)
        result.check_returncode()
        return result.stdout

    def binary(target):
        direct=build/target
        if direct.is_file():
            return direct
        candidates=[p for p in build.rglob(target) if p.is_file()]
        if len(candidates)!=1:
            raise ValueError('Ambiguous or missing built executable '+target)
        return candidates[0]

    try:
        if args.sanitized and args.reuse_build:
            cache=(build/'CMakeCache.txt').read_text()
            if '-fsanitize=address,undefined' not in cache or 'clang++' not in cache:
                raise ValueError('Sanitized reuse requires a recorded Clang ASan/UBSan build')
        flags = ['-DCMAKE_CXX_COMPILER=clang++',
                 '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer',
                 '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'] if args.sanitized else []
        if not args.reuse_build:
            run('configure', ['cmake', '-S', ROOT/'apps/scene-session', '-B', build, '-G', 'Ninja',
                              '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_CXX_FLAGS_RELEASE=-O2 -DNDEBUG',
                              '-DBUILD_TESTING=ON', *flags])
        targets = ['save-world-loader-test', 'scene-source-test', 'scene-animation-binding-test', 'scene-entity-lighting-test']
        if not args.sources_only:
            targets += ['scene-session-test', 'native-scene-session']
            if args.installed_root:
                targets += ['scene-installed-test']
            run('generate-platform-moc', ['cmake', '--build', build, '--target', 'mnm_scene_palette_headless_autogen'])
        commands = run('target-commands', ['ninja', '-C', build, '-t', 'commands', *targets])
        paths = set()
        for index, line in enumerate(commands.splitlines()):
            words = shlex.split(line)
            if '-c' not in words:
                continue
            src = words[words.index('-c')+1]
            flags, skip = [], False
            for value in words[1:]:
                if skip:
                    skip = False
                    continue
                if value in ('-o', '-MF', '-MT'):
                    skip = True
                    continue
                if value in ('-c', '-MD', '-MMD', src):
                    continue
                flags.append(value)
            deps = run(f'dependencies-{index}', [words[0], *flags, '-MM', src]).replace('\\\n', ' ').split(':', 1)[1]
            for name in shlex.split(deps):
                path = (build/name).resolve() if not Path(name).is_absolute() else Path(name).resolve()
                if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    paths.add(path.relative_to(ROOT).as_posix())
        register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        ids = ['NR.scene-native-sources', 'NR.scene-animation-views', 'NR.scene-entity-lighting']
        if not args.sources_only:
            ids.append('NR.runtime-scene-composition')
        builds = {b['id']: b for b in register['builds']}
        claims = []
        for identity in ids:
            behavior = next(b for b in register['behaviors'] if b['id'] == identity)
            scenario = next(s for s in register['scenarios'] if s['id'] == 'SCENE.production-'+identity.split('.')[-1]+'-20261020')
            paths.update(required_sources(behavior))
            claims.append(dict(behavior=identity, contract_sha256=behavior_contract(behavior, builds),
                               scenarios={scenario['id']: scenario_contract(scenario)}))
        paths.update(['tools/test-native-scene-session.py', 'tools/test-native-scene-sources.py'])
        sources = {p: sha(ROOT/p) for p in sorted(paths)}
        report.update(sources=sources, claims=claims, sanitized=args.sanitized)
        run('build', ['cmake', '--build', build, '--target', *targets, '--parallel', '4'])
        environment = dict(os.environ, QT_QPA_PLATFORM='mnm-headless-egl',
                           QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'), MNM_PROFILE_EGL_VENDOR='Mesa',
                           LIBGL_ALWAYS_SOFTWARE='1', QT_FORCE_STDERR_LOGGING='1')
        if args.sanitized:
            # Qt/Mesa process lifetime caches are external; bounds/UB remain fatal.
            environment['ASAN_OPTIONS'] = 'detect_leaks=0:halt_on_error=1'
        report['results'] = {}
        for target in targets:
            if target in ('native-scene-session', 'scene-installed-test'):
                continue
            raw = run(target, [binary(target)], env=environment)
            result = ({'success': 'world ownership, ranges, truncation, budgets and envelope checks passed' in raw}
                      if target == 'save-world-loader-test' else json.loads(raw.strip().splitlines()[-1]))
            if result.get('success') is not True:
                raise ValueError(target+' failed native assertions')
            report['results'][target] = result
        report['binaries'] = {t: sha(binary(t)) for t in targets}
        if args.installed_root:
            manifest_started = True
            run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
            installed = out/'installed'
            installed.mkdir()
            root = args.installed_root.resolve()
            raw = run('installed', [build/'scene-installed-test', root, installed], env=environment)
            report['installed'] = json.loads(raw.strip().splitlines()[-1])
            if report['installed'].get('success') is not True:
                raise ValueError('Installed scene assertions failed')
            inputs = ['Realms/Celtic/Forest/CFsec01.map', 'Realms/Celtic/Forest/CFsec01.mps',
                      'Realms/Celtic/Forest/Terrain.spr', 'Realms/Celtic/Forest/terrain.ani',
                      'Realms/Celtic/Forest/Terrain.ttd', 'Creatures/Crocodile.spr', 'Creatures/crocodile.ani',
                      'CFG/Encrypted/effects.cfg', 'CFG/Encrypted/chaos.cfg', 'CFG/prefs.cfg']
            report['installed_sources'] = {p: sha(root/p) for p in inputs}
            raw = run('executable', [build/'native-scene-session', root, installed/'scene.json',
                                     installed/'world.mnw', installed/'executable.png'], env=environment)
            report['executable'] = json.loads(raw.strip().splitlines()[-1])
            if report['executable'].get('success') is not True:
                raise ValueError('Native scene executable failed')
            if sha(installed/'installed.png') != sha(installed/'executable.png'):
                raise ValueError('Executable and application-owner scene images differ')
            report['installed_sources_stable'] = all(sha(root/p) == h for p, h in report['installed_sources'].items())
            if not report['installed_sources_stable']:
                raise ValueError('Installed source inputs changed')
            report['installed_artifacts'] = {p.name: sha(p) for p in installed.iterdir() if p.is_file()}
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        if manifest_started:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
                report['original_manifest_bookends'] = True
            except Exception as error:
                report.update(success=False, original_manifest_bookends=False, manifest_error=str(error))
        report['sources_stable'] = bool(sources) and all(sha(ROOT/p) == h for p, h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps(dict(success=report['success'], results=report.get('results'),
                              installed=report.get('installed'), sources=len(sources), error=report.get('error'))))
    return 0 if report['success'] else 1
if __name__ == '__main__':
    raise SystemExit(main())
