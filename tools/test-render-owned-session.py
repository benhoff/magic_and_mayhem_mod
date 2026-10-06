#!/usr/bin/env python3
"""Replay ordered owned uploads, copies and Flips through x86, CPU and OpenGL."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol

CASES = ('mixed', 'failed', 'alias', 'limit', 'restore', 'held', 'file-failed', 'byte-limit', 'record-limit', 'indexed')


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def checks(case, events, limit):
    expected, states, created, operations = [], {}, set(), 0
    for step, op, subject, status, frames, partial in events:
        if op == 0 or op == 1 or status & 0x80000000 or operations >= limit:
            continue
        native = {1: (case / f'back-{step:08x}.bin').read_bytes(),
                  2: (case / f'front-{step:08x}.bin').read_bytes()}
        if op == 2:
            if partial:
                expected.append((1, states[1]))
            expected.append((1, native[1]))
            states[1] = native[1]
            created.add(1)
        elif op == 3:
            expected += [(1, states[1]), (2, states.get(2, bytes(32))), (2, native[2])]
            states[2] = native[2]
            created.add(2)
        elif op == 4:
            expected += [(2, states[2]), (1, states[1]), (2, native[2]), (1, native[1])]
            states.update(native)
        else:
            raise AssertionError(op)
        operations += 1
    return expected, created, operations


def indexed(case, capture, stream, build, env, rs, oracle):
    events = list(struct.iter_unpack('<II', (case / 'events.bin').read_bytes()))
    assert [count for step, count in events] == [1, 2, 2, 3, 4, 5, 5]
    presentations, previous, pixels = [], 0, b''
    for step, count in events:
        raw = (case / f'frame-{step:08x}.bin').read_bytes()
        header = struct.unpack('<16I', raw[:64])
        native = (case / f'original-{step:08x}.bin').read_bytes()
        colors = (case / f'colors-{step:08x}.bin').read_bytes()
        if count > previous:
            pixels = b''.join(colors[index*4:index*4+3] + b'\xff' for index in native)
            presentations.append((native, pixels))
        assert header[4] == count * 2 and header[10] == count and raw[64:] == pixels
        previous = count
    # Interpret every indexed presentation independently; palette stays with ID
    # while SWAP rotates only native indices. CHECK is comparison data, not input.
    surfaces, palettes, observed = {}, {}, []
    for op, payload, offset in rs:
        if op == 1:
            sid, width, height, bits, r, g, b = struct.unpack_from('<7I', payload)
            assert bits == 8
            surfaces[sid] = payload[28:]
        elif op == 4:
            sid, first, count = struct.unpack_from('<3I', payload)
            palette = palettes.setdefault(sid, bytearray(768))
            palette[first*3:(first+count)*3] = payload[12:]
        elif op == 5:
            sid = struct.unpack_from('<I', payload)[0]
            surfaces[sid] = payload[4:]
        elif op == 6:
            sid = struct.unpack('<I', payload)[0]
            colors = palettes[sid]
            native = surfaces[sid]
            observed.append((native, b''.join(colors[index*3:index*3+3] + b'\xff' for index in native)))
    # CPU oracle validates COPY/SWAP and every CHECK before the independent
    # original engine snapshots are used to compare presentation boundaries.
    command = capture / 'session-00000001.bin'
    assert oracle.cpu_replay(command.read_bytes()) == presentations[-1][0]
    assert observed == presentations
    assert sum(op == 1 for op, payload, offset in rs) == 3
    assert sum(op == 3 for op, payload, offset in rs) == 1
    assert sum(op == 11 for op, payload, offset in rs) == 3
    output = case / 'session.native'
    result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(command), '--output', str(output)],
                            env=env, capture_output=True, text=True, timeout=30)
    (case / 'gpu.log').write_text(result.stdout + result.stderr)
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    assert output.read_bytes() == presentations[-1][0]
    assert report['presentations'] == 5
    assert report['presentation_rgba_sha256'] == hashlib.sha256(presentations[-1][1]).hexdigest()
    result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                            env=env, capture_output=True, text=True, timeout=20)
    (case / 'qt.log').write_text(result.stdout + result.stderr)
    assert result.returncode == 0, result.stderr
    assert 'session_finished' in (capture / 'lifecycle.log').read_text()
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=CASES, action='append')
    parser.add_argument('--build', type=Path, default=REPO / 'working/build/qt-shell')
    args = parser.parse_args()
    selected = args.case or CASES
    dll = load('session_build', 'tools/build-render-bridge.py').build(True)
    stage = load('session_stage', 'tools/prepare-shadow-experiment.py')
    oracle = load('session_oracle', 'tools/test-render-lock-blits.py')
    partial = load('session_partial', 'tools/test-render-partial-locks.py')
    parent = REPO / 'working/tests/render-owned-session'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = args.build.resolve()
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-qt-shell', 'mnm-render-commands', '--parallel', '4'], check=True)
    reports = []
    for mode in selected:
        case = root / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        stream = case / 'frame.bin'
        with stream.open('wb') as f:
            f.write(frame_protocol.initial_header())
            f.truncate(frame_protocol.SIZE)
        shutil.copy2(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        command = capture / 'session-00000001.bin'
        if mode == 'file-failed':
            command.write_bytes(b'existing-session-sentinel')
        env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', MNM_RENDER_OWNED_SESSION='1',
                   MNM_RENDER_STREAM='Z:' + str(stream).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        if mode == 'indexed':
            env['MNM_BOOTSTRAP_SELFTEST'] = 'idxflip-rotate'
        elif mode in ('byte-limit', 'record-limit'):
            env['MNM_PARTIAL_LOCK_SELFTEST'] = 'session-bytes' if mode == 'byte-limit' else 'replay-budget'
        else:
            env['MNM_OWNED_SESSION_SELFTEST'] = mode
        with (case / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(case / 'selftest.exe')], cwd=case, env=env,
                           stdout=log, stderr=log, check=True, timeout=45)
        data = command.read_bytes()
        if mode == 'file-failed':
            assert data == b'existing-session-sentinel'
            rs = []
        else:
            rs = partial.records(data)
            assert len(data) <= 64 * 1024 * 1024 and len(rs) <= 4096
        if mode == 'indexed':
            report = indexed(case, capture, stream, build, env, rs, oracle)
            reports.append({'mode': mode, 'valid_session': True, 'commands': len(rs), 'bytes': len(data), 'gpu': report})
            print('Owned session fixture indexed: passed', flush=True)
            continue
        valid = mode in ('mixed', 'failed', 'alias', 'limit')
        expected_gap = 3 if mode in ('restore', 'held') else 2
        if not valid and mode != 'file-failed':
            assert rs[-1][0] == 9 and struct.unpack('<I', rs[-1][1]) == (expected_gap,)
            assert not any(op == 8 for op, payload, offset in rs)
        gpu_report = None
        if mode not in ('byte-limit', 'record-limit'):
            events = list(struct.iter_unpack('<6I', (case / 'session-events.bin').read_bytes()))
            previous, pixels = 0, b''
            for step, op, subject, status, count, region in events:
                raw = (case / f'frame-{step:08x}.bin').read_bytes()
                h = struct.unpack('<16I', raw[:64])
                assert h[10] == count and h[4] == count * 2
                if count > previous:
                    pixels = partial.rgba((case / f'front-{step:08x}.bin').read_bytes(), 16, (0xf800, 0x7e0, 0x1f))
                assert raw[64:] == pixels, (mode, step)
                previous = count
            if valid or mode == 'file-failed':
                assert previous == 4
            if valid:
                expected_checks, created, ops = checks(case, events, 16)
                actual_checks = [(struct.unpack_from('<I', payload)[0], payload[4:])
                                 for op, payload, offset in rs if op == 5]
                assert actual_checks == expected_checks, (mode, len(actual_checks), len(expected_checks))
                assert {struct.unpack_from('<I', payload)[0] for op, payload, offset in rs if op == 1} == created == {1, 2}
                assert sum(op == 1 for op, payload, offset in rs) == 2
                assert sum(op == 3 for op, payload, offset in rs) == 2
                assert sum(op == 11 for op, payload, offset in rs) == 2
                native = (case / f'front-{events[-1][0]:08x}.bin').read_bytes()
                assert oracle.cpu_replay(data) == native
            if previous:
                result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                                        env=env, capture_output=True, text=True, timeout=20)
                (case / 'qt.log').write_text(result.stdout + result.stderr)
                assert result.returncode == 0, result.stderr
        if mode != 'file-failed':
            output = case / 'session.native'
            result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(command),
                                     '--output', str(output)], env=env, capture_output=True, text=True, timeout=30)
            (case / 'gpu.log').write_text(result.stdout + result.stderr)
            assert result.returncode == (0 if valid else 2), (mode, result.stdout, result.stderr)
            if valid:
                gpu_report = json.loads(result.stdout)
                assert output.read_bytes() == native
                assert gpu_report['checks'] == len(expected_checks)
                assert gpu_report['presentations'] == 4
                assert gpu_report['surface_stats']['uploads'] == (13 if mode == 'limit' else 6)
                assert gpu_report['surface_stats']['copies'] == 2
                assert gpu_report['presentation_rgba_sha256'] == hashlib.sha256(partial.rgba(native, 16, (0xf800, 0x7e0, 0x1f))).hexdigest()
            else:
                assert not output.exists()
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        assert ('session_finished' if valid else 'session_file_failed' if mode == 'file-failed' else 'session_gap') in reasons
        reports.append({'mode': mode, 'valid_session': valid, 'commands': len(rs), 'bytes': len(data), 'reasons': sorted(reasons), 'gpu': gpu_report})
        print(f'Owned session fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_owned_ordered_sessions',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(), 'fixtures': reports}, indent=2) + '\n')
    print(f'Owned ordered sessions passed: {root}')


if __name__ == '__main__':
    main()
