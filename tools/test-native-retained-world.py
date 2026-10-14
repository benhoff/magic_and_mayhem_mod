#!/usr/bin/env python3
"""Source-bound native World image reuse controls and captured-input comparison."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--hardware', action='store_true')
    parser.add_argument('--source', type=Path, required=True, help='Complete owned native producer prefix')
    parser.add_argument('--request', type=Path, required=True)
    parser.add_argument('--game', type=Path, required=True)
    args = parser.parse_args()
    parent = ROOT / 'working/tests/native-retained-world'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('Retained World evidence:', out, flush=True)
    subprocess.run(['python3', ROOT / 'tools/draft-coverage-claims.py',
                    '--behavior', 'NR.native-retained-world-image',
                    '--scenario', 'NR.native-retained-world-image.fixture',
                    '--output', out / 'claims.json'], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    declared = json.loads((out / 'claims.json').read_text())
    report = dict(success=False, scope=__doc__, claims=declared['claims'],
                  sources=declared['sources'], hardware=args.hardware)
    env = dict(os.environ, TMPDIR=str(ROOT / 'working/coverage-hook-tmp'),
               QT_QPA_PLATFORM='xcb', MNM_NATIVE_FRAME_TRACE='1',
               MNM_WORKER_STATUS_CADENCE='1', MNM_WORLD_IMAGE_REUSE='1')
    server = None
    inputs = {}

    def run(label, command):
        with (out / (label + '.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT,
                           check=True, timeout=600)

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        # A later ring slot can be a delta and cannot seed a fresh renderer.
        # Require a complete prefix; native admission still refuses missing history.
        source, request = out / 'source.bin', out / 'request.bin'
        for incoming, destination in [(args.source, source), (args.request, request)]:
            inputs[str(incoming)] = sha(incoming)
            destination.write_bytes(incoming.read_bytes())
        report['source_sha256'], report['request_sha256'] = sha(source), sha(request)
        manifest = args.game.parent / 'input-fingerprints.json'
        inputs[str(manifest)] = sha(manifest)
        for name, digest in json.loads(manifest.read_text()).items():
            path = args.game / name
            assert sha(path) == digest, name
            inputs[str(path)] = digest
        build = ROOT / 'working/build/qt-shell'
        run('configure', ['cmake', '-S', ROOT / 'apps/qt-shell', '-B', build,
                          '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DBUILD_TESTING=ON'])
        run('build', ['cmake', '--build', build, '--target', 'native-retained-world-test',
                      'native-prepared-world-test', 'native-render-worker-test',
                      'gpu-presentation-test', 'mnm-qt-shell', '-j4'])
        if args.hardware:
            spec = importlib.util.spec_from_file_location('native_campaign', ROOT / 'tools/test-native-campaign.py')
            campaign = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(campaign)
            server = campaign.start_native_hardware(out, env)
            report['hardware_driver'] = (out / 'hardware-driver.log').read_text()
        else:
            env.update(LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4')
            for key in ['MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                        '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION']:
                env.pop(key, None)
        for label, command in [
            ('retained', [build / 'world-rolling/native-retained-world-test', source, request, args.game]),
            ('prepared', [build / 'world-rolling/native-prepared-world-test']),
            ('worker', [build / 'native-render-worker-test']),
            ('presentation', [build / 'gpu-presentation-test'])]:
            run(label, command if args.hardware else ['xvfb-run', '-a', *command])
            report[label] = json.loads((out / (label + '.log')).read_text().splitlines()[-1])
        report['success'] = True
    except (OSError, ValueError, AssertionError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if server is not None:
            os.killpg(server.pid, signal.SIGTERM)
            try:
                server.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(server.pid, signal.SIGKILL)
                server.wait(timeout=10)
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_after'] = True
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, integrity_error=str(error))
        report['inputs'] = inputs
        report['inputs_stable'] = all(sha(Path(p)) == h for p, h in inputs.items())
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in declared['sources'].items())
        report['success'] &= report['inputs_stable'] and report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Report:', out / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
