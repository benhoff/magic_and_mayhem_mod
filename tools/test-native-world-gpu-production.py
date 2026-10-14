#!/usr/bin/env python3
"""Separate native World GPU owner TITAN controls; no live FPS claim."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import statistics
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as incoming:
        for block in iter(lambda: incoming.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--display-while-pending', action='store_true')
    parser.add_argument('--hitch-capture', action='store_true')
    parser.add_argument('--corpus', type=Path, required=True)
    parser.add_argument('--game', type=Path, required=True)
    parser.add_argument('--worlds', type=int, default=128, choices=range(64, 513))
    args = parser.parse_args()
    parent = ROOT / 'working/tests/native-world-gpu-production'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('World GPU production evidence:', out, flush=True)
    claims_command=['python3', ROOT / 'tools/draft-coverage-claims.py',
                    '--behavior', 'HOST.native-world-gpu-production',
                    '--scenario', 'HOST.native-world-gpu-production.controls',
                    '--output', out / 'claims.json']
    if args.hitch_capture:claims_command.extend(['--behavior','HOST.native-hitch-capture','--scenario','HOST.native-hitch-capture.controls','--behavior','HOST.native-gui-delivery-diagnostics','--scenario','HOST.native-gui-delivery-diagnostics.controls'])
    if args.display_while_pending:claims_command.extend(['--behavior','HOST.pending-import-display','--scenario','HOST.pending-import-display.controls'])
    subprocess.run(claims_command,cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
    declared = json.loads((out / 'claims.json').read_text())
    report = dict(success=False, scope=__doc__, sources=declared['sources'],
                  claims=declared['claims'], live_gameplay_executed=False)
    env = dict(os.environ, TMPDIR=str(ROOT / 'working/coverage-hook-tmp'),
               QT_QPA_PLATFORM='xcb', MNM_PROFILE_PUBLIC_BACKEND='1',
               MNM_NATIVE_FRAME_TRACE='1', MNM_WORKER_STATUS_CADENCE='1',
               MNM_WORLD_IMAGE_REUSE='1', MNM_PROFILE_CHECKPOINTS='1',
               MNM_ASYNC_WORLD_GPU='0', MNM_DISPLAY_WHILE_PENDING='0', MNM_NATIVE_HITCH_CAPTURE='1' if args.hitch_capture else '0')
    server = None
    inputs = {}

    def run(label, command):
        with (out / (label + '.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT,
                           check=True, timeout=600)

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        inputs[str(args.corpus)] = sha(args.corpus)
        # This corpus has independently verified native source/return provenance.
        # Admit a contiguous bounded prefix, never a synthetic continuation.
        corpus = out / 'prefix.bin'
        with args.corpus.open('rb') as incoming, corpus.open('xb') as outgoing:
            assert incoming.read(8) == b'MNMPROF1'
            outgoing.write(b'MNMPROF1')
            for generation in range(1, args.worlds + 1):
                header = incoming.read(16)
                g, source, request = struct.unpack('<QII', header)
                assert g == generation and 16 <= source <= 16777216 and 64 <= request <= 197184
                payload = incoming.read(source + request)
                assert len(payload) == source + request
                outgoing.write(header)
                outgoing.write(payload)
        report['prefix_sha256'] = sha(corpus)
        manifest = args.game.parent / 'input-fingerprints.json'
        inputs[str(manifest)] = sha(manifest)
        for name, digest in json.loads(manifest.read_text()).items():
            path = args.game / name
            assert sha(path) == digest, name
            inputs[str(path)] = digest
        build = ROOT / 'working/build/qt-shell'
        run('configure', ['cmake', '-S', ROOT / 'apps/qt-shell', '-B', build,
                          '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DBUILD_TESTING=ON'])
        run('build', ['cmake', '--build', build, '--target', 'gpu-production-queue-test',
                      'async-world-replay-test', 'native-render-worker-test',
                      'gpu-presentation-test', 'menu-resident-quit-test', 'native-hitch-capture-test', 'pending-world-display-test', 'mnm-qt-shell', '-j4'])
        spec = importlib.util.spec_from_file_location('native_campaign', ROOT / 'tools/test-native-campaign.py')
        campaign = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(campaign)
        server = campaign.start_native_hardware(out, env)
        report['hardware_driver'] = (out / 'hardware-driver.log').read_text()
        commands = [
            ('queue', [build / 'renderer/gpu-production-queue-test']),
            ('captured', [build / 'world-rolling/async-world-replay-test', corpus, args.game]),
            ('worker', [build / 'native-render-worker-test']),
            ('retirement', [build / 'menu-resident-quit-test']),
            ('presentation', [build / 'gpu-presentation-test'])]
        if args.display_while_pending:commands.append(('pending', [build / 'pending-world-display-test']))
        if args.hitch_capture:commands.append(('hitch', [build / 'native-hitch-capture-test']))
        for label, command in commands:
            env['MNM_WORLD_IMAGE_REUSE'] = '0' if label == 'retirement' else '1'
            run(label, command)
            report[label] = json.loads((out / (label + '.log')).read_text().splitlines()[-1])
            assert report[label]['success']
        env['MNM_ASYNC_WORLD_GPU'] = '1'
        run('async-worker', [build / 'native-render-worker-test'])
        report['async_worker'] = json.loads((out / 'async-worker.log').read_text().splitlines()[-1])
        assert report['async_worker']['success']
        report.update(success=True, worlds=args.worlds,
                      full_frame_pixel_comparisons=report['captured']['full_native_word_comparisons'],
                      ordinary_producer_readbacks=0)
    except (OSError, ValueError, AssertionError, KeyError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if server is not None:
            os.killpg(server.pid, signal.SIGTERM)
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
