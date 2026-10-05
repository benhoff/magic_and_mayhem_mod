#!/usr/bin/env python3
"""Un-Locked RGB primaries: x86 metadata -> complete 800x600 copy -> Qt/OpenGL."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import statistics
import subprocess
import tempfile
from importlib.util import spec_from_file_location, module_from_spec

REPO = Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol



def load(name, path):
    spec = spec_from_file_location(name, REPO / path)
    module = module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def rgba(native, bits):
    masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
    output = bytearray()
    for at in range(0, len(native), bits // 8):
        value = int.from_bytes(native[at:at + bits // 8], 'little')
        for mask in masks:
            low = mask & -mask
            output.append(((value & mask) // low) * 255 // (mask // low))
        output.append(255)
    return bytes(output)


def main():
    cases = ('dc-rgb24', 'dc-rgb32', 'dc', 'dc-bottom-up', 'dc-retry', 'dc-format', 'dc-unmatched', 'dc-swapped', 'thread-timeout', 'thread-contention', 'nested-source-contention', 'pixel-miss-overflow', 'partial-lock-contention', 'partial-blit-contention', 'nested-lock-contention', 'continuous', 'continuous-live', 'many-surfaces', 'continuous-release', 'continuous-unmatched', 'continuous-contention', 'lock-contention', 'unknown-unlock', 'property-contention', 'release-contention', 'fill', 'fill24', 'fill32', 'fill-failed', 'fill-partial', 'fill-update', 'fill-session', 'descriptor-key', 'descriptor-key-range', 'opaque', 'fast', 'legacy', 'negative', 'rgb24', 'rgb32', 'alias', 'created', 'retry',
             'nested-description', 'offscreen', 'partial', 'keyed', 'caps-missing',
             'dimensions-missing', 'format-missing', 'bad-mask', 'failed-description', 'lock-description-change')
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=cases, action='append')
    parser.add_argument('--tracker-wait-ms', type=int, choices=range(51))
    args = parser.parse_args()
    selected = args.case or cases
    dll = load('bootstrap_build', 'tools/build-render-bridge.py').build(True)
    stage = load('bootstrap_stage', 'tools/prepare-shadow-experiment.py')
    oracle = load('bootstrap_oracle', 'tools/test-render-lock-blits.py')
    parent = REPO / 'working/tests/render-bootstrap'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/qt-shell'
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-qt-shell', 'mnm-render-commands', '--parallel', '4'], check=True)
    # Keep this report's executables stable if another task rebuilds the shared
    # application build directory during the longer replay checks.
    shell_binary = root / 'mnm-qt-shell'
    replay_binary = root / 'mnm-render-commands'
    shutil.copy2(build / 'mnm-qt-shell', shell_binary)
    shutil.copy2(build / 'renderer/mnm-render-commands', replay_binary)
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
        env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', MNM_BOOTSTRAP_SELFTEST=mode,
                   MNM_RENDER_STREAM='Z:' + str(stream).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        if args.tracker_wait_ms is not None:
            env['MNM_RENDER_TRACKER_WAIT_MS'] = str(args.tracker_wait_ms)
        if mode == 'fill-session':
            env['MNM_RENDER_OWNED_SESSION'] = '1'
        with (case / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(case / 'selftest.exe')], cwd=case, env=env,
                           stdout=log, stderr=log, check=True, timeout=30)
        accepted = mode not in ('partial', 'keyed', 'caps-missing', 'dimensions-missing',
                               'format-missing', 'bad-mask', 'failed-description', 'lock-description-change', 'dc-format', 'dc-unmatched', 'dc-swapped')
        files = sorted(capture.glob('blit-*.bin'))
        thread_case = mode in ('thread-contention', 'thread-timeout')
        thread_no_wait = thread_case and (args.tracker_wait_ms == 0 or mode == 'thread-timeout')
        scoped_partial = thread_case or mode in ('partial-lock-contention', 'partial-blit-contention', 'nested-lock-contention')
        missed_draw = mode in ('continuous-contention', 'partial-blit-contention')
        continuous = scoped_partial or mode in ('continuous', 'continuous-live', 'continuous-release', 'continuous-unmatched', 'continuous-contention', 'lock-contention', 'unknown-unlock', 'property-contention', 'release-contention')
        recovery = mode in ('nested-source-contention', 'pixel-miss-overflow')
        metadata_lost = mode in ('property-contention', 'release-contention')
        assert len(files) == (3 if thread_no_wait or recovery or mode in ('fill-update', 'many-surfaces') else 2 if metadata_lost else 16 if continuous else 1 if mode == 'descriptor-key-range' else 2 if accepted else 0), (mode, files)
        # Only source and sprite Locks are allowed; the destination is never Locked.
        locks = sorted(capture.glob('lock-*.bin'))
        assert len(locks) == (0 if mode in ('dc-format', 'dc-unmatched', 'dc-swapped') else 1 if mode in ('dc', 'dc-bottom-up', 'dc-retry', 'dc-rgb24', 'dc-rgb32') else 3 if recovery else 2 if scoped_partial else 16 if continuous else 3 if mode == 'many-surfaces' else 2 if accepted else 0 if mode == 'lock-description-change' else 1)
        bits = 24 if mode in ('rgb24', 'fill24', 'dc-rgb24') else 32 if mode in ('rgb32', 'fill32', 'dc-rgb32') else 16
        replays = []
        offset = mode in ('retry', 'nested-description', 'fill-failed', 'fill-partial')
        for index, source in enumerate(files, 1):
            predicted = oracle.cpu_replay(source.read_bytes())
            draw_index = index + offset + int(missed_draw and index > 2)
            if recovery and index == 3:
                draw_index = 5
            if thread_no_wait and index == 3:
                draw_index = 23
            expected = (case / f'original-{draw_index:08x}.bin').read_bytes()
            assert predicted == expected, (mode, index, 'CPU differs from original engine')
            if index == 1:
                data = source.read_bytes()
                at = 16
                # Check that the synthetic destination-before contains only zeros;
                # this is allowed solely because the first BLIT covers every pixel.
                for sid in (1, 2):
                    op, seq, length = struct.unpack_from('<III', data, at)
                    assert op == 1 and seq == sid
                    if sid == 2:
                        assert data[at + 12 + 28:at + 12 + length] == bytes(800 * 600 * (bits // 8))
                    at += 12 + length
            output = case / f'gpu-{index}.bin'
            result = subprocess.run(['xvfb-run', '-a', str(replay_binary), str(source),
                                     '--output', str(output)], env=env, capture_output=True, text=True, timeout=30)
            (case / f'gpu-{index}.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, result.returncode, result.stdout, result.stderr)
            report = json.loads(result.stdout)
            assert result.returncode == 0 and output.read_bytes() == expected, (mode, report, result.stderr)
            replays.append(report)
        events = (case / 'events.bin').read_bytes()
        frame_count, last_pixels = 0, b''
        published = {1, 2, 23} if thread_no_wait else set(range(1, 24)) if thread_case else {1, 2, 5} if recovery else set(range(1, 63)) if mode == 'continuous-live' else {1, 2, 3} if mode in ('fill-update', 'many-surfaces') else {1, 2} if metadata_lost else set(range(1, 24)) - {3} if missed_draw else set(range(1, 23)) if continuous else {1} if mode == 'descriptor-key-range' else set(range(1 + offset, 3 + offset)) if accepted and mode != 'offscreen' else set()
        for draw, count in struct.iter_unpack('<II', events):
            raw = (case / f'frame-{draw:08x}.bin').read_bytes()
            header = struct.unpack('<16I', raw[:64])
            if draw in published:
                frame_count += 1
                last_pixels = rgba((case / f'original-{draw:08x}.bin').read_bytes(), bits)
            assert count == frame_count and header[10] == frame_count and header[4] == frame_count * 2
            assert raw[64:] == last_pixels, (mode, draw, 'RGBA differs from original engine')
            if frame_count:
                assert header[5:10] == (800, 600, 3200, 1, 1)
        qt_readback = False
        if frame_count:
            result = subprocess.run(['xvfb-run', '-a', str(shell_binary), '--stream-test', str(stream)],
                                    env=env, capture_output=True, text=True, timeout=20)
            assert result.returncode == 0, (mode, result.stderr)
            qt_readback = True
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        if (scoped_partial and not thread_no_wait) or mode in ('continuous-live', 'fill24', 'fill32', 'fill-session'):
            assert 'primary_region_presented' in reasons and 'primary_region_failed' not in reasons
        if continuous and not metadata_lost and not thread_no_wait:
            assert frame_count == (23 if thread_case else 62 if mode == 'continuous-live' else 22) and 'blit_recording_limit' in reasons and 'unlock_limit' not in reasons
            assert 'blit_live_in_place' in reasons
        if (scoped_partial and not thread_case) or mode in ('continuous-contention', 'lock-contention'):
            assert 'pixel_tracker_contended' in reasons and 'pixel_target_reset' in reasons
            assert 'surface_pixels_epoch_reset' not in reasons
            assert 'surface_metadata_epoch_reset' not in reasons
        if recovery:
            assert frame_count == 3 and 'pixel_target_reset' in reasons
            if mode == 'pixel-miss-overflow':
                assert 'pixel_miss_overflow' in reasons and 'surface_pixels_epoch_reset' in reasons
            else:
                assert 'blit_invalidated' in reasons and 'surface_pixels_epoch_reset' not in reasons
        if mode == 'unknown-unlock':
            assert 'unlock_epoch_invalidated' in reasons and 'surface_pixels_epoch_reset' in reasons
            assert 'surface_metadata_epoch_reset' not in reasons
        if metadata_lost:
            assert frame_count == 2 and 'surface_metadata_epoch_reset' in reasons
        if mode == 'many-surfaces':
            assert 'surface_capacity' not in reasons and 'surface_metadata_epoch_reset' not in reasons
        if mode.startswith('dc'):
            assert 'dc_released' in reasons
            assert ('dc_checkpoint' in reasons) == accepted
            if mode == 'dc-format': assert 'dc_bitmap_rejected' in reasons
            if mode in ('dc-unmatched', 'dc-swapped'): assert 'dc_unmatched' in reasons
            if mode == 'dc-retry': assert 'dc_release_failed' in reasons
        if mode.startswith('fill'):
            assert 'fill_ready' in reasons
        if mode == 'fill-partial':
            assert 'fill_incomplete_initialization' in reasons
        if mode == 'fill-failed':
            assert 'blit_failed' in reasons
        if mode == 'fill-session':
            session = (capture / 'session-00000001.bin').read_bytes()
            assert session[:8] == b'MNMCMD01'
            at, records = 16, []
            while at < len(session):
                op, seq, length = struct.unpack_from('<3I', session, at)
                assert seq == len(records) + 1 and at + 12 + length <= len(session)
                records.append((op, session[at + 12:at + 12 + length]))
                at += 12 + length
            assert records[-1] == (9, struct.pack('<I', 6))
            assert [op for op, _ in records] == [1, 5, 9], records
        if mode == 'continuous-unmatched':
            assert 'unlock_target_invalidated' in reasons and 'unlock_epoch_invalidated' not in reasons
        if mode == 'descriptor-key':
            assert 'source_key_descriptor' in reasons
        if mode == 'descriptor-key-range':
            assert 'source_key_range' in reasons and 'blit_source_key_unobserved' in reasons
        if accepted:
            assert 'blit_initialized' in reasons and 'blit_propagated' in reasons
        else:
            assert 'blit_initialized' not in reasons
        timings = None
        if thread_case:
            ticks = struct.unpack('<21Q', (case / 'timing.bin').read_bytes())
            durations = [value * 1_000_000 / ticks[0] for value in ticks[1:]]
            assert ticks[0] and len(durations) == 20
            assert frame_count == (3 if thread_no_wait else 23)
            if mode == 'thread-timeout':
                assert 'tracker_wait_timeout' in reasons
            else:
                assert ('tracker_wait_acquired' in reasons) != thread_no_wait
            if thread_no_wait:
                assert 'pixel_target_reset' in reasons
            else:
                assert 'pixel_target_reset' not in reasons and 'tracker_wait_timeout' not in reasons
            timings = {'origin': 'synthetic_cross_thread_tracker_overlap', 'samples': 20,
                       'tracker_wait_ms': args.tracker_wait_ms if args.tracker_wait_ms is not None else 8,
                       'delivered_overlap_updates': 0 if thread_no_wait else 20,
                       'median_hooked_draw_us': statistics.median(durations),
                       'max_hooked_draw_us': max(durations), 'hooked_draw_us': durations}
        reports.append({'mode': mode, 'commands': len(files), 'frames': frame_count,
                        'destination_locks': 0, 'qt_readback': qt_readback, 'timings': timings, 'reasons': sorted(reasons), 'replays': replays})
        print(f'Bootstrap fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_unlocked_primaries',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Primary bootstrap passed: {root}')


if __name__ == '__main__':
    main()
