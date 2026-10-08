#!/usr/bin/env python3
"""Replay every captured startup queue natively; compare each completed canvas."""
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


def differences(actual, expected, width):
    if len(actual) != len(expected) or len(actual) % 2:
        raise ValueError('Completed canvas extent differs')
    offsets = [i for i, (a, b) in enumerate(zip(struct.iter_unpack('<H', actual), struct.iter_unpack('<H', expected))) if a != b]
    return {'pixels': len(actual)//2, 'mismatches': len(offsets),
            'bounds': [min(i % width for i in offsets), min(i//width for i in offsets),
                       max(i % width for i in offsets), max(i//width for i in offsets)] if offsets else None,
            'examples': [{'x': i % width, 'y': i//width, 'native_word': struct.unpack_from('<H', actual, i*2)[0],
                          'original_word': struct.unpack_from('<H', expected, i*2)[0]} for i in offsets[:16]],
            'actual_sha256': hashlib.sha256(actual).hexdigest(), 'expected_sha256': hashlib.sha256(expected).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture-report', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    spec = importlib.util.spec_from_file_location('world', ROOT/'tools/test-world-frames.py')
    world = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(world)
    sources = world.sources()
    for name in ['tools/test-native-startup-sequence.py', 'tests/world-startup-wave-abi.c', 'tests/canvas-startup-call.S', 'tests/scene-history-test.cpp']:
        sources[name] = sha(ROOT/name)
    claims = json.loads(args.claims.read_text()) if args.claims else None
    if claims:
        for name, digest in claims['sources'].items():
            if sha(ROOT/name) != digest:
                raise ValueError('Coverage declaration changed: '+name)
            sources[name] = digest
    parent = ROOT/'working/tests/native-startup-sequence'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = {'success': False, 'sources': sources, 'source_executable_sha256': world.BUILD_HASH,
              'scope': 'Every completed native startup World queue in a contiguous chain initialized with native word-zero storage; comparison to independent original raster replay and actual live after-canvases. Original menu/loading and between-queue HUD producers are separate measured gaps.',
              'native_clear_word': 0, 'original_pixels_used_as_native_inputs': False,
              'original_initialization_recovered': False, 'original_work_bypassed': False, 'live_replacement': False}
    if claims:
        report['claims'] = claims['claims']
    inputs = {}
    env = {**os.environ, 'QT_QPA_PLATFORM': 'xcb', 'LIBGL_ALWAYS_SOFTWARE': '1', 'WINEDEBUG': '-all'}

    # Other sessions share this checkout. Build/reference execution uses one
    # frozen source tree; later workspace edits are reported as freshness limits.
    sourceRoot = out/'source'
    names = subprocess.check_output(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=ROOT).decode().split('\0')
    snapshot = {}
    for name in sorted(set(names)):
        path = ROOT/name
        if not name or name.split('/')[0] in {'original', 'working', '.git'} or not path.is_file():
            continue
        if path.suffix not in {'.c', '.h', '.S', '.cpp', '.hpp', '.py', '.cmake', '.mk'} and path.name not in {'CMakeLists.txt', 'README.md'}:
            continue
        data = path.read_bytes()
        target = sourceRoot/name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        snapshot[name] = hashlib.sha256(data).hexdigest()
    for name, digest in sources.items():
        # Research format documents are provenance inputs, not compilation inputs.
        if name not in snapshot:
            target = sourceRoot/name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((ROOT/name).read_bytes())
            snapshot[name] = sha(target)
        if snapshot[name] != digest:
            raise RuntimeError('Source changed while freezing experiment: '+name)
    manifest = out/'source-manifest.json'
    manifest.write_text(json.dumps(snapshot, indent=2)+'\n')

    def run(name, command, runenv=None, timeout=180):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=runenv or env,
                           stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=True)

    def pin(path):
        inputs[str(path.relative_to(ROOT))] = sha(path)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        capturePath = args.capture_report.resolve()
        pin(capturePath)
        capture = json.loads(capturePath.read_text())
        if not capture['success'] or not capture['startup_replay'] or capture['source_executable_sha256'] != world.BUILD_HASH:
            raise ValueError('Requires an original --startup-replay capture')
        experiment = Path(capture['experiment'])
        queueInfo = capture['startup_queues']
        count = queueInfo['requested_queues']
        if count != capture['samples'] or not queueInfo['contiguous_from_first_queue'] or not queueInfo['all_raw_rows_captured']:
            raise ValueError('Requires a complete, one-sample-per-queue prefix')
        for i, q in enumerate(queueInfo['queues'], 1):
            path = experiment/'capture'/q['path']
            if q['queue'] != i or q['sample'] != i or sha(path) != q['sha256']:
                raise ValueError('Raw queue/sample sequence changed')
            pin(path)
        for name, digest in capture['world_frames'].items():
            path = experiment/name
            if sha(path) != digest:
                raise ValueError('Original World capture changed')
            pin(path)
        lifetime = experiment/capture['canvas_lifetime']['path']
        if sha(lifetime) != capture['canvas_lifetime']['sha256']:
            raise ValueError('Canvas identity trace changed')
        pin(lifetime)
        identities = list(struct.iter_unpack('<16I', lifetime.read_bytes()[64:]))[:count]
        if len(identities) != count or not identities[0][3] or any(row[3:7] != identities[0][3:7] for row in identities):
            raise ValueError('Startup chain requires one observed canvas identity/extent')
        pe = ROOT/'working/game-nocd/Chaos.exe'
        if sha(pe) != world.BUILD_HASH:
            raise ValueError('Unsupported original reference executable')
        pin(pe)
        report['entry_anchors'] = world.reference_anchor(pe)
        requests = [experiment/'capture'/f'world-{i:04}.bin' for i in range(1, count+1)]
        needed = {world.identity(raw, header[9]) for path in requests for header, raw in world.records(path.read_bytes())}
        bindings = out/'bindings.json'
        bindings.write_text(json.dumps(world.pinned_bindings(experiment/'game', needed), indent=2)+'\n')
        pin(bindings)
        requestRoot = out/'requests'
        requestRoot.mkdir()
        frames = []
        for i, path in enumerate(requests, 1):
            target = requestRoot/path.name
            target.write_bytes(path.read_bytes())
            pin(target)
            frames.append({'canvas': 1, 'queue': i, 'reset': i == 1, 'snapshot': target.name, 'sha256': sha(target)})
        timeline = requestRoot/'timeline.json'
        timeline.write_text(json.dumps({'version': 1, 'native_clear_word': 0, 'frames': frames}, indent=2)+'\n')
        pin(timeline)
        build = out/'build-native'
        run('configure', ['cmake', '-S', sourceRoot/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'mnm-world-history-preview', 'world-frame-test', 'scene-history-test', '-j4'])
        run('native-regressions', ['ctest', '--test-dir', build, '-R', '^(native-world-frame|native-scene-history)$', '--output-on-failure'])
        run('bridge-build', ['python3', sourceRoot/'tools/build-scene-observer.py', '--selftest'])
        pebuild = sourceRoot/'working/build/scene-observer-selftest'
        flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
                 '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-DMNM_SCENE_STARTUP_REPLAY_SELFTEST']
        objects = []
        for name in ['tests/world-startup-wave-abi.c', 'tests/canvas-startup-call.S', 'runtime/scene/world_trace.c', 'runtime/scene/world_entry.S']:
            obj = out/(Path(name).name+'.obj')
            run('abi-build-'+Path(name).name, [*flags, '-c', sourceRoot/name, '-o', obj])
            objects.append(obj)
        abi = out/'wave-abi.exe'
        run('abi-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                         '/base:0x400000', '/dynamicbase:no', '/safeseh:no', '/timestamp:0', '/out:'+str(abi), *objects, pebuild/'kernel32.lib'])
        run('abi-forwarding', ['wine', abi], {**env, 'WINEPREFIX': str(ROOT/'working/tests/scene-selftest-wine')})
        report['wave_return_abi_passed'] = True
        report['shade_minus127_palette_capture_passed'] = True
        reference = out/'reference'
        run('reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                               '-fno-pie', '-no-pie', sourceRoot/'tests/world-frame-reference.cpp', '-o', reference])
        native = out/'native'
        run('native-chain', [build/'mnm-world-history-preview', '--root', experiment/'game', '--bindings', bindings,
                             '--timeline', timeline, '--output', native], timeout=300)
        nativeReport = json.loads((native/'report.json').read_text())
        if not nativeReport['success'] or nativeReport['remaining_surfaces'] or nativeReport['original_pixels_used_as_native_inputs']:
            raise RuntimeError('Native history invariant failed')
        rows = []
        previousReference = previousNative = previousLive = None
        for i, path in enumerate(requests, 1):
            raw = path.read_bytes()
            width, height, stride = struct.unpack_from('<3I', raw, 24)
            crop = lambda pixels: b''.join(pixels[y*stride*2:(y*stride+width)*2] for y in range(height))
            beforePath, afterPath = path.with_suffix('.before.565'), path.with_suffix('.after.565')
            before, live = crop(beforePath.read_bytes()), crop(afterPath.read_bytes())
            original = out/f'original-chain-{i:04}.565'
            fromBefore = out/f'original-from-before-{i:04}.565'
            run(f'original-chain-{i:04}', [reference, pe, path, original, *([previousReference] if previousReference else [])])
            run(f'original-from-before-{i:04}', [reference, pe, path, fromBefore, beforePath])
            actual = (native/f'{i}.565').read_bytes()
            zero = bytes(width*height*2)
            rows.append({'queue': i, 'draws': struct.unpack_from('<I', raw, 36)[0],
                         'queue_kinds': queueInfo['queues'][i-1]['kind_counts'],
                         'native_vs_original_same_initial_state': differences(actual, original.read_bytes(), width),
                         'native_vs_live_original': differences(actual, live, width),
                         'original_from_captured_before_vs_live': differences(fromBefore.read_bytes(), live, width),
                         'native_before_vs_original_before': differences(previousNative if previousNative is not None else zero, before, width),
                         'between_queue_original_writes': differences(previousLive, before, width) if previousLive is not None else None,
                         'native_canvas': str((native/f'{i}.565').relative_to(ROOT)),
                         'original_live_canvas': str(afterPath.relative_to(ROOT))})
            previousReference, previousNative, previousLive = original, actual, live
            print(f'queue {i}: original replay {rows[-1]["native_vs_original_same_initial_state"]["mismatches"]} differences; live {rows[-1]["native_vs_live_original"]["mismatches"]}', flush=True)
        report.update(frames=rows, completed_canvases_compared=len(rows), pixels_compared=sum(r['native_vs_live_original']['pixels'] for r in rows),
                      all_native_original_same_initial_state_match=all(not r['native_vs_original_same_initial_state']['mismatches'] for r in rows),
                      all_original_before_replays_match_live=all(not r['original_from_captured_before_vs_live']['mismatches'] for r in rows),
                      all_native_live_canvases_match=all(not r['native_vs_live_original']['mismatches'] for r in rows),
                      original_before_used_only_by_private_original_reference=True,
                      native_history=nativeReport, capture_report=str(capturePath.relative_to(ROOT)))
        if any(sha(sourceRoot/name) != digest for name, digest in snapshot.items()):
            raise RuntimeError('Frozen comparison sources changed during execution')
        if any(sha(ROOT/name) != digest for name, digest in inputs.items()):
            raise RuntimeError('Comparison inputs changed during execution')
        report.update(success=True, sources_stable=True, inputs=inputs,
                      current_workspace_sources_match=all(sha(ROOT/name) == digest for name, digest in sources.items()),
                      frozen_source_manifest=str(manifest.relative_to(ROOT)), frozen_source_manifest_sha256=sha(manifest),
                      binaries={str(path.relative_to(ROOT)): sha(path) for path in [abi, reference, build/'mnm-world-history-preview']})
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        with (out/'report.json').open('x') as output:
            json.dump(report, output, indent=2)
            output.write('\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
