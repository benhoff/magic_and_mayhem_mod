#!/usr/bin/env python3
"""Observe a bounded original-game command session under an isolated Xvfb/Wine."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'protocols/python'))
from mnm_protocols import frame_v1, render_commands_v1, render_commands_v2


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def create(path, header, size):
    with path.open('xb') as out:
        out.write(header)
        out.truncate(size)


def stop(process):
    if process and process.poll() is None:
        os.killpg(process.pid, signal.SIGTERM)
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait(timeout=5)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path, help='Build containing live-render-channel-test')
    parser.add_argument('--wire-version', type=int, choices=[1,2], default=1)
    parser.add_argument('--seconds', type=int, default=20, choices=range(5, 31))
    parser.add_argument('--presentations', type=int, choices=range(1, 33),
                        help='Request a bounded successive-frame session instead of the ordinary sample')
    parser.add_argument('--prefix-template', type=Path, default=ROOT / 'working/wineprefix-x86_64')
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error("Run under xvfb-run -a -s '-screen 0 1280x1024x24'")
    protocol = render_commands_v2 if args.wire_version == 2 else render_commands_v1
    sources = sorted((ROOT / 'runtime/render').glob('*.[ch]'))
    sources += [ROOT / p for p in [
        'tools/test-live-render-game.py', 'tools/run-opengl-game.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'tests/live-render-channel-test.cpp', 'renderer/blit.cpp', 'renderer/blit.hpp',
        'renderer/commands.cpp', 'renderer/commands.hpp', 'renderer/command_state.hpp',
        'renderer/command_consumer.cpp', 'apps/qt-shell/command_channel.cpp',
        'apps/qt-shell/command_channel.hpp', 'apps/qt-shell/live_command_renderer.cpp',
        'apps/qt-shell/live_command_renderer.hpp', 'apps/qt-shell/gl_viewport.cpp',
        'apps/qt-shell/gl_viewport.hpp', 'protocols/include/mnm/render_commands_v1.h', 'protocols/include/mnm/render_commands_v2.h',
        'protocols/include/mnm/render_command_ring.h', 'protocols/python/mnm_protocols/render_commands_v2.py',
        'protocols/python/mnm_protocols/render_commands_v1.py',
        'protocols/python/mnm_protocols/frame_v1.py', 'runtime/shadow/win32_min.h']]
    fingerprints = {str(p.relative_to(ROOT)): sha(p) for p in sources}
    parent = ROOT / 'working/tests/live-render-game'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    qt = wine = None
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(run / 'wineprefix'))
    env.pop('WAYLAND_DISPLAY', None)
    try:
        subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()),
                        env['WINEPREFIX']], check=True)
        # A copied prefix needs Wine's host/drive initialization before the
        # bounded game timer starts; first explorer startup can exceed it.
        with (run / 'wineboot.log').open('w') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log,
                           stderr=log, timeout=90, check=True)
        frame, channel = run / 'frame.bin', run / 'commands.bin'
        create(frame, frame_v1.initial_header(), frame_v1.SIZE)
        header = bytearray(protocol.initial_header())
        struct.pack_into('<I', header, 16, 1)
        create(channel, header, protocol.SIZE)
        staged = subprocess.run([sys.executable, str(ROOT / 'tools/run-opengl-game.py'),
                                 '--stream', str(frame), '--command-channel', str(channel),
                                 '--capture-draws', '--skip-movies', '--stage-only'],
                                env=env, capture_output=True, text=True, check=True)
        (run / 'stage.log').write_text(staged.stdout + staged.stderr)
        experiment = Path(next(line.removeprefix('Render experiment: ')
                               for line in staged.stdout.splitlines()
                               if line.startswith('Render experiment: ')))
        metadata = json.loads((experiment / 'manifest.json').read_text())
        game = experiment / 'game'
        for name, key in [('Chaos.exe', 'staged_sha256'), ('MnmRender.dll', 'dll_sha256')]:
            if sha(game / name) != metadata[key]:
                raise RuntimeError('Staged binary changed: ' + name)
        for name, path in [('MNM_RENDER_STREAM', frame), ('MNM_RENDER_COMMAND_CHANNEL', channel),
                           ('MNM_RENDER_LOCK_CAPTURE_DIR', experiment / 'lock-capture'),
                           ('MNM_RENDER_CAPTURE_DIR', experiment / 'draw-capture'),
                           ('MNM_RENDER_FAILURE_LOG', experiment / 'surface-failures.log')]:
            env[name] = 'Z:' + str(path).replace('/', '\\')
        env['MNM_RENDER_OWNED_SESSION'] = '1'
        if args.presentations:
            env['MNM_RENDER_SESSION_PRESENTATIONS'] = str(args.presentations)
        active, output = run / 'producer.active', run / 'qt.json'
        with (run / 'qt.log').open('w') as qlog, (run / 'wine.log').open('w') as wlog:
            wine = subprocess.Popen(['wine', 'explorer', '/desktop=RenderShadow,800x600',
                                     str(game / 'Chaos.exe')], cwd=game, env=env,
                                    stdout=wlog, stderr=wlog, start_new_session=True)
            active.write_text(str(wine.pid))
            started = time.monotonic()
            deadline = started + 90
            while True:
                with channel.open('rb') as file:
                    file.seek(20)
                    published, state = struct.unpack('<II', file.read(8))
                if published or state == render_commands_v1.STATE_FAILED:
                    break
                if wine.poll() is not None or time.monotonic() > deadline:
                    subprocess.run(['import', '-window', 'root', str(run / 'startup.png')],
                                   env=env, timeout=10, check=False)
                    raise RuntimeError('Original startup produced no command activity; inspect wine.log')
                time.sleep(.1)
            startup_seconds = time.monotonic() - started
            qt = subprocess.Popen([str(args.build.resolve() / 'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env,
                                  stdout=qlog, stderr=qlog, start_new_session=True)
            deadline = time.monotonic() + 15
            while not Path(str(output) + '.ready').exists():
                if qt.poll() is not None or time.monotonic() > deadline:
                    raise RuntimeError('Native consumer did not become ready')
                time.sleep(.02)
            # Keep observing original drawing after a refused native stream.
            deadline = time.monotonic() + args.seconds
            while time.monotonic() < deadline and wine.poll() is None:
                time.sleep(.1)
            subprocess.run(['import', '-window', 'root', str(run / 'original-window.png')],
                           env=env, timeout=10, check=True)
            stop(wine)
            active.unlink(missing_ok=True)
            if qt.poll() is None:
                qt.wait(timeout=35)
        with channel.open('rb') as file:
            control = file.read(64)
            published = struct.unpack_from('<I', control, 20)[0]
            payload = file.read(published)
        mirror = experiment / 'lock-capture/session-00000001.bin'
        mirror_bytes = mirror.read_bytes() if mirror.exists() else b''
        if args.wire_version == 2:
            # Ring stores only the retained suffix; command archive stays exact.
            with channel.open('rb') as file:
                file.seek(64);ring_bytes = file.read(protocol.CAPACITY)
            start = max(0,published-protocol.CAPACITY)
            offset = start % protocol.CAPACITY
            retained = (ring_bytes[offset:]+ring_bytes[:offset])[:published-start]
            mirror_matches = retained == mirror_bytes[start:published]
            payload = mirror_bytes[:published]
        else:
            mirror_matches = mirror_bytes[:published] == payload
        qt_report = json.loads(output.read_text())
        diagnostics = (experiment / 'lock-capture/lifecycle.log').read_text()
        with frame.open('rb') as file:
            frame_header = struct.unpack('<16I', file.read(64))
        startup_fill_gap = (published == 0 and struct.unpack_from('<II', control, 24) == (3, 2)
                            and 'fill_ready ' in diagnostics and 'session_started ' not in diagnostics)
        records, at = [], 16
        while at + 12 <= len(payload):
            op, seq, length = struct.unpack_from('<III', payload, at)
            if at + 12 + length > len(payload):
                raise RuntimeError('Partial published command')
            records.append(dict(opcode=op, sequence=seq, bytes=length))
            at += 12 + length
        for name, key in [('Chaos.exe', 'staged_sha256'), ('MnmRender.dll', 'dll_sha256')]:
            if sha(game / name) != metadata[key]:
                raise RuntimeError('Staged binary changed during execution')
        if any(sha(ROOT / p) != h for p, h in fingerprints.items()):
            raise RuntimeError('Source changed during execution')
        report = dict(schema=1, success=True, wire_version=args.wire_version, scope='Bounded original-game startup observation; '
                      'original rendering retained. No independent pixel equivalence or replacement claim.',
                      sources=fingerprints, source_sha256=metadata['source_sha256'],
                      experiment=str(experiment.relative_to(ROOT)), seconds=args.seconds,
                      startup_seconds=startup_seconds,
                      producer_state=struct.unpack_from('<I', control, 24)[0],
                      producer_reason=struct.unpack_from('<I', control, 28)[0],
                      requested_presentations=args.presentations,
                      published_bytes=published, records=records, native_consumer=qt_report,
                      mirror_prefix_matches=mirror_matches,
                      startup_fill_gap=startup_fill_gap,
                      original_owned_frame_sequence=frame_header[4],
                      original_owned_frame_size=list(frame_header[5:7]),
                      lifecycle=diagnostics.splitlines(), original_drawing_retained=True,
                      pixel_equivalence=False, live_replacement=False,
                      consumer_binary_sha256=sha(args.build.resolve() / 'live-render-channel-test'))
        if not diagnostics or not (records or startup_fill_gap):
            raise RuntimeError('No original owned-surface command activity observed')
        report['artifacts'] = {str(p.relative_to(ROOT)): sha(p) for p in [
            run / 'qt.json', run / 'stage.log', run / 'wine.log', run / 'original-window.png',
            experiment / 'manifest.json', experiment / 'bridge-build.json',
            experiment / 'lock-capture/lifecycle.log']}
        (run / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(dict(report=str(run / 'report.json'),
                              native_session_complete=qt_report['success'],
                              presentations=qt_report['presentations'])), flush=True)
    finally:
        stop(qt)
        stop(wine)
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
