#!/usr/bin/env python3
"""Validate512 ordered native World queues on NVIDIA EGL; no desktop or original game launch."""
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
BEHAVIOR = 'NR.world-rolling-hardware'
SCENARIO = 'native-world-rolling-hardware-20261010'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def distribution(values):
    ordered = sorted(values)
    return dict(median=statistics.median(ordered), p95=ordered[math.ceil(len(ordered) * .95) - 1],
                max=ordered[-1], samples=len(ordered))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--corpus', type=Path, required=True)
    p.add_argument('--queues', type=int, default=512)
    p.add_argument('--gl33', action='store_true')
    p.add_argument('--prepared', type=Path)
    p.add_argument('--median-ms', type=float, default=60)
    p.add_argument('--p95-ms', type=float, default=120)
    p.add_argument('--max-ms', type=float, default=300)
    p.add_argument('--cold-max-ms', type=float, default=500)
    p.add_argument('--rss-growth-mib', type=float, default=32)
    args = p.parse_args()
    if not 64 <= args.queues <= 10000 or not 0 < args.median_ms <= args.p95_ms <= args.max_ms or args.cold_max_ms <= 0 or args.rss_growth_mib <= 0:
        p.error('Require64..10000 queues and positive ordered latency/memory limits')
    r = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    b = next(b for b in r['behaviors'] if b['id'] == BEHAVIOR)
    s = next(s for s in r['scenarios'] if s['id'] == SCENARIO)
    sources = {n: sha(ROOT / n) for n in sorted(required_sources(b))}
    claims = [dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(b, {v['id']: v for v in r['builds']}),
                   scenarios={SCENARIO: scenario_contract(s)})]
    parent = ROOT / 'working/tests/native-world-rolling-hardware'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    (out / 'prospective-claims.json').write_text(json.dumps(dict(sources=sources, claims=claims), indent=2) + '\n')
    env = {k:v for k,v in os.environ.items() if k not in ('DISPLAY','WAYLAND_DISPLAY','LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME')}
    env.update(QT_QPA_PLATFORM='mnm-headless-egl',MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
    if args.gl33:
        p.error('NVIDIA EGL is a separate hardware path, not the Mesa GL3.3 fallback')
    report = dict(success=False, scope=b['scope'], sources=sources, claims=claims,
                  settings={k: str(v) if isinstance(v, Path) else v for k, v in vars(args).items()},
                  original_pixels_used_as_native_inputs=False, live_gameplay_executed=False)
    inputs = {}
    before = False

    def run(name, command, timeout=600):
        with (out / (name + '.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, check=True, timeout=timeout)

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        before = True
        corpus = args.corpus.resolve()
        historical = json.loads((corpus / 'report.json').read_text())
        assert historical['success'] and historical['inputs_stable'] and historical['original_pixels_used_as_native_inputs'] is False
        assert sha(corpus / 'inputs/canvas-producers.bin') == historical['owned_stream_sha256']
        for n, digest in historical['asset_sources'].items():
            path = (corpus / 'assets' / n).resolve()
            assert path.is_relative_to(corpus / 'assets') and sha(path) == digest, n
            inputs[str(path)] = digest
        for n in ('report.json', 'inputs/canvas-producers.bin', 'inputs/canvas-producers.done', 'after/live-report.json'):
            inputs[str(corpus / n)] = sha(corpus / n)
        expected = json.loads((corpus / 'after/live-report.json').read_text())
        assert expected['success'] and expected['world_queues'] == 32 and expected['original_pixels_used_as_native_inputs'] is False and not expected['original_oracles_read']
        import struct
        marker = (corpus / 'inputs/canvas-producers.done').read_bytes()
        assert len(marker) == 32 and marker[:8] == b'MNMPDONE'
        version, queues, records, error, checkpoints, size = struct.unpack('<6I', marker[8:])
        assert version == 1 and queues == 32 and error == 0 and checkpoints == len(expected['checkpoints']) and size == (corpus / 'inputs/canvas-producers.bin').stat().st_size
        report['closed_input'] = dict(queues=queues, records=records, checkpoints=checkpoints, bytes=size)
        frozen = out / 'source'
        tracked = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
        configured = {n for n in tracked if n and n.split('/')[0] not in ('original', 'working')
                      and (Path(n).suffix in ('.c', '.cpp', '.h', '.hpp', '.S', '.cmake') or Path(n).name == 'CMakeLists.txt')}
        for n in sorted(configured | set(sources)):
            destination = frozen / n
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / n, destination)
        prepared = args.prepared.resolve() if args.prepared else None
        if prepared:
            old = json.loads((prepared / 'report.json').read_text())
            assert old['success'] and old['sources_stable'] and old['sources'] == sources
            assert sha(prepared / 'build/native-world-rolling') == old['binary_sha256']
            inputs[str(prepared / 'report.json')] = sha(prepared / 'report.json')
            for n, value in old['debug_binaries'].items():
                assert sha(prepared / 'debug' / n) == value
        build = prepared / 'build' if prepared else out / 'build'
        debug = prepared / 'debug' if prepared else out / 'debug'
        dependency_root = prepared / 'source' if prepared else frozen

        def dependencies(tree):
            names = set()
            for d in tree.rglob('*.o.d'):
                for token in d.read_text().replace('\\\n', ' ').split():
                    path = Path(token).resolve()
                    if path.is_file() and path.is_relative_to(dependency_root):
                        n = path.relative_to(dependency_root).as_posix()
                        assert n in sources and sha(path) == sources[n], n
                        names.add(n)
            assert names
            return dict(compiled_dependencies=sorted(names), undeclared_dependencies=[])

        if not prepared:
            run('configure', ['cmake', '-S', frozen / 'tests/native-world-rolling', '-B', build, '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DBUILD_TESTING=OFF'])
            run('build', ['cmake', '--build', build, '--target', 'native-world-rolling', '-j4'])
        report['compiled_dependency_review'] = dependencies(build)
        binary = build / 'native-world-rolling'
        report['binary_sha256'] = sha(binary)
        inputs[str(binary)] = sha(binary)
        plugin=out/'egl-build'
        run('egl-configure',['cmake','-S',frozen/'tools/native-render-profile','-B',plugin,'-DCMAKE_BUILD_TYPE=RelWithDebInfo'])
        run('egl-build',['cmake','--build',plugin,'--target','mnm_headless_egl','-j4'])
        report['egl_compiled_dependency_review']=dependencies(plugin)
        report['egl_binary_sha256']=sha(plugin/'platforms/libmnm_headless_egl.so')
        env['QT_QPA_PLATFORM_PLUGIN_PATH']=str(plugin/'platforms')
        report['hardware_environment']={k:env[k] for k in ('QT_QPA_PLATFORM','MNM_PROFILE_EGL_VENDOR','__EGL_VENDOR_LIBRARY_FILENAMES')}
        run('rolling', [binary, corpus / 'inputs/canvas-producers.bin', corpus / 'assets', corpus / 'after/live-report.json', out / 'native-report.json', args.queues], timeout=3600)
        native = json.loads((out / 'native-report.json').read_text())
        report['native'] = native
        assert 'NVIDIA' in native['driver']['vendor'] and 'llvmpipe' not in native['driver']['renderer']
        report['hardware_vendor_verified']=True
        assert native['success'] and native['world_queues'] == args.queues and native['acknowledgements'] == args.queues and native['remaining_surfaces'] == 0
        assert native['saved_checkpoint_comparisons'] == checkpoints and native['backpressure_attempts'] > 0 and native['source_packet_refusals'] >= 15
        frames = native['frames']
        assert len(frames) == args.queues
        warm = distribution([f['delivery_completion_ms'] for f in frames[32:]])
        cold = frames[0]['delivery_completion_ms']
        memory = [f['rss_bytes'] for f in frames[63:]]
        growth = max(memory) - memory[0]
        passed = warm['median'] <= args.median_ms and warm['p95'] <= args.p95_ms and warm['max'] <= args.max_ms and cold <= args.cold_max_ms and growth <= args.rss_growth_mib * 1024 * 1024
        report['performance_budget'] = dict(passed=passed, continued_delivery_completion_ms=warm, cold_delivery_completion_ms=cold,
            median_limit_ms=args.median_ms, p95_limit_ms=args.p95_ms, max_limit_ms=args.max_ms, cold_limit_ms=args.cold_max_ms,
            rss_growth_bytes=growth, rss_growth_limit_bytes=args.rss_growth_mib * 1024 * 1024)
        assert passed, 'Rolling delivery latency/memory budget exceeded'
        if not prepared:
            run('debug-configure', ['cmake', '-S', frozen / 'tests/native-world-rolling', '-B', debug, '-DCMAKE_BUILD_TYPE=Debug', '-DBUILD_TESTING=ON'])
            run('debug-build', ['cmake', '--build', debug, '--target', 'world-rolling-channel-test', 'canvas-world-test', 'canvas-sequence-test', 'world-raster-batch-test', 'sprite-atlas-test', '-j4'])
        report['debug_compiled_dependency_review'] = dependencies(debug)
        report['debug_binaries'] = {}
        for n in ('world-rolling-channel-test', 'canvas-world-test', 'canvas-sequence-test', 'world-raster-batch-test', 'sprite-atlas-test'):
            candidates = [p for p in debug.rglob(n) if p.is_file() and os.access(p, os.X_OK)]
            assert len(candidates) == 1
            path = candidates[0]
            report['debug_binaries'][path.relative_to(debug).as_posix()] = sha(path)
            inputs[str(path)] = sha(path)
        env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8')
        env.pop('QT_QPA_PLATFORM_PLUGIN_PATH',None)
        env.pop('__EGL_VENDOR_LIBRARY_FILENAMES',None)
        report['debug_fixture_environment']={k:env[k] for k in ('QT_QPA_PLATFORM','LIBGL_ALWAYS_SOFTWARE','LP_NUM_THREADS')}
        run('debug-tests', ['xvfb-run', '-a', 'ctest', '--test-dir', debug, '-R', 'native-world-rolling-channel|native-canvas-world-handoff|native-canvas-sequence|native-world-raster-batch|opengl-sprite-atlas', '--output-on-failure'])
        report['assertion_enabled_tests_passed'] = 5
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
        if (out / 'native-report.json').exists():
            report['native'] = json.loads((out / 'native-report.json').read_text())
    finally:
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = before
        except Exception as error:
            report['success'] = False
            report['manifest_error'] = str(error)
        report['inputs'] = inputs
        report['inputs_stable'] = all(sha(Path(n)) == value for n, value in inputs.items())
        report['sources_stable'] = all(sha(ROOT / n) == value for n, value in sources.items())
        report['frozen_sources_stable'] = all((out / 'source' / n).is_file() and sha(out / 'source' / n) == value for n, value in sources.items())
        report['success'] = report['success'] and report['inputs_stable'] and report['sources_stable'] and report['frozen_sources_stable'] and report.get('original_manifest_verified_before_after', False)
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(out / 'report.json', flush=True)
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
