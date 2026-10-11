#!/usr/bin/env python3
"""Run original menu/campaign journeys on a private display; retain every attempt."""
import argparse
import configparser
import ctypes as c
import hashlib
from PIL import ImageGrab
import json
import mmap
import os
from pathlib import Path
import signal
import struct
import subprocess
import time
from drawing_census import ROOT, trace


def capture(root, template, journey, claims=None):
    root = root.resolve()
    if (root / 'journey.json').exists(): raise ValueError('Attempt already recorded; prepare a fresh experiment')
    manifest = json.loads((root / 'manifest.json').read_text())
    declaration = json.loads(claims.read_text()) if claims else None
    if declaration:
        for p, digest in declaration['sources'].items():
            if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != digest: raise ValueError('Changed declaration source: ' + p)
    source_paths = ['tools/capture-drawing-census-game.py', 'tools/drawing_census.py', 'tools/drawing-census-game-runner.py']
    frozen_sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in source_paths}
    if declaration: frozen_sources.update(declaration['sources'])
    prefix = root / 'wineprefix'
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.pop('WAYLAND_DISPLAY', None)
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', LIBGL_ALWAYS_SOFTWARE='1', WINEDLLOVERRIDES='mscoree,mshtml=d',
               MNM_DRAW_CENSUS_EXPERIMENT=str(root))
    report = {'schema': 1, 'journey': journey, 'success': False, 'actions': [], 'phases': [],
              'scenario_closure': False, 'original_work_forwarded': True}
    server = process = display = channel = control = log = None
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        with (root / 'display').open('x+') as number_file, (root / 'xvfb.log').open('x') as out:
            server = subprocess.Popen(['Xvfb', '-displayfd', str(number_file.fileno()), '-screen', '0',
                '800x600x24', '-nolisten', 'tcp'], pass_fds=(number_file.fileno(),), stdout=out, stderr=subprocess.STDOUT)
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                number_file.seek(0); number = number_file.read().strip()
                if number: break
                if server.poll() is not None: raise RuntimeError('Private Xvfb exited')
                time.sleep(.1)
            else: raise RuntimeError('No private display')
        env['DISPLAY'] = ':' + number
        subprocess.run(['cp', '-a', '--reflink=auto', str(template.resolve()), str(prefix)], check=True)
        with (root / 'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (root / 'channel.bin').open('r+b') as f: channel = mmap.mmap(f.fileno(), 0)
        with (root / 'capture/census-control.bin').open('r+b') as f: control = mmap.mmap(f.fileno(), 0)
        heartbeat = 1; sequence = 2; request = 0; phase_sequence = 2
        def tick():
            nonlocal heartbeat, sequence
            heartbeat += 1; sequence += 2
            struct.pack_into('<I', channel, 16, sequence - 1)
            struct.pack_into('<I', channel, 24, heartbeat)
            struct.pack_into('<I', channel, 16, sequence)
        def state():
            before = struct.unpack_from('<I', channel, 128)[0]
            row = struct.unpack_from('<7I', channel, 132)
            return row if before and not before & 1 and before == struct.unpack_from('<I', channel, 128)[0] else None
        def wait(screen=None, ack=None, seconds=60):
            deadline = time.monotonic() + seconds
            while time.monotonic() < deadline:
                tick(); row = state()
                if row and (screen is None or row[1] == screen and row[2]) and (ack is None or row[3] == ack): return row
                if process.poll() is not None: raise RuntimeError('Original process exited before expected reply')
                time.sleep(.02)
            raise RuntimeError('Original screen/ack timeout: ' + str((screen, ack, state())))
        def hold(seconds):
            deadline = time.monotonic() + seconds
            while time.monotonic() < deadline:
                if process.poll() is not None: raise RuntimeError('Original exited during observation')
                tick(); time.sleep(.02)
        def phase(value, purpose):
            nonlocal phase_sequence
            phase_sequence += 2
            struct.pack_into('<I', control, 16, phase_sequence - 1)
            struct.pack_into('<I', control, 20, value)
            struct.pack_into('<I', control, 16, phase_sequence)
            report['phases'].append({'phase': value, 'purpose': purpose, 'time': time.monotonic()})
            ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / ('phase-%02d-%02d.png' % (value, len(report['phases']))))
        def send(screen, action, argument=0, rules=None):
            nonlocal heartbeat, sequence, request
            row = wait(screen); heartbeat += 1; sequence += 2; request += 1
            struct.pack_into('<I', channel, 16, sequence - 1)
            struct.pack_into('<6I', channel, 20, 1, heartbeat, request, action, row[0], argument)
            if rules is not None: struct.pack_into('<17I', channel, 44, *rules)
            struct.pack_into('<I', channel, 16, sequence)
            reply = wait(ack=request)
            report['actions'].append({'screen': screen, 'action': action, 'argument': argument,
                                      'request': request, 'reply': list(reply)})
            if reply[4] != 1: raise RuntimeError('Original action refused: ' + str(reply))
        x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
        x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
        x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
        x.XKeysymToKeycode.argtypes = [c.c_void_p, c.c_ulong]; x.XKeysymToKeycode.restype = c.c_uint
        xt.XTestFakeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_int, c.c_ulong]
        xt.XTestFakeButtonEvent.argtypes = xt.XTestFakeKeyEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
        display = x.XOpenDisplay(env['DISPLAY'].encode())
        if not display: raise RuntimeError('No private input display')
        x.XDefaultRootWindow.argtypes = [c.c_void_p]; x.XDefaultRootWindow.restype = c.c_ulong
        x.XQueryTree.argtypes = [c.c_void_p, c.c_ulong, c.POINTER(c.c_ulong), c.POINTER(c.c_ulong), c.POINTER(c.POINTER(c.c_ulong)), c.POINTER(c.c_uint)]
        x.XFetchName.argtypes = [c.c_void_p, c.c_ulong, c.POINTER(c.c_void_p)]; x.XFree.argtypes = [c.c_void_p]
        x.XSetInputFocus.argtypes = [c.c_void_p, c.c_ulong, c.c_int, c.c_ulong]
        def focus_game():
            pending = [x.XDefaultRootWindow(display)]; seen = set(); names = []
            while pending:
                window = pending.pop()
                if window in seen: continue
                seen.add(window)
                if len(seen) > 512: raise RuntimeError('Private X11 window budget')
                name = c.c_void_p(); label = ''
                if x.XFetchName(display, window, c.byref(name)) and name.value:
                    label = c.string_at(name.value).decode(errors='replace'); x.XFree(name)
                names.append({'window': window, 'label': label})
                if any(token in label.lower() for token in ['magic', 'mayhem', 'chaos']):
                    x.XSetInputFocus(display, window, 2, 0); x.XFlush(display)
                    report['input_window'] = {'window': window, 'label': label}; return
                parent = c.c_ulong(); returned = c.c_ulong(); children = c.POINTER(c.c_ulong)(); count = c.c_uint()
                if x.XQueryTree(display, window, c.byref(returned), c.byref(parent), c.byref(children), c.byref(count)):
                    pending.extend(children[i] for i in range(count.value))
                    if children: x.XFree(children)
            raise RuntimeError('Original game input window absent: ' + str(names))
        def click(px, py):
            xt.XTestFakeMotionEvent(display, -1, px, py, 0); x.XFlush(display); hold(.2)
            xt.XTestFakeButtonEvent(display, 1, 1, 0); x.XFlush(display); hold(.12)
            xt.XTestFakeButtonEvent(display, 1, 0, 0); x.XFlush(display); hold(.3)
        def button(config, section):
            cfg = configparser.ConfigParser(inline_comment_prefixes=(';',), interpolation=None)
            cfg.read(root / 'game/Interface' / config)
            r = list(map(int, cfg[section]['Rect2'].split(',')))
            click((r[0] + r[2]) // 2, (r[1] + r[3]) // 2)
        def key(sym):
            code = x.XKeysymToKeycode(display, sym)
            xt.XTestFakeKeyEvent(display, code, 1, 0); x.XFlush(display); hold(.35)
            xt.XTestFakeKeyEvent(display, code, 0, 0); x.XFlush(display); hold(.3)
        log = (root / 'game.log').open('x')
        process = subprocess.Popen([str(ROOT / 'tools/run-game.sh'), 'launch', '--no-gamescope',
            '--prefix', str(prefix), '--runner', str(ROOT / 'tools/drawing-census-game-runner.py')],
            env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        wait(3); phase(2, 'Main and setup original menu traversal'); hold(1)
        send(3, 18); wait(10); hold(1); send(10, 20); wait(3)
        send(3, 1); send(22, 4); wait(14)
        rules = list(struct.unpack_from('<13I', channel, 168)) + [struct.unpack_from('<I', channel, 232 + i * 48)[0] for i in range(4)]
        send(14, 6, rules=rules); wait(25); hold(1); send(25, 10)
        send(14, 5); send(22, 2); wait(3)
        if journey == 'campaign':
            phase(3, 'Fresh campaign ingress and all four difficulty choices')
            send(3, 22); wait(18)
            for choice in [0, 2, 3, 1]: send(18, 23, choice); hold(.4)
            send(18, 25); hold(6); focus_game(); phase(5, 'Original campaign world active'); click(400, 300); hold(2)
            phase(6, 'Camera arrow input')
            for sym in [0xff51, 0xff53, 0xff52, 0xff54]: key(sym)
            phase(9, 'Cursor movement and world click'); click(350, 300); click(420, 350)
            phase(10, 'Original campaign Mini and Preferences'); focus_game()
            def open_mini():
                focus_game()
                for attempt in range(8):
                    # The original Hermes introduction can hold World input
                    # after camera motion or a menu return. Dismiss through
                    # original mouse input, then request Mini through Escape.
                    ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / ('before-mini-%02d-%02d.png' % (request, attempt)))
                    click(400, 160)
                    key(0xff1b)
                    try: return wait(17, seconds=1)
                    except RuntimeError:
                        if process.poll() is not None: raise
                raise RuntimeError('Campaign Mini not ready')
            open_mini()
            send(17, 14); wait(10); hold(1); send(10, 20); hold(2)
            ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / 'after-campaign-preferences.png')
            open_mini()
            send(17, 15); hold(1); click(265, 395); wait(6); phase(13, 'Original campaign defeat result and return'); send(6, 26); wait(3)
        phase(14, 'Original Main quit and DLL terminal flush')
        # Quit can finish before its acknowledgement is published.
        row = wait(3); sequence += 2; request += 1
        struct.pack_into('<I', channel, 16, sequence - 1)
        struct.pack_into('<6I', channel, 20, 1, heartbeat + 1, request, 3, row[0], 0)
        struct.pack_into('<I', channel, 16, sequence)
        process.wait(timeout=30)
        census = trace(root / 'capture/census.bin', json.loads((root / 'census-build.json').read_text()),
            json.loads((ROOT / 'research/runtime/drawing-callers-dispatch-expanded-20261010.json').read_text()),
            json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text()))
        (root / 'decoded.json').write_text(json.dumps(census, indent=2) + '\n')
        report.update(success=True, process_exit=process.returncode, decoded_sha256=hashlib.sha256(
            (root / 'decoded.json').read_bytes()).hexdigest(), events=census['events'], calls=census['calls'])
    except Exception as error:
        report['error'] = str(error)
        if display:
            ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / 'failure.png')
            report['failure_menu_state'] = list(state()) if state() else None
        raise
    finally:
        if display: x.XCloseDisplay(display)
        if process is not None and process.poll() is None:
            os.killpg(process.pid, signal.SIGTERM)
            try: process.wait(timeout=5)
            except subprocess.TimeoutExpired: os.killpg(process.pid, signal.SIGKILL); process.wait()
        if prefix.exists(): subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False,
                                          stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if server is not None:
            server.terminate()
            try: server.wait(timeout=5)
            except subprocess.TimeoutExpired: server.kill(); server.wait()
        for resource in [channel, control, log]:
            if resource is not None: resource.close()
        report['inputs_unchanged'] = all(hashlib.sha256((root / 'game' / name).read_bytes()).hexdigest() == digest
                                        for name, digest in manifest['binaries'].items())
        report['sources'] = frozen_sources
        report['sources_stable'] = all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == digest for p, digest in frozen_sources.items())
        if declaration: report['claims'] = declaration['claims']
        if not report['sources_stable']: report['success'] = False
        (root / 'journey.json').write_text(json.dumps(report, indent=2) + '\n')
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment', type=Path)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--journey', choices=['menus', 'campaign'], default='menus')
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    print(json.dumps(capture(args.experiment, args.prefix_template, args.journey, args.claims)), flush=True)
