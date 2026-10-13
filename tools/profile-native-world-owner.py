#!/usr/bin/env python3
"""Bounded same-input TITAN World-owner phase profiling; no live FPS claim."""
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
    parser.add_argument('--corpus', type=Path, required=True)
    parser.add_argument('--game', type=Path, required=True)
    parser.add_argument('--worlds', type=int, default=128, choices=range(64, 513))
    args = parser.parse_args()
    parent = ROOT / 'working/tests/native-world-owner'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('World owner evidence:', out, flush=True)
    subprocess.run(['python3', ROOT / 'tools/draft-coverage-claims.py',
                    '--behavior', 'NR.native-world-owner-costs',
                    '--scenario', 'NR.native-world-owner-costs.offline',
                    '--output', out / 'claims.json'], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    declared = json.loads((out / 'claims.json').read_text())
    report = dict(success=False, scope=__doc__, sources=declared['sources'],
                  claims=declared['claims'], live_gameplay_executed=False)
    env = dict(os.environ, TMPDIR=str(ROOT / 'working/coverage-hook-tmp'),
               QT_QPA_PLATFORM='xcb', MNM_PROFILE_PUBLIC_BACKEND='1',
               MNM_WORLD_IMAGE_REUSE='1', MNM_PROFILE_CHECKPOINTS='1')
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
        build = ROOT / 'working/build/native-world-owner-profile'
        run('configure', ['cmake', '-S', ROOT / 'tools/native-render-profile', '-B', build,
                          '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_CXX_FLAGS_RELEASE=-O2 -DNDEBUG -g'])
        run('build', ['cmake', '--build', build, '--target', 'mnm-resident-profile', '-j4'])
        report['binary_sha256'] = sha(build / 'mnm-resident-profile')
        spec = importlib.util.spec_from_file_location('native_campaign', ROOT / 'tools/test-native-campaign.py')
        campaign = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(campaign)
        server = campaign.start_native_hardware(out, env)
        report['hardware_driver'] = (out / 'hardware-driver.log').read_text()
        phases = {}
        for mode, enabled in [('control', '0'), ('profile', '1')]:
            env['MNM_WORLD_OWNER_PROFILE'] = enabled
            destination = out / (mode + '.json')
            run(mode, [build / 'mnm-resident-profile', corpus, args.game,
                       'throughput', '1', destination, str(args.worlds), '0'])
            result = json.loads(destination.read_text())
            rows = result['runs'][0]['samples']
            assert len(rows) == args.worlds
            assert all(row['owner_costs']['enabled'] == (enabled == '1') for row in rows)
            phases[mode] = result
        control = phases['control']['runs'][0]
        profile = phases['profile']['runs'][0]
        assert [r['native_words_sha256'] for r in control['samples']] == [r['native_words_sha256'] for r in profile['samples']]
        assert control['final_native_words_sha256'] == profile['final_native_words_sha256']
        warm = [row for row in profile['samples'] if int(row['generation']) >= 64]
        keys = [k for k, v in warm[0]['owner_costs'].items() if isinstance(v, (int, float)) and k != 'enabled']
        report['warm_phase_mean_ms'] = {k: statistics.fmean(row['owner_costs'][k] for row in warm) for k in keys}
        report['warm_producer_opcode_mean_ms'] = [statistics.fmean(row['owner_costs']['producer_opcode_ms'][op] for row in warm) for op in range(28)]
        report['warm_existing_mean_ms'] = {k: statistics.fmean(row[k] for row in warm) for k in ['native_consume_ms', 'native_cpu_producer_ms', 'native_prepare_ms', 'native_submit_cpu_ms']}
        report.update(success=True, worlds=args.worlds, full_frame_pixel_comparisons=args.worlds,
                      ordinary_readbacks=0, raw_reports={mode: dict(path=str(out / (mode + '.json')), sha256=sha(out / (mode + '.json'))) for mode in phases})
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
