#!/usr/bin/env python3
"""Build/validate native resource ownership with optional installed Redcap smoke."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
OWN_SOURCES = [
    'assets/resources/CMakeLists.txt', 'assets/resources/resource_manager.cpp',
    'assets/resources/resource_manager.hpp', 'renderer/resources/CMakeLists.txt',
    'renderer/resources/resource_cache.cpp', 'renderer/resources/resource_cache.hpp',
    'renderer/resources/preview_main.cpp', 'tests/resource-fixtures.hpp',
    'tests/resource-manager-test.cpp', 'tests/resource-cache-test.cpp',
    'tests/test-resource-preview.py', 'tools/test-native-resources.py',
]
DEPENDENCIES = [
    'assets/CMakeLists.txt', 'assets/path_resolver.cpp', 'assets/path_resolver.hpp',
    'assets/asset_file.cpp', 'assets/asset_file.hpp', 'assets/sprite_loader.cpp',
    'assets/sprite_loader.hpp', 'assets/sprite_frame_decoder.hpp', 'assets/animation.cpp',
    'assets/animation_decode.cpp', 'assets/animation.hpp', 'assets/terrain_catalog.cpp',
    'assets/terrain_catalog.hpp', 'assets/bmp.cpp', 'assets/bmp.hpp', 'assets/pcx.cpp',
    'assets/pcx.hpp', 'assets/jpeg.cpp', 'assets/jpeg.hpp',
    'renderer/sprites/CMakeLists.txt', 'renderer/sprites/sprite.cpp', 'renderer/sprites/sprite.hpp',
    'renderer/CMakeLists.txt', 'renderer/blit.cpp', 'renderer/blit.hpp',
]
TARGETS = ['resource-manager-test', 'resource-cache-test', 'mnm-resource-preview',
           'sprite-render-test', 'sprite-loader-test', 'animation-loader-test',
           'terrain-catalog-test', 'bmp-loader-test', 'pcx-loader-test', 'jpeg-loader-test',
           'asset-path-test', 'asset-file-test', 'render-blit-test', 'render-surfaces-test']
TESTS = '^(native-resource-(manager|cache|preview)|sprite-loader|animation-loader|terrain-catalog-owned-records|bmp-loader|pcx-loader|jpeg-loader|asset-path-resolution|asset-file-io|opengl-sprite-frames|opengl-native-blits|opengl-persistent-surfaces)$'


def hashes(paths):
    return {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, default=ROOT/'working/build/resources')
    parser.add_argument('--root', type=Path, help='Optional installed root; verifies immutable originals before/after')
    args = parser.parse_args()
    parent = ROOT/'working/tests/native-resources'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    sources = hashes(OWN_SOURCES + DEPENDENCIES)
    report = {'schema': 1, 'success': False, 'scope': 'Native ownership/cache policy; synthetic independent pixels and optional installed recipe smoke, no original equivalence or live replacement',
              'sources': sources, 'steps': [], 'installed': None}

    def run(name, command, env=None):
        with (output/(name+'.log')).open('x') as log:
            result = subprocess.run([str(c) for c in command], cwd=ROOT, stdout=log,
                                    stderr=subprocess.STDOUT, env=env, timeout=300)
        report['steps'].append({'name': name, 'command': [str(c) for c in command], 'exit_code': result.returncode})
        if result.returncode:
            raise RuntimeError(name+' failed; see '+str(output/(name+'.log')))

    failure = None
    try:
        if args.root:
            run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        run('configure', ['cmake', '-S', ROOT/'renderer/resources', '-B', args.build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', args.build, '--target', *TARGETS, '-j4'])
        run('tests', ['ctest', '--test-dir', args.build, '-R', TESTS, '--output-on-failure'])
        binaries = [args.build/'resources/resource-manager-test', args.build/'resource-cache-test', args.build/'mnm-resource-preview']
        report['binaries'] = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in binaries}
        if args.root:
            inputs = [args.root/'Creatures/RedCap.spr', args.root/'Creatures/redcap.ani']
            before = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
            env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
            png = output/'redcap.png'
            run('installed', ['xvfb-run', '-a', args.build/'mnm-resource-preview', '--root', args.root,
                             '--kind', 'creature', '--id', '10', '--image', 'Creatures/RedCap.spr',
                             '--animation', 'Creatures/redcap.ani', '--frame', '0', '--output', png], env)
            result = json.loads((output/'installed.log').read_text())
            assert result['ok'] and result['id'] == 'creature:10' and result['surfaces'] == result['decoded_bytes'] == 0
            after = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
            assert before == after, 'Installed inputs changed'
            report['installed'] = {'inputs': before, 'stable': True, 'result': result, 'png': str(png), 'png_sha256': hashlib.sha256(png.read_bytes()).hexdigest()}
        report['sources_stable'] = hashes(OWN_SOURCES + DEPENDENCIES) == sources
        assert report['sources_stable'], 'Sources changed during validation; retain result and rerun'
        report['success'] = True
    except Exception as error:
        failure = str(error)
        report['error'] = failure
    finally:
        if args.root:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            except Exception as error:
                failure = str(error)
                report['success'] = False
                report['original_after_error'] = failure
        with (output/'report.json').open('x') as file:
            json.dump(report, file, indent=2)
            file.write('\n')
    print(output/'report.json')
    if failure:
        print(failure)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
