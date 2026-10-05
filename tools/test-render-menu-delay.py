#!/usr/bin/env python3
"""Compare Forest of Pain menu updates with Wine in an isolated X11 session."""
import ctypes as c
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
from PIL import Image, ImageChops, ImageGrab

REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / 'protocols/python'))
from mnm_protocols import frame_v1


def main():
    parent = REPO / 'working/tests/render-menu-delay'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    prefix = root / 'wineprefix'
    print(root, flush=True)
    subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)
    stream = root / 'frame.bin'
    with stream.open('wb') as f:
        f.write(frame_v1.initial_header())
        f.truncate(frame_v1.SIZE)
    with (root / 'stage.log').open('w') as log:
        subprocess.run([sys.executable, str(REPO / 'tools/run-opengl-game.py'),
                        '--stream', str(stream), '--stage-only', '--capture-locks', '--skip-movies'],
                       stdout=log, check=True)
    experiment = Path(next(line.removeprefix('Render experiment: ')
                           for line in (root / 'stage.log').read_text().splitlines()
                           if line.startswith('Render experiment: ')))
    metadata = json.loads((experiment / 'manifest.json').read_text())
    server = wine = display = None
    report = {'success': False, 'experiment': str(experiment), 'dll_sha256': metadata['dll_sha256'],
              'scope': 'Original Wine menu versus frame stream; Qt final-frame readback. Software X11, not live NVIDIA performance.'}
    try:
        with (root / 'display').open('w+') as f, (root / 'xvfb.log').open('w') as log:
            server = subprocess.Popen(['Xvfb', '-displayfd', str(f.fileno()), '-screen', '0',
                                       '800x600x24', '-nolisten', 'tcp'], pass_fds=(f.fileno(),),
                                      stdout=log, stderr=subprocess.STDOUT)
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                f.seek(0)
                number = f.read().strip()
                if number:
                    break
                time.sleep(.1)
            else:
                raise RuntimeError('Xvfb did not become ready')
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(DISPLAY=':' + number, WINEPREFIX=str(prefix), WINEDEBUG='-all',
                   LIBGL_ALWAYS_SOFTWARE='1', QT_QPA_PLATFORM='xcb',
                   MNM_RENDER_STREAM='Z:' + str(stream).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + metadata['lock_capture_directory'].replace('/', '\\'))
        env.pop('WAYLAND_DISPLAY', None)
        with (root / 'wine.log').open('w') as log:
            wine = subprocess.Popen(['wine', str(experiment / 'game/Chaos.exe')],
                                    cwd=experiment / 'game', env=env, stdout=log,
                                    stderr=subprocess.STDOUT, start_new_session=True)
        x = c.CDLL('libX11.so.6')
        xt = c.CDLL('libXtst.so.6')
        x.XOpenDisplay.argtypes = [c.c_char_p]
        x.XOpenDisplay.restype = c.c_void_p
        x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
        xt.XTestFakeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_int, c.c_ulong]
        xt.XTestFakeButtonEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
        display = x.XOpenDisplay(env['DISPLAY'].encode())
        if not display:
            raise RuntimeError('Cannot open test display')

        def frame():
            with stream.open('rb') as f:
                head = f.read(64)
                words = struct.unpack('<16I', head)
                if words[4] & 1 or not words[10] or words[5:8] != (800, 600, 3200):
                    return None, words[10]
                pixels = f.read(800 * 600 * 4)
                f.seek(16)
                if struct.unpack('<I', f.read(4))[0] != words[4]:
                    return None, words[10]
            return Image.frombytes('RGBA', (800, 600), pixels).convert('RGB'), words[10]

        def click(px, py):
            xt.XTestFakeMotionEvent(display, -1, px, py, 0)
            xt.XTestFakeButtonEvent(display, 1, 1, 0)
            xt.XTestFakeButtonEvent(display, 1, 0, 0)
            x.XFlush(display)
            return time.monotonic()

        def compare(name, clicked, old, rect, settle):
            time.sleep(settle)
            reference = ImageGrab.grab(xdisplay=env['DISPLAY']).convert('RGB')
            reference.save(root / (name + '-wine.png'))
            if max(hi for lo, hi in ImageChops.difference(old.crop(rect), reference.crop(rect)).getextrema()) < 32:
                raise RuntimeError('Original menu did not change: ' + name)
            deadline = clicked + 4
            while time.monotonic() < deadline:
                picture, count = frame()
                if picture is not None:
                    difference = ImageChops.difference(picture.crop(rect), reference.crop(rect))
                    maximum = max(hi for lo, hi in difference.getextrema())
                    # Wine display and native RGB565 conversion can differ by one
                    # channel level. A stale screen differs by far more than eight.
                    if maximum <= 8:
                        picture.save(root / (name + '-frame.png'))
                        return {'match_observed_within_seconds': time.monotonic() - clicked,
                                'roi': rect, 'max_channel_difference': maximum, 'published_count': count}
                time.sleep(.02)
            raise RuntimeError('Frame stream stayed on old menu: ' + name)

        deadline = time.monotonic() + 100
        while time.monotonic() < deadline:
            picture, count = frame()
            if picture is not None and count >= 30:
                break
            if wine.poll() is not None:
                raise RuntimeError('Game exited before menu')
            time.sleep(.1)
        else:
            raise RuntimeError('No continuously published main menu')
        time.sleep(1)
        old = ImageGrab.grab(xdisplay=env['DISPLAY']).convert('RGB')
        old.save(root / 'main-wine.png')
        report['forest_transition'] = compare('forest', click(400, 330), old, (45, 40, 220, 80), 2)
        old = ImageGrab.grab(xdisplay=env['DISPLAY']).convert('RGB')
        report['difficulty_change'] = compare('difficulty', click(234, 104), old, (45, 92, 620, 114), .3)
        samples = []
        start = time.monotonic()
        while time.monotonic() - start < 10:
            samples.append([time.monotonic() - start, frame()[1]])
            time.sleep(.1)
        report['idle_publication_samples'] = samples
        if samples[-1][1] - samples[0][1] < 10:
            raise RuntimeError('Menu publication stopped during idle observation')
        result = subprocess.run([str(REPO / 'working/build/qt-shell/mnm-qt-shell'), '--stream-test', str(stream)],
                                env=env, capture_output=True, text=True, timeout=20)
        (root / 'qt.log').write_text(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError('Qt frame readback failed')
        report.update(success=True, qt_readback=True)
        print('Forest of Pain transition and difficulty update passed', flush=True)
    finally:
        if display:
            x.XCloseDisplay(display)
        if wine is not None and wine.poll() is None:
            os.killpg(wine.pid, signal.SIGTERM)
            try:
                wine.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(wine.pid, signal.SIGKILL)
                wine.wait()
        subprocess.run(['wineserver', '-k'], env=dict(os.environ, WINEPREFIX=str(prefix)),
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False, timeout=10)
        if server is not None:
            server.terminate()
            server.wait(timeout=5)
        (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        (root / 'artifacts.json').write_text(json.dumps({p.name: hashlib.sha256(p.read_bytes()).hexdigest()
            for p in root.iterdir() if p.is_file()}, indent=2) + '\n')
        subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
