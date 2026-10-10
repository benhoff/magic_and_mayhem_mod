#!/usr/bin/env python3
"""Soak saved owned World inputs and seeded scalar/GPU adversarial draws, without gameplay."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import tempfile

from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
BEHAVIOR = 'NR.offline-render-soak'
SCENARIO = 'native-render-offline-soak-20261010'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def distribution(values):
    ordered = sorted(values)
    return {'median': statistics.median(ordered),
            'p95': ordered[math.ceil(len(ordered) * .95) - 1],
            'max': ordered[-1], 'samples': len(ordered)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--corpus', type=Path, required=True,
                        help='Successful same-input World resource-reuse run, including inputs/assets/after report')
    parser.add_argument('--iterations', type=int, default=20)
    parser.add_argument('--seed', type=int, default=20261010)
    parser.add_argument('--rounds', type=int, default=40)
    parser.add_argument('--gl33', action='store_true', help='Disable copy-image to test the GL3.3 snapshot fallback')
    parser.add_argument('--prepared', type=Path, help='Reuse a successful run only after source, binary and compiler dependency fingerprints match')
    parser.add_argument('--median-ms', type=float, default=50)
    parser.add_argument('--p95-ms', type=float, default=100)
    parser.add_argument('--max-ms', type=float, default=250)
    parser.add_argument('--rss-growth-mib', type=float, default=32)
    args = parser.parse_args()
    if not 4 <= args.iterations <= 10000 or not 1 <= args.rounds <= 10000:
        parser.error('Iterations must be4..10000; rounds1..10000')
    if not 0 <= args.seed <= 0xffffffff:
        parser.error('Seed must fit uint32')
    if not 0 < args.median_ms <= args.p95_ms <= args.max_ms or args.rss_growth_mib <= 0:
        parser.error('Require positive ordered latency limits and positive memory-growth limit')
    corpus = args.corpus.resolve()
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    scenario = next(s for s in register['scenarios'] if s['id'] == SCENARIO)
    sources = {name: sha(ROOT / name) for name in sorted(required_sources(behavior))}
    claims = [dict(behavior=BEHAVIOR,
                   contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}),
                   scenarios={SCENARIO: scenario_contract(scenario)})]
    parent = ROOT / 'working/tests/native-render-soak'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(output, flush=True)
    env = {**os.environ, 'QT_QPA_PLATFORM': 'xcb', 'LIBGL_ALWAYS_SOFTWARE': '1',
           'LP_NUM_THREADS': os.environ.get('LP_NUM_THREADS', '8')}
    if args.gl33:
        env.update(MESA_GL_VERSION_OVERRIDE='3.3', MESA_EXTENSION_OVERRIDE='-GL_ARB_copy_image')
    report = dict(success=False, sources=sources, claims=claims,
                  scope=scenario['scope'], settings=vars(args) | {'corpus': str(corpus), 'prepared': str(args.prepared) if args.prepared else None},
                  environment={k: env[k] for k in ('LP_NUM_THREADS', 'LIBGL_ALWAYS_SOFTWARE')},
                  original_pixels_used_as_native_inputs=False)
    inputs = {}
    before_verified = False

    def run(name, command, timeout=600):
        with (output / (name + '.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=timeout)

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        before_verified = True
        historical = json.loads((corpus / 'report.json').read_text())
        assert historical['success'] and historical['inputs_stable']
        assert historical['original_pixels_used_as_native_inputs'] is False
        assert historical['owned_stream_sha256'] == sha(corpus / 'inputs/canvas-producers.bin')
        for name, digest in historical['asset_sources'].items():
            path = (corpus / 'assets' / name).resolve()
            assert path.is_relative_to(corpus / 'assets') and sha(path) == digest, name
            inputs[str(path)] = digest
        for name in ('report.json', 'inputs/canvas-producers.bin', 'inputs/canvas-producers.done', 'after/live-report.json'):
            inputs[str(corpus / name)] = sha(corpus / name)
        expected = json.loads((corpus / 'after/live-report.json').read_text())
        assert expected['success'] and expected['world_queues'] == 32 and expected['world_readbacks'] == 32
        assert expected['original_pixels_used_as_native_inputs'] is False and not expected['original_oracles_read']
        marker = (corpus / 'inputs/canvas-producers.done').read_bytes()
        assert len(marker) == 32 and marker[:8] == b'MNMPDONE'
        import struct
        version, queues, records, error, checkpoints, size = struct.unpack('<6I', marker[8:])
        assert version == 1 and queues == 32 and error == 0
        assert checkpoints == len(expected['checkpoints']) and size == (corpus / 'inputs/canvas-producers.bin').stat().st_size
        encoded = (corpus / 'inputs/canvas-producers.bin').read_bytes()
        offset, actual_records = 64, 0
        while offset < len(encoded):
            extent = struct.unpack_from('<I', encoded, offset)[0]
            assert extent >= 96 and offset + extent <= len(encoded)
            offset += extent
            actual_records += 1
        assert offset == len(encoded) and actual_records == records
        report['closed_input'] = dict(queues=queues, records=records, checkpoints=checkpoints, bytes=size)
        frozen = output / 'source'
        # CMake checks source existence even for preview/test targets that are
        # not built. Freeze those tracked configuration inputs too; compiler
        # dependency closure still must match the explicitly declared contract.
        tracked = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
        configured = {name for name in tracked if name and name.split('/')[0] not in ('original', 'working')
                      and (Path(name).suffix in ('.c', '.cpp', '.h', '.hpp', '.S', '.cmake', '.py')
                           or Path(name).name == 'CMakeLists.txt')}
        configuration_sources = {}
        for name in sorted(configured | set(sources)):
            destination = frozen / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, destination)
            configuration_sources[name] = sha(destination)
        report['frozen_configuration_sources'] = configuration_sources
        prepared = args.prepared.resolve() if args.prepared else None
        if prepared:
            old = json.loads((prepared / 'report.json').read_text())
            assert old['success'] and old['sources'] == sources and old['sources_stable']
            inputs[str(prepared / 'report.json')] = sha(prepared / 'report.json')
            assert sha(prepared / 'build/native-render-soak') == old['binary_sha256']
            for name, value in old['debug_binaries'].items():
                assert sha(prepared / 'debug' / name) == value
            report['prepared_build'] = str(prepared)
        build = prepared / 'build' if prepared else output / 'build'
        dependency_root = prepared / 'source' if prepared else frozen
        if not prepared:
            run('configure', ['cmake', '-S', frozen / 'tests/native-render-soak', '-B', build,
                              '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DBUILD_TESTING=OFF'])
            run('build', ['cmake', '--build', build, '--target', 'native-render-soak', '-j4'])
        compiled = set()
        for dependency in build.rglob('*.o.d'):
            for token in dependency.read_text().replace('\\\n', ' ').split():
                path = Path(token).resolve()
                if path.is_relative_to(dependency_root):
                    name = path.relative_to(dependency_root).as_posix()
                    assert name in sources and sha(path) == sources[name], name
                    compiled.add(name)
        assert compiled
        report['compiled_dependency_review'] = dict(compiled_dependencies=sorted(compiled), undeclared_dependencies=[])
        binary = build / 'native-render-soak'
        report['binary_sha256'] = sha(binary)
        inputs[str(binary)] = sha(binary)
        native_report = output / 'native-report.json'
        run('soak', ['xvfb-run', '-a', binary, corpus / 'inputs/canvas-producers.bin',
                     corpus / 'assets', corpus / 'after/live-report.json', native_report,
                     args.iterations, args.seed, args.rounds], timeout=3600)
        native = json.loads(native_report.read_text())
        report['native'] = native
        assert native['success'] and not native['remaining_surfaces']
        sessions = native['sessions']
        assert len(sessions) == args.iterations
        assert all(s['queues'] == 32 and s['checkpoints'] == checkpoints for s in sessions)
        # First session establishes allocator/driver warm state; each first queue
        # remains a separately measured cold upload rather than a warm-frame sample.
        warm = [f['native_work_ms'] for s in sessions[1:] for f in s['frames'] if f['queue'] > 1]
        cold = [s['frames'][0]['native_work_ms'] for s in sessions]
        latency = distribution(warm)
        tail_rss = [s['rss_bytes'] for s in sessions[1:]]
        growth = max(tail_rss) - tail_rss[0]
        assert all(s['scratch_allocations'] == sessions[0]['scratch_allocations']
                   and s['scratch_pixels'] == sessions[0]['scratch_pixels'] for s in sessions), 'Repeated identical sessions grew scratch storage'
        passed = (latency['median'] <= args.median_ms and latency['p95'] <= args.p95_ms
                  and latency['max'] <= args.max_ms and growth <= args.rss_growth_mib * 1024 * 1024)
        report['performance_budget'] = dict(passed=passed, warm_native_work_ms=latency,
                                            cold_native_work_ms=distribution(cold),
                                            median_limit_ms=args.median_ms, p95_limit_ms=args.p95_ms,
                                            max_limit_ms=args.max_ms, rss_growth_bytes=growth,
                                            rss_growth_limit_bytes=args.rss_growth_mib * 1024 * 1024)
        report['totals'] = dict(world_queues=32 * args.iterations,
                               checkpoint_comparisons=checkpoints * args.iterations,
                               session_teardowns=args.iterations)
        assert passed, 'Latency tail or sustained resident-memory growth exceeds declared budget'
        # Existing atlas/cache eviction and transport refusal fixtures stay in
        # an assertion-enabled build, distinct from optimized timed execution.
        debug = prepared / 'debug' if prepared else output / 'debug'
        if not prepared:
            run('debug-configure', ['cmake', '-S', frozen / 'compat/legacy/canvas-producers', '-B', debug,
                                    '-DCMAKE_BUILD_TYPE=Debug', '-DBUILD_TESTING=ON'])
            run('debug-build', ['cmake', '--build', debug, '--target', 'canvas-world-test',
                               'world-raster-batch-test', 'sprite-atlas-test', '-j4'])
        report['debug_binaries'] = {}
        for name in ('canvas-world-test', 'world-raster-batch-test', 'scenes/resources/sprite-atlas-test'):
            # Renderer test location follows its CMake subdirectory in the shared build.
            candidates = [path for path in debug.rglob(Path(name).name) if path.is_file() and os.access(path, os.X_OK)]
            assert len(candidates) == 1, name
            path = candidates[0]
            report['debug_binaries'][path.relative_to(debug).as_posix()] = sha(path)
            inputs[str(path)] = sha(path)
        debug_compiled = set()
        for dependency in debug.rglob('*.o.d'):
            for token in dependency.read_text().replace('\\\n', ' ').split():
                path = Path(token).resolve()
                if path.is_relative_to(dependency_root):
                    name = path.relative_to(dependency_root).as_posix()
                    assert name in sources and sha(path) == sources[name], name
                    debug_compiled.add(name)
        report['debug_compiled_dependency_review'] = dict(compiled_dependencies=sorted(debug_compiled), undeclared_dependencies=[])
        run('debug-tests', ['xvfb-run', '-a', 'ctest', '--test-dir', debug,
                           '-R', 'native-canvas-world-handoff|native-world-raster-batch|opengl-sprite-atlas',
                           '--output-on-failure'])
        report['assertion_enabled_tests_passed'] = 3
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
        if (output / 'native-report.json').exists():
            report['native'] = json.loads((output / 'native-report.json').read_text())
    finally:
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = before_verified
        except Exception as error:
            report['success'] = False
            report['manifest_error'] = str(error)
        report['inputs'] = inputs
        report['inputs_stable'] = all(sha(Path(name)) == value for name, value in inputs.items())
        report['sources_stable'] = all(sha(ROOT / name) == value for name, value in sources.items())
        report['frozen_sources_stable'] = all((output / 'source' / name).is_file() and sha(output / 'source' / name) == value for name, value in sources.items())
        report['success'] = (report['success'] and report['inputs_stable'] and report['sources_stable']
                             and report['frozen_sources_stable'] and report.get('original_manifest_verified_before_after', False))
        (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(output / 'report.json', flush=True)
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
