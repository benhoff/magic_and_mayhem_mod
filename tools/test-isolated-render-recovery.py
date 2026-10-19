#!/usr/bin/env python3
"""Execute isolated native rendering recovery and ordinary presentation controls.

Run under an X display (for example xvfb-run). No original artifacts are consumed.
The caller supplies the repository storage guard for the two bounded builds.
"""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--renderer-build', type=Path, required=True)
    parser.add_argument('--qt-build', type=Path, required=True)
    parser.add_argument('--drawing-build', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(exist_ok=False)
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    def differences():
        return [p for p, h in sources.items()
                if not (ROOT / p).is_file() or hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != h]
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'],
                  original_game_executed=False, physical_hardware_reset=False, commands=[])
    environment = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
                       MNM_NATIVE_DEVICE_RECOVERY='0', MNM_RENDER_SERVICE='0',
                       MNM_NATIVE_FRAME_TRACE='1')
    def run(name, command, extra=None, timeout=120):
        with (out / (name + '.log')).open('x') as log:
            result = subprocess.run(list(map(str, command)), cwd=ROOT,
                                    env=environment | (extra or {}), stdout=log,
                                    stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(name=name, exit_code=result.returncode))
        if result.returncode:
            raise RuntimeError(name + ': exit ' + str(result.returncode))
    renderer = args.renderer_build.resolve()
    qt = args.qt_build.resolve()
    try:
        if differences():
            raise ValueError('Prospective inputs changed')
        run('configure-renderer', ['cmake', '-S', ROOT / 'renderer', '-B', renderer, '-DBUILD_TESTING=ON'])
        run('configure-qt', ['cmake', '-S', ROOT / 'apps/qt-shell', '-B', qt, '-DBUILD_TESTING=ON'])
        run('build-renderer', ['cmake', '--build', renderer, '--target', 'isolated-render-recovery-test',
                              'render-context-recovery-test', 'gpu-production-queue-test', '-j2'], timeout=600)
        run('build-qt', ['cmake', '--build', qt, '--target', 'mnm-qt-shell', 'raster-recovery-viewport-test',
                        'native-render-worker-test', 'gpu-presentation-test', 'viewport-presentation-test',
                        'qt-input-test', 'mnm-world-continuous', 'mnm-world-rolling-live',
                        'mnm-canvas-producers-live', '-j2'], timeout=900)
        run('configure-drawing', ['cmake', '-S', ROOT / 'apps/drawing-session', '-B', args.drawing_build.resolve(), '-DBUILD_TESTING=OFF'])
        run('build-drawing', ['cmake', '--build', args.drawing_build.resolve(), '--target', 'mnm-drawing-present', '-j2'], timeout=600)
        run('isolated-faults', [renderer / 'isolated-render-recovery-test'])
        run('platform-recreation', [qt / 'raster-recovery-viewport-test'])
        run('worker-isolated', [qt / 'native-render-worker-test'], {'MNM_NATIVE_DEVICE_RECOVERY': '1'})
        run('worker-ordinary', [qt / 'native-render-worker-test'])
        run('gpu-presentation', [qt / 'gpu-presentation-test'])
        run('viewport-presentation', [qt / 'viewport-presentation-test'])
        run('viewport-raster', [qt / 'viewport-presentation-test'], {'MNM_NATIVE_DEVICE_RECOVERY': '1'})
        run('input', [qt / 'qt-input-test'])
        run('local-checkpoints', [renderer / 'render-context-recovery-test'], {'MNM_RENDER_SERVICE': '1'})
        run('queue-checkpoints', [renderer / 'gpu-production-queue-test'],
            {'MNM_RENDER_SERVICE': '1', 'MNM_NATIVE_DEVICE_RECOVERY': '1'})
        run('queue-ordinary', [renderer / 'gpu-production-queue-test'])
        compiler_inputs = set()
        for build in (renderer, qt, args.drawing_build.resolve()):
            for dependency in build.rglob('*.o.d'):
                for name in re.findall(r'/[^\s\\]+', dependency.read_text().replace('\\\n', ' ')):
                    file = Path(name)
                    try:
                        relative = file.resolve().relative_to(ROOT)
                    except ValueError:
                        continue
                    if relative.parts[0] not in ('working', 'original') and file.is_file():
                        compiler_inputs.add(str(relative))
        report['compiler_inputs'] = sorted(compiler_inputs)
        report['undeclared_compiler_inputs'] = sorted(compiler_inputs - sources.keys())
        if report['undeclared_compiler_inputs']:
            raise RuntimeError('Compiler inputs absent from prospective sources')
        report.update(success=True, interrupted_draw_recovery=True, hung_worker_recovery=True,
                      platform_surface_recovery=True, ordinary_path_regressions_passed=True,
                      application_build=True, original_semantics_comparison=False)
    except Exception as error:
        report['error'] = str(error)
    report['source_differences'] = differences()
    report['sources_stable'] = not report['source_differences']
    report['success'] = report['success'] and report['sources_stable']
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
