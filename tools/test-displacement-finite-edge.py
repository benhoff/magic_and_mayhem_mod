#!/usr/bin/env python3
"""Reproduce retained public bottom-edge displacement and validate native policy."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--experiment', type=Path, default=ROOT/'working/experiments/menu-observer/run-r9nz3st2')
    parser.add_argument('--executable', type=Path, default=ROOT/'working/game-nocd/Chaos.exe')
    args = parser.parse_args()
    if not args.experiment.resolve().is_relative_to(ROOT/'working'):
        parser.error('Require an owned retained experiment under working')
    parent = ROOT / 'working/tests/displacement-finite-edge'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    subprocess.run(['python3', ROOT / 'tools/draft-coverage-claims.py',
                    '--behavior', 'NR.displacement-finite-edge',
                    '--scenario', 'displacement-finite-edge-fixture-20261013',
                    '--output', out / 'claims.json'], check=True, cwd=ROOT,
                   stdout=subprocess.DEVNULL)
    claims = json.loads((out / 'claims.json').read_text())
    report = dict(success=False, sources=claims['sources'], claims=claims['claims'],
                  scope=__doc__, original_pixels_used_as_native_inputs=False)
    env = dict(os.environ, TMPDIR=str(ROOT / 'working/coverage-hook-tmp'),
               QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4')
    Path(env['TMPDIR']).mkdir(parents=True,exist_ok=True)
    for key in ['MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION']:
        env.pop(key, None)

    def run(label, command):
        with (out / (label + '.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True,
                           timeout=600)

    inputs = {}
    before = False
    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        before = True
        experiment = args.experiment.resolve()
        channel = experiment / 'world-continuous.bin'
        inputs[str(channel)] = sha(channel)
        raw = channel.read_bytes()
        assert len(raw) == 35869312 and raw[:8] == b'MNMCON05'
        slot = 128 + (len(raw) - 128) // 2
        header = struct.unpack_from('<32I', raw, slot)
        assert header[2:4] == (2354, 0) and 64 <= header[4] <= 16777216
        source = out / 'source.bin'
        source.write_bytes(raw[slot + 128:slot + 128 + header[4]])
        spec = importlib.util.spec_from_file_location(
            'producers', ROOT / 'tools/inspect-canvas-producers.py')
        helper = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helper)
        _, records = helper.decode(source.read_bytes(), False)
        draw, payload = next((r, b) for r, b in records if r[1] == 1592)
        assert draw[2] == 9 and draw[14] == 3 and draw[21] == 0
        assert draw[8:10] == (763, 603)
        owned = out / 'frame-offsets.bin'
        owned.write_bytes(payload)
        report.update(generation=2354, producer_sequence=1592,
                      retained_source_sha256=sha(source), owned_frame_sha256=sha(owned))
        assets = experiment / 'game'
        manifest = experiment / 'input-fingerprints.json'
        inputs[str(manifest)] = sha(manifest)
        for name, digest in json.loads(manifest.read_text()).items():
            assert sha(assets / name) == digest, name
            inputs[str(assets / name)] = digest
        pe = args.executable.resolve()
        inputs[str(pe)] = sha(pe)
        assert inputs[str(pe)] == '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
        reference = out / 'reference'
        run('reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall',
                               '-Wextra', '-Werror', '-fno-pie', '-no-pie',
                               'tests/displacement-linear-reference.cpp', '-o', reference])
        run('original-edge', [reference, pe, owned, out / 'original.json', '--bottom-edge'])
        report['original'] = json.loads((out / 'original.json').read_text())
        build = ROOT / 'working/build/qt-shell'
        run('configure', ['cmake', '-S', ROOT / 'apps/qt-shell', '-B', build,
                          '-DBUILD_TESTING=ON', '-DCMAKE_BUILD_TYPE=RelWithDebInfo'])
        run('build', ['cmake', '--build', build, '--target', 'masked-displacement-test',
                      'scene-renderer-test', 'sprite-atlas-test', '-j4'])
        run('fixture', ['xvfb-run', '-a', build / 'world-rolling/masked-displacement-test',
                        source, assets, 'edge'])
        report['controls'] = json.loads((out / 'fixture.log').read_text().splitlines()[-1])
        for name in ['scene-renderer-test', 'sprite-atlas-test']:
            run(name, ['xvfb-run', '-a', build / 'world-rolling/canvas-producers/scenes' / name])
        report.update(success=True, regressions=2)
    except (OSError, ValueError, StopIteration, AssertionError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if before:
            try:
                run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except (OSError, subprocess.SubprocessError) as error:
                report.update(success=False, manifest_error=str(error))
        report['inputs'] = inputs
        report['inputs_stable'] = all(sha(Path(p)) == h for p, h in inputs.items())
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in claims['sources'].items())
        report['success'] &= report['inputs_stable'] and report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Finite displacement report:', out / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
