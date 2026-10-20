#!/usr/bin/env python3
"""Prospective, asset-free closeout checks for native world/audio/asset foundations.

Run beneath working-retention.py. Builds are private to each retained run. These
checks establish native policies only; they never start Wine, a game or a device.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile
import xml.etree.ElementTree as ET

from coverage_claims import required_sources

ROOT = Path(__file__).resolve().parents[1]
GROUPS = {
    'world': ['WORLD.owned'],
    'audio': ['NA.voices', 'NA.mixer', 'NA.output'],
    'assets': ['AS.access', 'AS.profile', 'AS.persistence', 'AS.map',
               'AS.terrain_catalog', 'AS.region_recipe', 'AS.sprite',
               'AS.animation', 'AS.cursor', 'AS.pcx'],
}
WORLD_TESTS = ['native-world-lifecycle', 'native-world-wire']
AUDIO_TESTS = ['audio-native-boundaries', 'audio-buffer-lifecycle',
               'audio-native-voice-state', 'audio-stereo-mixer', 'audio-qt-output',
               'audio-output-consumed-timing']
ASSET_TESTS = ['asset-path-resolution', 'asset-file-io', 'persistence-loaders',
               'map-loader-owned-grid', 'terrain-catalog-owned-records',
               'region-recipe-owned', 'sprite-loader', 'animation-loader',
               'cursor-loader', 'pcx-loader', 'asset-byte-comparison',
               'persistence-codec-comparison', 'legacy-asset-comparison',
               'cursor-byte-comparison', 'pcx-byte-comparison']
TARGETS = ['audio-boundaries-test', 'audio-buffers-test', 'audio-voice-state-test',
           'audio-mixer-test', 'audio-output-test', 'audio-output-timeline-test',
           'asset-path-test', 'asset-file-test', 'persistence-loaders-test',
           'map-loader-test', 'terrain-catalog-test', 'region-recipe-test',
           'sprite-loader-test', 'animation-loader-test', 'cursor-loader-test',
           'pcx-loader-test', 'mnm-asset-compare', 'mnm-persistence-inspect',
           'mnm-animation-inspect', 'mnm-sprite-inspect', 'mnm-cursor-inspect',
           'mnm-pcx-inspect']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dependencies(register, ids):
    paths = {'tools/test-engine-foundations.py', 'tools/draft-coverage-claims.py',
             'tools/coverage_claims.py', 'tools/decode-cfg.py', 'tests/test-native-world.py',
             'tests/profile-native-test.cpp', 'tests/native-world-test.cpp',
             'tests/audio-output-timeline-test.cpp', 'tests/audio-buffers-test.cpp',
             'tests/test-asset-compare.py', 'tests/test-persistence-loaders.py',
             'tests/test-legacy-asset-loaders.py', 'tests/test-cursor-loader.py',
             'tests/test-pcx-loader.py'}
    roots = ['assets', 'audio', 'game', 'apps/world-sandbox',
             'reconstruction/pathfinding', 'reconstruction/motion',
             'reconstruction/animation', 'reconstruction/audio']
    for behavior in register['behaviors']:
        if behavior['id'] in ids:
            paths.update(required_sources(behavior))
    for directory in roots:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT / directory).rglob('*')
                     if p.is_file() and (p.suffix in {'.cpp', '.hpp', '.h', '.c', '.inc', '.S', '.cmake'}
                                        or p.name == 'CMakeLists.txt'))
    for name in ['map-loader', 'terrain-catalog', 'region-recipe', 'sprite-loader',
                 'animation-loader', 'cursor-loader', 'pcx-loader', 'persistence-loaders']:
        paths.add('tests/' + name + '-test.cpp')
    # Include fixture headers and Python helpers before the experiment, then check
    # actual compiler dependencies against this prospective declaration.
    pending = list(paths)
    while pending:
        name = pending.pop()
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', (ROOT / name).read_text(), re.M):
            for base in [(ROOT / name).parent, ROOT / 'tests', ROOT / 'protocols/include',
                         *(ROOT / directory for directory in roots)]:
                candidate = base / include
                if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):
                    relative = candidate.resolve().relative_to(ROOT).as_posix()
                    if relative not in paths:
                        paths.add(relative)
                        pending.append(relative)
                    break
    return sorted(paths)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    parent = ROOT / 'working/tests/engine-foundations'
    parent.mkdir(parents=True, exist_ok=True)
    out = args.output or Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    if args.output:
        out.mkdir(parents=True, exist_ok=False)
    print(out, flush=True)
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    ids = [bid for group in GROUPS.values() for bid in group]
    scenarios = [s['id'] for s in register['scenarios']
                 if s['id'].startswith('foundations.') and set(s['behaviors']) & set(ids)]
    spec = importlib.util.spec_from_file_location('foundation_declarations', ROOT / 'tools/draft-coverage-claims.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    declaration = module.declaration(ROOT, register, ids, scenarios)
    declaration['sources'].update({p: sha(ROOT / p) for p in dependencies(register, ids)})
    (out / 'declaration.json').write_text(json.dumps(declaration, indent=2) + '\n')
    report = dict(schema=1, success=False, claims=declaration['claims'], sources=declaration['sources'],
                  scope='Synthetic native ownership/v1 wire, PCM push output, and supported read-only asset policies; no original comparison, physical playback or live replacement.',
                  game_assets_read=False, original_execution=False, device_output=False,
                  live_replacement=False, commands=[], groups={})
    environment = dict(os.environ, QT_QPA_PLATFORM='offscreen')
    environment.pop('QT_FATAL_WARNINGS', None)

    def run(label, command, timeout=180):
        command = [str(arg) for arg in command]
        with (out / (label + '.log')).open('x') as log:
            process = subprocess.run(command, cwd=ROOT, env=environment,
                                     stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=command, exit_code=process.returncode))
        if process.returncode:
            raise RuntimeError(f'{label} failed ({process.returncode}); see {out / (label + ".log")}')

    def ctests(label, build, expected):
        xml = out / (label + '.xml')
        run(label, ['ctest', '--test-dir', build, '--output-on-failure',
                    '--no-tests=error', '-R', '^(' + '|'.join(expected) + ')$', '--output-junit', xml])
        cases = ET.parse(xml).findall('.//testcase')
        if sorted(c.attrib['name'] for c in cases) != sorted(expected) or any(
                c.find('failure') is not None or c.find('skipped') is not None for c in cases):
            raise RuntimeError('Missing, failed or skipped controls: ' + label)
        return expected

    def compiler_closure(build):
        result = set()
        for dependency_file in build.rglob('*.o.d'):
            tokens = shlex.split(dependency_file.read_text().replace('\\\n', ' '))
            for token in tokens[1:]:
                candidate = Path(token)
                if not candidate.is_absolute():
                    candidate = build / candidate
                candidate = candidate.resolve()
                if candidate.is_relative_to(ROOT) and candidate.is_file():
                    name = candidate.relative_to(ROOT).as_posix()
                    if name.startswith('working/'):
                        continue
                    result.add(name)
        if not result or result - set(declaration['sources']):
            raise RuntimeError('Incomplete compiler declaration: ' + repr(sorted(result - set(declaration['sources']))))
        return sorted(result)

    try:
        world = out / 'world-build'
        run('world-configure', ['cmake', '-S', ROOT / 'game', '-B', world, '-DCMAKE_BUILD_TYPE=Debug'])
        run('world-build', ['cmake', '--build', world, '--target', 'native-world-test', 'mnm-world-sandbox', '-j4'])
        report['groups']['world'] = dict(success=True, tests=ctests('world-tests', world, WORLD_TESTS),
                                       compiler_dependencies=compiler_closure(world))
        print('World: lifecycle and independent wire/process controls passed', flush=True)
        audio = out / 'audio-build'
        run('audio-configure', ['cmake', '-S', ROOT / 'audio', '-B', audio, '-DCMAKE_BUILD_TYPE=Debug'])
        run('audio-assets-build', ['cmake', '--build', audio, '--target', *TARGETS, '-j4'], timeout=300)
        closure = compiler_closure(audio)
        report['groups']['audio'] = dict(success=True, tests=ctests('audio-tests', audio, AUDIO_TESTS),
                                       compiler_dependencies=closure)
        print('Audio: PCM, voice, mixer, partial-write and consumed-clock controls passed', flush=True)
        asset_tests = ctests('asset-tests', audio, ASSET_TESTS)
        flags = shlex.split(subprocess.check_output(['pkg-config', '--cflags', '--libs', 'Qt6Core'], text=True))
        profile = out / 'profile-native-test'
        run('profile-build', ['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic',
                              '-g', '-fPIC', '-I' + str(ROOT / 'assets'),
                              ROOT / 'tests/profile-native-test.cpp', ROOT / 'assets/profile.cpp',
                              ROOT / 'assets/path_resolver.cpp', ROOT / 'assets/asset_file.cpp',
                              '-o', profile, *flags])
        run('profile-tests', [profile])
        report['groups']['assets'] = dict(success=True, tests=asset_tests + ['profile-native-policy'],
                                        compiler_dependencies=closure)
        report['binaries'] = {str(p.relative_to(out)): sha(p) for build in (world, audio)
                              for p in build.rglob('*') if p.is_file() and not p.suffix
                              and os.access(p, os.X_OK)}
        report['binaries'][profile.name] = sha(profile)
        print('Assets: supported reader, independent byte/codec and native profile controls passed', flush=True)
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
        raise
    finally:
        report['sources_stable'] = all(sha(ROOT / p) == digest for p, digest in declaration['sources'].items())
        report['changed_sources'] = [p for p, digest in declaration['sources'].items() if sha(ROOT / p) != digest]
        report['success'] = report['success'] and report['sources_stable']
        report['artifacts'] = {p.relative_to(out).as_posix(): sha(p) for p in out.iterdir()
                               if p.is_file() and p.name != 'report.json'}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print('Report: ' + str(out / 'report.json'), flush=True)
    if not report['success']:
        raise RuntimeError('Sources changed during execution')


if __name__ == '__main__':
    main()
