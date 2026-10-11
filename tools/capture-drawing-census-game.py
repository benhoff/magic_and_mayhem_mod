#!/usr/bin/env python3
"""Run original menu/campaign/Quick Battle journeys; retain every attempt."""
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
import shutil
import struct
import subprocess
import time
from drawing_census import ROOT, trace
from drawing_census_bindings import decode as core_bindings, physical as core_physical
from drawing_census_callers import review as core_caller_review
from drawing_api_journal import trace as api_trace
from drawing_api_details import trace as api_detail_trace
from drawing_api_resources import trace as api_resource_trace
from drawing_api_bindings import decode as api_bindings, physical as api_physical
from campaign_gameplay import read_rows, creatures
from campaign_spell import read_spell_rows


from drawing_api_cohorts import cohort, require_bindings, call_role


def capture(root, template, journey, claims=None):
    root = root.resolve()
    if (root / 'journey.json').exists(): raise ValueError('Attempt already recorded; prepare a fresh experiment')
    manifest = json.loads((root / 'manifest.json').read_text())
    metadata_paths=['manifest.json','census-build.json','menu-build.json']
    if manifest.get('api_observation_enabled',False):metadata_paths.append('api-build.json')
    frozen_metadata={p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in metadata_paths}
    selected=cohort(manifest)
    if journey == 'movies' and not manifest['movies_enabled']:
        raise ValueError('Movie journey requires explicitly enabled disposable preferences')
    if journey == 'campaign-effects' and not manifest.get('gameplay_observation_enabled', False):
        raise ValueError('Creature/effect journey requires opt-in passive original gameplay observation')
    declaration = json.loads(claims.read_text()) if claims else None
    if declaration:
        for p, digest in declaration['sources'].items():
            if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != digest: raise ValueError('Changed declaration source: ' + p)
    source_paths = ['tools/capture-drawing-census-game.py', 'tools/drawing_census.py', 'tools/drawing-census-game-runner.py',
                    'tools/campaign_gameplay.py', 'tools/campaign_spell.py']
    source_paths += ['tools/drawing_api_cohorts.py']
    if manifest.get('call_census_enabled',True):source_paths+=['tools/drawing_census_bindings.py','tools/drawing_census_callers.py']
    if manifest.get('api_observation_enabled', False): source_paths += ['tools/drawing_api_journal.py', 'tools/drawing_api_pairs.py', 'tools/drawing_api_details.py','tools/drawing_api_resources.py', 'tools/drawing_api_contracts.py','tools/drawing_api_surface_contracts.py','research/runtime/drawing-surface-api-layouts-20261011.json', 'tools/drawing_api_bindings.py']
    frozen_sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in source_paths}
    if declaration: frozen_sources.update(declaration['sources'])
    prefix = root / 'wineprefix'
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.pop('WAYLAND_DISPLAY', None)
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all,+seh' if selected=='menu_control' else '-all', LIBGL_ALWAYS_SOFTWARE='1', WINEDLLOVERRIDES='mscoree,mshtml=d',
               MNM_DRAW_CENSUS_EXPERIMENT=str(root))
    report = {'schema': 1, 'journey': journey, 'success': False, 'actions': [], 'phases': [],
              'scenario_closure': False, 'original_work_forwarded': True,
              'call_census_enabled': manifest.get('call_census_enabled', True),
              'gameplay_observation_enabled': manifest.get('gameplay_observation_enabled', False)}
    report['api_observation_enabled'] = manifest.get('api_observation_enabled', False)
    report['cohort']=selected
    report['call_boundary_role']=call_role(manifest)
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
        heartbeat = 1; sequence = 2; request = 0; phase_sequence = 2; observer_seen_at = None
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
            nonlocal observer_seen_at
            deadline = time.monotonic() + seconds
            while time.monotonic() < deadline:
                # Keep loader-time admission stable, then renew the lease while
                # original assets load. Waiting for Main can exceed its lease.
                events = root / 'events.bin'
                if observer_seen_at is None and events.exists() and events.stat().st_size >= 16:
                    observer_seen_at = time.monotonic()
                    report['observer_header_seen_at'] = observer_seen_at
                if observer_seen_at is not None and time.monotonic() - observer_seen_at >= .1: tick()
                row = state()
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
        def send(screen, action, argument=0, rules=None, slots=None):
            nonlocal heartbeat, sequence, request
            row = wait(screen); heartbeat += 1; sequence += 2; request += 1
            struct.pack_into('<I', channel, 16, sequence - 1)
            struct.pack_into('<6I', channel, 20, 1, heartbeat, request, action, row[0], argument)
            if rules is not None: struct.pack_into('<17I', channel, 44, *rules)
            if slots is not None: struct.pack_into('<63I', channel, 40000, *slots)
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
        def click(px, py, mouse_button=1):
            xt.XTestFakeMotionEvent(display, -1, px, py, 0); x.XFlush(display); hold(.2)
            xt.XTestFakeButtonEvent(display, mouse_button, 1, 0); x.XFlush(display); hold(.12)
            xt.XTestFakeButtonEvent(display, mouse_button, 0, 0); x.XFlush(display); hold(.3)
            report.setdefault('physical_inputs', []).append({'kind': 'click', 'x': px, 'y': py,
                'button': mouse_button, 'time': time.monotonic()})
        def hover(px, py, seconds=1):
            xt.XTestFakeMotionEvent(display, -1, px, py, 0); x.XFlush(display); hold(seconds)
            report.setdefault('physical_inputs', []).append({'kind': 'hover', 'x': px, 'y': py,
                'time': time.monotonic()})
        def button(config, section):
            cfg = configparser.ConfigParser(inline_comment_prefixes=(';',), interpolation=None)
            cfg.read(root / 'game/Interface' / config)
            r = list(map(int, cfg[section]['Rect2'].split(',')))
            click((r[0] + r[2]) // 2, (r[1] + r[3]) // 2)
        def key(sym):
            code = x.XKeysymToKeycode(display, sym)
            xt.XTestFakeKeyEvent(display, code, 1, 0); x.XFlush(display); hold(.35)
            xt.XTestFakeKeyEvent(display, code, 0, 0); x.XFlush(display); hold(.3)
            report.setdefault('physical_inputs', []).append({'kind': 'key', 'keysym': sym,
                'time': time.monotonic()})
        def heading(name, expected):
            shot = ImageGrab.grab(xdisplay=env['DISPLAY'])
            shot.save(root / (name + '.png'))
            cropped = root / (name + '-heading.png')
            shot.crop((230, 55, 575, 102)).save(cropped)
            text = subprocess.run(['tesseract', str(cropped), 'stdout', '--psm', '6'],
                capture_output=True, text=True, check=True, timeout=10).stdout.strip()
            report.setdefault('screen_checks', []).append({'capture': name, 'expected': expected, 'text': text})
            normalized = ''.join(char for char in text.casefold() if char.isalnum())
            if ''.join(char for char in expected.casefold() if char.isalnum()) not in normalized:
                raise RuntimeError('Original heading differs: ' + text)
        def filename():
            click(400, 171)
            ctrl = x.XKeysymToKeycode(display, 0xffe3)
            xt.XTestFakeKeyEvent(display, ctrl, 1, 0); x.XFlush(display)
            key(ord('a'))
            xt.XTestFakeKeyEvent(display, ctrl, 0, 0); x.XFlush(display)
            for char in 'draw01': key(ord(char))
            index = len(report.setdefault('filename_inputs', [])) + 1
            shot = root / ('filename-input-%02d.png' % index)
            ImageGrab.grab(xdisplay=env['DISPLAY']).save(shot)
            report['filename_inputs'].append({'requested': 'draw01', 'capture': shot.name})
        log = (root / 'game.log').open('x')
        process = subprocess.Popen([str(ROOT / 'tools/run-game.sh'), 'launch', '--no-gamescope',
            '--prefix', str(prefix), '--runner', str(ROOT / 'tools/drawing-census-game-runner.py')],
            env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        if manifest['movies_enabled']:
            phase(1, 'Enabled original startup movie requests; playback needs callee evidence')
        wait(3, seconds=180 if manifest['movies_enabled'] else 60)
        phase(2, 'Main and setup original menu traversal'); hold(1)
        send(3, 18); wait(10); hold(1); send(10, 20); wait(3)
        send(3, 1); send(22, 4); wait(14)
        rules = list(struct.unpack_from('<13I', channel, 168)) + [struct.unpack_from('<I', channel, 232 + i * 48)[0] for i in range(4)]
        send(14, 6, rules=rules); wait(25); hold(1); send(25, 10)
        send(14, 5); send(22, 2); wait(3)
        if journey == 'quick-battle':
            phase(4, 'Original Quick Battle setup, second offered map and unchanged rules')
            send(3, 1); send(22, 4); wait(14)
            rules = list(struct.unpack_from('<13I', channel, 168)) + [struct.unpack_from('<I', channel, 232 + i * 48)[0] for i in range(4)]
            send(14, 6, rules=rules); wait(25)
            map_count = struct.unpack_from('<I', channel, 164)[0]
            if map_count < 2: raise RuntimeError('Original second offered map unavailable')
            send(25, 9, 2); wait(14)
            def wire_text(offset, size):
                return bytes(channel[offset:offset + size]).split(b'\0', 1)[0].decode('cp1252')
            report['quick_setup'] = {'map': struct.unpack_from('<I', channel, 160)[0],
                'map_name': wire_text(412, 128), 'rules': rules,
                'players': [{'values': list(struct.unpack_from('<4I', channel, 220 + i * 48)),
                    'name': wire_text(236 + i * 48, 32)} for i in range(4)]}
            if report['quick_setup']['map'] != 2: raise RuntimeError('Original map selection differs')
            send(14, 7, rules=rules)
            deadline = time.monotonic() + 60
            while time.monotonic() < deadline:
                tick(); row = state()
                if row and (row[1] == 7 and row[2] or row[6] == 2): break
                if process.poll() is not None: raise RuntimeError('Original exited during Quick Battle handoff')
                time.sleep(.02)
            else: raise RuntimeError('Original Quick Battle handoff timeout: ' + str(state()))
            if row[1] == 7:
                wait(7)
                slots = list(struct.unpack_from('<63I', channel, 18000 + 28))
                report['quick_spell_selection'] = {'policy': 'Preserve original offered assignments',
                    'owner': struct.unpack_from('<I', channel, 18000)[0],
                    'counts': list(struct.unpack_from('<3I', channel, 18000 + 16)), 'slots': slots}
                send(7, 12, slots=slots)
            hold(6); focus_game(); phase(5, 'Original Quick Battle World; passive bounded activity'); hold(2)
            phase(6, 'Quick Battle camera arrow and recenter inputs')
            for sym in [0xff51, 0xff53, 0xff52, 0xff54, 0x20]: key(sym)
            phase(8, 'Quick Battle HUD/minimap hover inputs; semantic tooltip coverage requires review')
            for point in [(75, 75), (75, 525), (523, 575), (573, 575)]: hover(*point)
            phase(9, 'Quick Battle world cursor/click inputs'); click(350, 300); click(420, 350)
            # Original contexts 1/2 route Escape to results, not ordinary Mini.
            # Keep the independently experimental Quick Mini command lane disabled.
            phase(13, 'Quick Battle original Escape-to-results transition')
            focus_game(); key(0xff1b)
            wait(26); heading('original-quick-results', 'Game Over')
            report['quick_escape_transition'] = 'Original Escape-to-results; no Mini/Preferences claim'
            report['quick_results'] = {'header': list(struct.unpack_from('<4I', channel, 42100)),
                'rows': [{'active': struct.unpack_from('<I', channel, 42116 + i * 648)[0],
                    'portrait': struct.unpack_from('<I', channel, 42120 + i * 648)[0],
                    'texts': [wire_text(42124 + i * 648 + j * 128, 128) for j in range(5)]} for i in range(4)]}
            send(26, 16); hold(2)
            phase(5, 'Original Quick Battle Continue returns to World')
            focus_game(); key(0xff1b); wait(26)
            phase(13, 'Original second results visit and Quit to Quick Battle')
            heading('original-quick-results-second', 'Game Over')
            send(26, 17); wait(22); send(22, 2); wait(3)
        if journey.startswith('campaign'):
            phase(3, 'Fresh campaign ingress and all four difficulty choices')
            send(3, 22); wait(18)
            for choice in [0, 2, 3, 1]: send(18, 23, choice); hold(.4)
            send(18, 25); hold(6); focus_game(); phase(5, 'Original campaign world active'); click(400, 300); hold(2)
            if journey == 'campaign-effects':
                # Introductory Hermes dialogues consume ordinary cast inputs.
                # Check/dismiss through original mouse input before counting a cast.
                report['dialogue_checks'] = []
                quiet = 0
                dialogue_start = time.monotonic()
                for attempt in range(16):
                    name = 'effects-dialogue-%02d' % attempt
                    shot = ImageGrab.grab(xdisplay=env['DISPLAY']); shot.save(root / (name + '.png'))
                    crop = root / (name + '-text.png'); shot.crop((225, 90, 600, 270)).save(crop)
                    text = subprocess.run(['tesseract', str(crop), 'stdout', '--psm', '6'],
                        capture_output=True, text=True, check=True, timeout=10).stdout.strip()
                    visible = 'hermes' in text.casefold()
                    report['dialogue_checks'].append({'capture': name, 'text': text, 'hermes': visible})
                    quiet = 0 if visible else quiet + 1
                    if quiet >= 2 and time.monotonic() - dialogue_start >= 12: break
                    if visible: click(400, 160)
                    hold(1)
                else: raise RuntimeError('Original introductory Hermes dialogue did not clear')
                gameplay = root / 'gameplay-events.bin'
                prior_rows = read_rows(gameplay); prior = creatures(prior_rows)
                wizards = [a for a in prior.values() if a['type'] == 0 and a['active'] and a['health'] > 0]
                if len(wizards) != 1: raise RuntimeError('Expected unique living original campaign wizard')
                wizard = wizards[0]; owner = wizard['owner']
                phase(7, 'Original physical Zombie summon; require new living owned creature in passive pool')
                before_sequence = prior_rows[-1][0]
                click(523, 575); click(440, 310, 3)
                deadline = time.monotonic() + 12
                while time.monotonic() < deadline:
                    hold(.25); rows = read_rows(gameplay); current = creatures(rows)
                    summoned = [a for a in current.values() if a['type'] == 14 and a['owner'] == owner
                        and a['active'] and a['health'] > 0 and (a['slot'] not in prior
                        or not prior[a['slot']]['active'] or prior[a['slot']]['type'] != 14
                        or prior[a['slot']]['owner'] != owner)]
                    if summoned: break
                else: raise RuntimeError('Original physical cast created no living owned Zombie')
                report['original_summon'] = {'before_sequence': before_sequence, 'after_sequence': rows[-1][0],
                    'wizard': wizard, 'summoned': summoned, 'prior_creatures': list(prior.values()),
                    'observed': True, 'scope': 'Passive sampled original pool transition; no complete effect-to-pixel or lifetime proof'}
                ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / 'effects-summon-confirmed.png'); hold(2)
                phase(8, 'Original campaign HUD/spell/minimap tooltip hover inputs after observed summon')
                for point in [(523, 575), (573, 575), (695, 55)]: hover(*point)
                phase(7, 'Bounded original creature animation after observed summon'); hold(2)
            phase(6, 'Camera arrow input')
            for sym in [0xff51, 0xff53, 0xff52, 0xff54]: key(sym)
            phase(9, 'Cursor movement and world click'); click(350, 300); click(420, 350)
            phase(10, 'Original campaign Mini and Preferences'); focus_game()
            mini_visits = 0
            def open_mini():
                nonlocal mini_visits
                mini_visits += 1
                focus_game()
                for attempt in range(8):
                    # The original Hermes introduction can hold World input
                    # after camera motion or a menu return. Dismiss through
                    # original mouse input, then request Mini through Escape.
                    snapshot = 'before-mini-%02d-%02d.png' % (mini_visits, attempt)
                    ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / snapshot)
                    report.setdefault('mini_attempts', []).append({'visit': mini_visits, 'attempt': attempt, 'capture': snapshot})
                    click(400, 160)
                    key(0xff1b)
                    try: return wait(17, seconds=1)
                    except RuntimeError:
                        if process.poll() is not None: raise
                raise RuntimeError('Campaign Mini not ready')
            open_mini()
            if journey == 'campaign-save-load':
                phase(11, 'Original campaign save screen, file creation and load screen')
                before_files = {str(p.relative_to(root / 'game')) for p in (root / 'game').rglob('*') if p.is_file()}
                button('MiniMenu/screen (Mini Menu).cfg', 'TEXTBUTTON_2'); hold(1)
                heading('original-save-screen', 'Save Game'); filename()
                button('SaveGame/screen (Save Game).cfg', 'TEXTBUTTON_1'); hold(2)
                created = [p for p in (root / 'game').rglob('*') if p.is_file() and str(p.relative_to(root / 'game')) not in before_files]
                report['created_game_files'] = [{'path': str(p.relative_to(root / 'game')),
                    'bytes': p.stat().st_size, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()} for p in created]
                if not any('draw01' in p.name.lower() and p.stat().st_size for p in created):
                    raise RuntimeError('Original Save produced no named nonempty draw01 file')
                save_files = sorted((root / 'game/Save').glob('*.sav'))
                if len(save_files) != 1 or save_files[0].name.lower() != 'draw01.sav':
                    raise RuntimeError('Save fixture requires one unambiguous newly created draw01 row')
                retained = root / 'save-artifacts'; retained.mkdir()
                shutil.copy2(save_files[0], retained / save_files[0].name)
                open_mini(); button('MiniMenu/screen (Mini Menu).cfg', 'TEXTBUTTON_1'); hold(1)
                heading('original-load-screen', 'Load Game')
                # Original Load consumes list selection; typing alone does not
                # authorize a selected file. The private fixture has one row.
                click(400, 247)
                report['loaded_save_selection'] = {'path': 'Save/draw01.sav', 'click': [400, 247],
                    'sha256': hashlib.sha256(save_files[0].read_bytes()).hexdigest()}
                ImageGrab.grab(xdisplay=env['DISPLAY']).save(root / 'original-selected-load-row.png')
                phase(12, 'Original saved-map load and World drawing return')
                button('LoadGame/screen (Load Game).cfg', 'TEXTBUTTON_1'); hold(4); open_mini()
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
        if report['call_census_enabled']:
            census = trace(root / 'capture/census.bin', json.loads((root / 'census-build.json').read_text()),
                json.loads((ROOT / 'research/runtime/drawing-callers-movie-sites-20261010.json').read_text()),
                json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text()))
            (root / 'decoded.json').write_text(json.dumps(census, indent=2) + '\n')
            report.update(decoded_sha256=hashlib.sha256((root / 'decoded.json').read_bytes()).hexdigest(),
                          events=census['events'], calls=census['calls'])
        if report['call_census_enabled']:
            report['core_bindings']=core_bindings(root/'capture/core-bindings.bin',json.loads((root/'census-build.json').read_text()))
            (root/'decoded-core-bindings.json').write_text(json.dumps(report['core_bindings'],indent=2)+'\n')
            report['core_caller_review']=core_caller_review(root/'capture/census.bin',json.loads((root/'census-build.json').read_text()),root/'capture/core-bindings.bin',json.loads((ROOT/'research/runtime/drawing-callers-movie-sites-20261010.json').read_text()),json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text()))
            (root/'decoded-core-callers.json').write_text(json.dumps(report['core_caller_review'],indent=2)+'\n')
            core_callers=sorted({r['caller'] for r in census['entry_counts']})
            report['core_physical_caller_review']={'scope':'Exact per-run relocated E8 and named leave identities only; logical tail flow/original owners remain pending.','callers':[{'physical_caller':pc,'classification':core_physical(pc,report['core_bindings'])} for pc in core_callers]}
        if report['api_observation_enabled']:
            api_manifest = json.loads((root / 'api-build.json').read_text())
            if api_manifest.get('call_boundary_role','graphics_api')!=report['call_boundary_role']:raise RuntimeError('CALL target observer role differs from prepared cohort')
            if api_manifest.get('cohort','core_api')!=selected:raise RuntimeError('API manifest observation cohort mismatch')
            if api_manifest.get('synthetic_fixture',False):raise RuntimeError('Original journey cannot use synthetic API namespace')
            bindings = api_bindings(root / 'capture/api-bindings.bin', api_manifest)
            if report['call_census_enabled']:
                cb=report['core_bindings']
                if bindings['modules']['core']!=cb['core_module'] or bindings['core_leave']!=cb['core_leave']:raise ValueError('Independent core/API per-run module/leave identities disagree')
            api = (api_detail_trace if api_manifest.get('journal_version')==2 else api_trace)(root / 'capture/api-calls.bin', api_manifest)
            if api_manifest.get('journal_version')==2:
                report['api_resources']=api_resource_trace(root / 'capture/api-calls.bin',api_manifest)
                (root/'decoded-api-resources.json').write_text(json.dumps(report['api_resources'],indent=2)+'\n')
            if not api['paired_calls']: raise RuntimeError('No selected original API CALL was observed')
            require_bindings(bindings,selected)
            (root / 'decoded-api-bindings.json').write_text(json.dumps(bindings, indent=2) + '\n')
            (root / 'decoded-api.json').write_text(json.dumps(api, indent=2) + '\n')
            physical_callers = [{'physical_caller': pc, 'classification': api_physical(pc, bindings)}
                for pc in sorted({item['caller'] for item in census['entry_counts'] if item['caller']})] if report['call_census_enabled'] else []
            report['api'] = api
            report['api_bindings'] = bindings
            report['physical_caller_review'] = {'scope': 'Exact prepared per-run PCs/module membership only; original/logical owners and tail flow remain pending.', 'callers': physical_callers}
        if report['gameplay_observation_enabled']:
            gp = root / 'gameplay-events.bin'; sp = root / 'spell-events.bin'
            report['original_gameplay_trace'] = {'rows': len(read_rows(gp, complete=True)),
                'sha256': hashlib.sha256(gp.read_bytes()).hexdigest(), 'bytes': gp.stat().st_size,
                'scope': 'Bounded passive original samples; no protocol terminal, all-tick or exhaustive lifecycle claim'}
            report['original_spell_trace'] = {'rows': len(read_spell_rows(sp, complete=True)),
                'sha256': hashlib.sha256(sp.read_bytes()).hexdigest(), 'bytes': sp.stat().st_size,
                'scope': 'Bounded passive original spell callbacks; no all-path/whole-frame equivalence claim'}
        if process.returncode != 0: raise RuntimeError('Original process exited abnormally: ' + str(process.returncode))
        report.update(success=True, process_exit=process.returncode)
    except Exception as error:
        report['success'] = False
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
        report['metadata_sources']=frozen_metadata
        report['metadata_inputs_unchanged']=all(hashlib.sha256((root/p).read_bytes()).hexdigest()==digest for p,digest in frozen_metadata.items())
        report['sources_stable'] = all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == digest for p, digest in frozen_sources.items())
        if declaration: report['claims'] = declaration['claims']
        if not report['sources_stable'] or not report['inputs_unchanged'] or not report['metadata_inputs_unchanged']: report['success'] = False
        (root / 'journey.json').write_text(json.dumps(report, indent=2) + '\n')
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment', type=Path)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--journey', choices=['menus', 'campaign', 'campaign-save-load', 'campaign-effects', 'movies', 'quick-battle'], default='menus')
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    print(json.dumps(capture(args.experiment, args.prefix_template, args.journey, args.claims)), flush=True)
