#!/usr/bin/env python3
"""Check packed unlock wire input against independent PE32 fixture and GPU pixels."""
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
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('live_fixture', ROOT / 'tools/test-live-render-channel.py')
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)
CASES = ['modern', 'legacy', 'surface2', 'surface7', 'negative', 'rgb24', 'rgb32',
         'indexed-palette', 'indexed-palette-change', 'retry', 'failed-lock', 'chain',
         'full-repeat', 'full-width', 'full-width-negative', 'one-row', 'budget',
         'no-base', 'readonly', 'discard', 'layout', 'changed', 'argument', 'invalidate',
         'session-bytes', 'replay-budget']
REFUSED = ['no-base', 'readonly', 'discard', 'layout', 'changed', 'argument', 'invalidate',
           'session-bytes', 'replay-budget']


def records(data):
    assert data[:16] == b'MNMCMD01' + struct.pack('<II', 1, 16)
    result, at = [], 16
    while at < len(data):
        op, sequence, length = struct.unpack_from('<III', data, at)
        assert sequence == len(result) + 1 and at + 12 + length <= len(data)
        result.append((op, data[at + 12:at + 12 + length]))
        at += 12 + length
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--case', choices=CASES, action='append')
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    sources = sorted((ROOT / 'runtime/render').glob('*.[ch]')) + [ROOT / p for p in [
        'tools/test-render-session-unlocks.py', 'tools/test-live-render-channel.py',
        'tools/test-render-partial-locks.py', 'tools/test-render-owned-session.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
        'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h', 'protocols/python/mnm_protocols/render_commands_v1.py']]
    fingerprints = {str(p.relative_to(ROOT)): live.sha(p) for p in sources}
    parent = ROOT / 'working/tests/render-session-unlocks'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    dll = live.load('unlock_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('unlock_stage', 'tools/prepare-shadow-experiment.py')
    oracle = live.load('unlock_oracle', 'tools/test-render-lock-blits.py')
    partial = live.load('unlock_partial', 'tools/test-render-partial-locks.py')
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT / 'working/tests/render-wine'))
    report = dict(schema=1, sources=fingerprints, scope='Bounded existing v1 UPDATE per admitted unlock '
                  'rectangle; independent fixture input and GPU pixels, no original-driver equivalence.', cases=[])
    for mode in args.case or CASES:
        valid = mode not in REFUSED
        case = run / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        channel, frame = case / 'commands.bin', case / 'frame.bin'
        header = bytearray(live.protocol.initial_header())
        struct.pack_into('<I', header, 16, 123)
        live.create(channel, header, live.protocol.SIZE)
        live.create(frame, live.frame_v1.initial_header(), live.frame_v1.SIZE)
        shutil.copyfile(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        active, output = case / 'producer.active', case / 'qt.json'
        child = dict(env, MNM_PARTIAL_LOCK_SELFTEST=mode, MNM_RENDER_OWNED_SESSION='1', MNM_COMMAND_TEST_DELAY='1',
                     MNM_RENDER_COMMAND_CHANNEL='Z:' + str(channel).replace('/', '\\'),
                     MNM_RENDER_STREAM='Z:' + str(frame).replace('/', '\\'),
                     MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        with (case / 'qt.log').open('w') as qlog, (case / 'wine.log').open('w') as wlog:
            qt = subprocess.Popen([str(args.build.resolve() / 'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env, stdout=qlog, stderr=qlog)
            try:
                live.wait_ready(Path(str(output) + '.ready'), qt)
                wine = subprocess.Popen(['wine', str(case / 'selftest.exe')], cwd=case, env=child,
                                        stdout=wlog, stderr=wlog)
                active.write_text(str(wine.pid))
                try:
                    code = wine.wait(timeout=45)
                    assert code == 0, (mode, code)
                finally:
                    if wine.poll() is None:
                        wine.terminate()
                        wine.wait(timeout=5)
                active.unlink()
                assert qt.wait(timeout=35) == (0 if valid else 8), mode
            finally:
                if qt.poll() is None:
                    qt.terminate()
                    qt.wait(timeout=5)
        observed = json.loads(output.read_text())
        mirror = capture / 'session-00000001.bin'
        session = mirror.read_bytes() if mirror.exists() else b''
        decoded = records(session) if session else []
        with channel.open('rb') as file:
            control = file.read(64)
            published = file.read(struct.unpack_from('<I', control, 20)[0])
        assert session[:len(published)] == published and len(decoded) <= 4096 and len(session) <= 64 * 1024 * 1024
        assert observed['success'] == valid and observed.get('native_readbacks', 0) == observed.get('rgba_readbacks', 0) == observed['viewport_uploads'] == observed.get('live_surfaces', 0) == 0
        uploads = [data for op, data in decoded if op == 2]
        verified_report = None
        if valid:
            bits = 8 if mode.startswith('indexed') else 24 if mode == 'rgb24' else 32 if mode == 'rgb32' else 16
            masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
            expected, natives, count = [], [], 0
            for original in sorted(case.glob('original-*.bin')):
                step = original.name[9:17]
                h = struct.unpack('<16I', (case / f'frame-{step}.bin').read_bytes()[:64])
                if h[10] > count:
                    native = original.read_bytes()
                    colors = (case / f'colors-{step}.bin').read_bytes() if bits == 8 else None
                    rgba = partial.resolved(native, bits, masks, colors)
                    if bits == 16:
                        # Native RGB565 presentation uses replicated bits (the
                        # independently committed renderer policy). Legacy
                        # frame-v1 conversion still floors mask scaling.
                        rgba = b''.join(bytes((((v>>11)<<3)|((v>>11)>>2),
                            (((v>>5)&63)<<2)|(((v>>5)&63)>>4),
                            ((v&31)<<3)|((v&31)>>2),255)) for v, in struct.iter_unpack('<H',native))
                    # Qt's framebuffer hash uses little-endian opaque QRgb.
                    qr = b''.join(bytes((rgba[i+2], rgba[i+1], rgba[i], rgba[i+3])) for i in range(0, len(rgba), 4))
                    expected.append(hashlib.sha256(qr).hexdigest())
                    natives.append(native)
                count = h[10]
            if mode == 'budget':
                expected, natives = expected[:16], natives[:16]
            assert observed['frames'] == expected, (mode, observed, expected)
            assert session == published and struct.unpack_from('<I', control, 24)[0] == 2 and decoded[-1][0] == 8
            # Initial full checkpoint is CREATE; each later unlock has one UPDATE.
            assert len(uploads) == len(natives) - 1
            for index, (data, native) in enumerate(zip(uploads, natives[1:])):
                x, y, width, height = (0, 0, 4, 4) if mode == 'full-repeat' else (0, 1, 4, 2) if mode.startswith('full-width') else (1, 1, 2, 1) if mode == 'one-row' else (0, 0, 2, 2) if mode == 'chain' and index else (1, 1, 2, 2)
                assert struct.unpack_from('<5I', data) == (1, x, y, width, height), mode
                crop = b''.join(native[((y+row)*4+x)*(bits//8):((y+row)*4+x+width)*(bits//8)] for row in range(height))
                assert data[20:] == crop, mode
            final = natives[-1]
            assert oracle.cpu_replay(session) == final
            native_file = case / 'native-verified.bin'
            verified = subprocess.run([str(args.build.resolve() / 'mnm-render-commands'),
                                       str(capture / 'session-00000001.bin'), '--output', str(native_file)],
                                      env=env, capture_output=True, text=True, timeout=30)
            (case / 'native-verify.log').write_text(verified.stdout + verified.stderr)
            assert verified.returncode == 0 and native_file.read_bytes() == final, (mode, verified.stderr)
            verified_report = json.loads(verified.stdout)
        else:
            if mode == 'no-base':
                assert not mirror.exists() and not published and not observed['frames']
                assert struct.unpack_from('<II', control, 24) == (3, 4)  # Interrupted before a complete base existed.
            else:
                assert struct.unpack_from('<II', control, 24) == (3, 2) and decoded[-1][0] == 9
                gap = struct.unpack('<I', decoded[-1][1])[0]
                assert gap == (2 if mode in ['session-bytes', 'replay-budget'] else 3), (mode, gap)
        report['cases'].append(dict(mode=mode, valid_session=valid, updates=len(uploads),
                                    commands=len(decoded), command_bytes=len(session), native_check_verify=verified_report, **observed))
        print(mode + ': passed', flush=True)
    assert all(live.sha(ROOT / p) == h for p, h in fingerprints.items()), 'Source changed during execution'
    report.update(success=True, full_frame_comparisons=sum(len(c['frames']) for c in report['cases']),
                  valid_sessions=sum(c['valid_session'] for c in report['cases']),
                  refused_sessions=sum(not c['valid_session'] for c in report['cases']),
                  ordinary_readbacks=0, viewport_uploads=0)
    (run / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Packed unlock sessions passed: ' + str(run / 'report.json'), flush=True)


if __name__ == '__main__':
    main()
