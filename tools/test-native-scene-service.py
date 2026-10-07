#!/usr/bin/env python3
"""Retain source-bound synthetic checks for the shared native scene service."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DOCUMENTS = [
    'renderer/scenes/README.md', 'assets/resources/README.md',
    'apps/sprite-scene/README.md', 'apps/world-scene/README.md',
    'research/runtime/native-shared-scene-renderer.md',
]
TEST_SOURCES = [
    'tests/resource-manager-test.cpp', 'tests/resource-cache-test.cpp',
    'tests/resource-fixtures.hpp', 'tests/scene-renderer-test.cpp',
    'tests/scene-resource-fixtures.hpp', 'tests/sprite-scene-test.cpp',
    'tests/sprite-scene-queue-test.cpp', 'tests/world-scene-test.cpp',
    'tests/world-picking-test.cpp', 'tests/scene-window-driver.cpp',
    'tests/scene-window-driver.hpp', 'tools/test-native-scene-service.py',
    'tools/test-sprite-scene.py', 'tools/test-animation-layers.py',
    'tools/test-world-scene.py', 'tools/test-native-scene-window.py',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sources():
    paths = set(DOCUMENTS + TEST_SOURCES)
    for directory in ['assets', 'renderer', 'apps/sprite-scene', 'apps/world-scene',
                      'apps/world-sandbox', 'game', 'reconstruction']:
        for path in (ROOT/directory).rglob('*'):
            if path.is_file() and (path.suffix in ('.cpp', '.hpp') or path.name == 'CMakeLists.txt'):
                paths.add(str(path.relative_to(ROOT)))
    return {path: sha(ROOT/path) for path in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sprite-build', type=Path, default=ROOT/'working/build/sprite-scene')
    parser.add_argument('--world-build', type=Path, default=ROOT/'working/build/world-scene')
    args = parser.parse_args()
    parent = ROOT/'working/tests/native-scene-service'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    report = {'schema': 1, 'success': False, 'sources': sources(), 'steps': [],
              'scope': 'Synthetic native resource/scene ownership and migrated preview contracts; no original equivalence or live replacement'}

    def run(name, command):
        with (output/(name+'.log')).open('x') as log:
            result = subprocess.run([str(c) for c in command], cwd=ROOT,
                                    stdout=log, stderr=subprocess.STDOUT, timeout=300)
        report['steps'].append({'name': name, 'command': [str(c) for c in command],
                                'exit_code': result.returncode})
        if result.returncode:
            raise RuntimeError(name+' failed; see '+str(output/(name+'.log')))

    try:
        for name, build in [('sprite', args.sprite_build), ('world', args.world_build)]:
            run(name+'-configure', ['cmake', '-S', ROOT/('apps/'+name+'-scene'),
                                    '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('sprite-build', ['cmake', '--build', args.sprite_build, '--target',
                            'scene-renderer-test', 'resource-manager-test', 'resource-cache-test',
                            'sprite-scene-test', 'sprite-scene-queue-test', 'mnm-sprite-scene-preview', '-j4'])
        run('world-build', ['cmake', '--build', args.world_build, '--target',
                           'world-scene-test', 'world-picking-test', 'mnm-world-scene-preview',
                           'world-scene-window-test', 'mnm-world-sandbox', 'mnm-map-navigation-export', '-j4'])
        run('sprite-tests', ['ctest', '--test-dir', args.sprite_build, '-R',
                            '^(native-shared-scene-renderer|native-resource-manager|native-resource-cache|opengl-animation-scene|opengl-sprite-queue-scene)$',
                            '--output-on-failure'])
        run('world-tests', ['ctest', '--test-dir', args.world_build, '-R',
                           '^native-world-(scene|picking)$', '--output-on-failure'])
        binaries = [args.sprite_build/'mnm-sprite-scene-preview',
                    args.world_build/'mnm-world-scene-preview',
                    args.world_build/'world-scene-window-test',
                    args.world_build/'world/mnm-world-sandbox',
                    args.world_build/'mnm-map-navigation-export']
        report['binaries'] = {str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p): sha(p) for p in binaries}
        report['tests_passed'] = 7
        report['sources_stable'] = sources() == report['sources']
        if not report['sources_stable']:
            raise RuntimeError('Sources changed during validation; retain result and rerun')
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        with (output/'report.json').open('x') as file:
            json.dump(report, file, indent=2)
            file.write('\n')
    print(output/'report.json')
    if not report['success']:
        print(report['error'])
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
