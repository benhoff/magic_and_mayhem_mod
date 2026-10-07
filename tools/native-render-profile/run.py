#!/usr/bin/env python3
"""Replay a complete-frame capture prefix on headless NVIDIA EGL and llvmpipe.

This profiles the unchanged renderer and shared GPU lease path, not the live
producer, Qt widgets, a compositor, scanout or input-to-screen latency.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import statistics
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prefix(archive):
    data = archive.read_bytes()
    if len(data) > 64 * 1024 * 1024 or data[:8] not in (b'MNMCMD01', b'MNMCMD02', b'MNMCMD03'):
        raise ValueError('Expected bounded native command archive, at most64MiB')
    at, last, frames, ops = 16, None, 0, Counter()
    while at + 12 <= len(data):
        op, sequence, size = struct.unpack_from('<III', data, at)
        if at + 12 + size > len(data):
            raise ValueError('Truncated archive record')
        if op in (8, 9):
            break
        at += 12 + size
        ops[op] += 1
        if op == 6:
            last = at
            frames += 1
    if last is None:
        raise ValueError('No complete PRESENT in archive')
    return data[:last], dict(archive=str(archive), archive_sha256=sha(archive),
                            selected_prefix_bytes=last, selected_frames=frames,
                            policy='Exact bytes through final complete PRESENT preceding archival terminal marker; exclude unfinished tail. Decoder/consumer validate all selected records; no commands rewritten.')


def fixture():
    data = bytearray(b'MNMCMD01' + struct.pack('<II', 1, 16))
    sequence = 0

    def record(op, words, pixels=b''):
        nonlocal sequence
        sequence += 1
        payload = struct.pack('<' + 'I' * len(words), *words) + pixels
        data.extend(struct.pack('<III', op, sequence, len(payload)) + payload)

    record(1, [1, 2, 2, 16, 0xf800, 0x7e0, 31], struct.pack('<4H', 0xf800, 0x7e0, 31, 0xffff))
    record(6, [1])
    record(2, [1, 0, 0, 1, 1], struct.pack('<H', 0x8410))
    record(6, [1])
    record(1, [2, 2, 2, 16, 0xf800, 0x7e0, 31], struct.pack('<4H', 0xffff, 0xf800, 0x7e0, 31))
    record(3, [2, 1, 0, 0, 2, 2, 0, 0, 1, 0xffff])
    record(6, [1])
    record(2, [1, 1, 1, 1, 1], struct.pack('<H', 0xffff))
    record(6, [1])
    rgba = bytearray(bytes([0, 0, 0, 255]) * 800 * 600)
    for x, y, color in [(0, 0, [132, 130, 132, 255]), (1, 0, [255, 0, 0, 255]),
                         (0, 1, [0, 255, 0, 255]), (1, 1, [255, 255, 255, 255])]:
        at = (y * 800 + x) * 4
        rgba[at:at + 4] = bytes(color)
    return bytes(data), bytes(rgba)


def distribution(values):
    values = sorted(values)
    return dict(count=len(values), median=statistics.median(values),
                p95=values[int((len(values) - 1) * .95)], maximum=max(values))


def summarize(report):
    result = dict(frames_per_replay=report['runs'][0]['frames'],
                  commands_per_replay=report['runs'][0]['commands'],
                  completed_frames_per_second=distribution([r['frames'] * 1000 / r['completion_ms'] for r in report['runs']]),
                  replay_completion_ms=distribution([r['completion_ms'] for r in report['runs']]))
    if report['samples']:
        for selection, samples in [('all_frames', report['samples']),
                                   ('after_checkpoint', [s for s in report['samples'] if s['frame'] > 0])]:
            if samples:
                result[selection] = {key: distribution([s[key] for s in samples]) for key in
                                     ['cpu_submit_ms', 'completion_ms', 'completion_wait_ms', 'gpu_timeline_ms']}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--build', type=Path, default=ROOT/'working/build/native-render-profile')
    parser.add_argument('--replays', type=int, default=12)
    parser.add_argument('--perf', action='store_true', help='CPU-sample additional NVIDIA throughput replay')
    args = parser.parse_args()
    if not 1 <= args.replays <= 100:
        parser.error('--replays must be1..100')
    build = args.build.resolve()
    parent = ROOT/'working/tests/native-render-gpu-profile'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    sources = sorted(set([str(p.relative_to(ROOT)) for p in (ROOT/'tools/native-render-profile').iterdir() if p.is_file()]
                         + [str(p.relative_to(ROOT)) for p in (ROOT/'renderer').iterdir() if p.suffix in ('.cpp', '.hpp') or p.name == 'CMakeLists.txt']))
    fingerprints = {p: sha(ROOT/p) for p in sources}
    artifacts = {str(p.relative_to(ROOT)): sha(p) for p in
                 [build/'mnm-headless-profile', build/'platforms/libmnm_headless_egl.so', build/'renderer/libmnm-renderer.a']}
    data, provenance = prefix(args.archive.resolve())
    workload = run/'prefix.bin'
    workload.write_bytes(data)
    synthetic, expected = fixture()
    fixture_path = run/'synthetic.bin'
    fixture_path.write_bytes(synthetic)
    report = dict(schema=1, success=False, scope=__doc__, sources=fingerprints,
                  artifacts_sha256=artifacts, provenance=provenance, runs={}, checks={}, raw_directory=str(run),
                  timing_policy='Two warmup replays per process. Decode/context/shader construction, resource cleanup and one final diagnostic readback per replay are outside timing. Serial completion uses per-frame GPU timestamps and cross-context fences plus glFinish; GPU timeline spans GPU start through sampling completion and includes host feeding gaps, not pure shader busy time. Throughput avoids timer queries and per-frame waits, then drains GPU once at replay end. No frame or command skipping.',
                  limitations=['Startup/Main-menu capture prefix only; no battle.', 'No live producer, IPC queue age, game pacing, Qt widgets, compositor, vsync, monitor or input-to-screen measurement.', 'EGL device/pbuffer context switching differs from desktop GLX and timing instrumentation adds some overhead.', 'Repeated finite workload and uncontrolled host load; renderer capacity is not live-game FPS.'])

    def execute(vendor, mode, name, source, repetitions, rgba=None, profile=False):
        env = {k: v for k, v in os.environ.items() if k not in
               ['DISPLAY', 'WAYLAND_DISPLAY', 'LIBGL_ALWAYS_SOFTWARE', '__GLX_VENDOR_LIBRARY_NAME', '__EGL_VENDOR_LIBRARY_FILENAMES', 'MNM_PROFILE_RGBA_OUT']}
        env.update(QT_QPA_PLATFORM='mnm-headless-egl', QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'), MNM_PROFILE_EGL_VENDOR=vendor)
        if vendor == 'Mesa':
            env['LIBGL_ALWAYS_SOFTWARE'] = '1'
        vendor_file = Path('/usr/share/glvnd/egl_vendor.d')/('10_nvidia.json' if vendor == 'NVIDIA' else '50_mesa.json')
        if vendor_file.exists():
            env['__EGL_VENDOR_LIBRARY_FILENAMES'] = str(vendor_file)
        if rgba:
            env['MNM_PROFILE_RGBA_OUT'] = str(rgba)
        out = run/(name+'.json')
        command = [str(build/'mnm-headless-profile'), str(source), mode, str(repetitions), str(out)]
        if profile:
            command = ['perf', 'record', '-e', 'cpu-clock:u', '-F', '499', '-g', '--call-graph', 'dwarf,16384', '-o', str(run/'perf.data'), '--'] + command
        with (run/(name+'.log')).open('w') as log:
            result = subprocess.run(command, env=env, stdout=log, stderr=log, timeout=180)
        assert result.returncode == 0, (name, result.returncode, (run/(name+'.log')).read_text()[-2000:])
        r = json.loads(out.read_text())
        assert r['success'] and not r['desktop_used']
        assert ('TITAN RTX' in r['renderer']) if vendor == 'NVIDIA' else ('llvmpipe' in r['renderer'])
        assert all(a['native_readbacks'] == a['rgba_readbacks'] == 0 for a in r['runs'])
        return r

    try:
        for vendor in ['NVIDIA', 'Mesa']:
            name = vendor.lower()+'-synthetic'
            rgba = run/(name+'.rgba')
            r = execute(vendor, 'serial', name, fixture_path, 1, rgba)
            assert rgba.read_bytes() == expected, 'Independent final-frame keyed-copy/update/RGB565 pixels disagree'
            assert r['runs'][0]['frames'] == 4 and r['runs'][0]['commands'] == 9
            report['checks'][name] = dict(success=True, expected_rgba_sha256=hashlib.sha256(expected).hexdigest(), renderer=r['renderer'])
        output_hashes = set()
        for vendor in ['NVIDIA', 'Mesa']:
            for mode in ['serial', 'throughput']:
                name = vendor.lower()+'-'+mode
                r = execute(vendor, mode, name, workload, args.replays)
                output_hashes.add(r['final_rgba_sha256'])
                report['runs'][name] = dict(summary=summarize(r), report=r)
                print(name, json.dumps(report['runs'][name]['summary']), flush=True)
        assert len(output_hashes) == 1, 'Hardware/software or mode final pixels disagree'
        report['checks']['identical_hardware_software_final_pixels'] = True
        if args.perf:
            r = execute('NVIDIA', 'throughput', 'nvidia-cpu-profile', workload, max(args.replays, 40), profile=True)
            assert r['final_rgba_sha256'] in output_hashes
            report['cpu_profile'] = dict(scope='Additional NVIDIA throughput replay, including warmup/setup/diagnostic cleanup; CPU-clock samples of the native process only, no Wine or producer.', summary=summarize(r))
            for name, sort in [('cpu-dso', 'dso'), ('cpu-symbol', 'symbol')]:
                with (run/(name+'.txt')).open('w') as out:
                    subprocess.run(['perf', 'report', '--stdio', '-i', str(run/'perf.data'), '--no-children', '--sort', sort, '--percent-limit', '.5', '--call-graph', 'none'], stdout=out, check=True)
            with (run/'cpu-callers.txt').open('w') as out:
                subprocess.run(['perf', 'report', '--stdio', '-i', str(run/'perf.data'), '--children', '--sort', 'symbol', '--percent-limit', '1', '--call-graph', 'none'], stdout=out, check=True)
        assert fingerprints == {p: sha(ROOT/p) for p in sources}, 'Sources changed during profile'
        assert artifacts == {p: sha(ROOT/p) for p in artifacts}, 'Build changed during profile'
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    report['raw_artifacts_sha256'] = {str(p.relative_to(ROOT)): sha(p) for p in run.iterdir() if p.is_file()}
    path = run/'report.json'
    path.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(dict(success=report['success'], report=str(path))), flush=True)
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
