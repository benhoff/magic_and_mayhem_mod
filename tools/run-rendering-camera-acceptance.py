#!/usr/bin/env python3
"""Bounded physical camera input through the unchanged public native player."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import signal
import sqlite3
import struct
import subprocess
import sys
import time

from coverage_evidence import Reader, write_report
from native_only_contract import configure, outcome
from native_prefix_template import borrow
from rendering_pilot import require
from rendering_build_binding import native_runtime_source_paths

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-camera-matrix-results-20261106/'
PLAN = PREFIX + 'input-plan-01.json'


def module(name):
    s = importlib.util.spec_from_file_location('camera_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    m = importlib.util.module_from_spec(s)
    s.loader.exec_module(m)
    return m


menus = module('run-rendering-menu-acceptance')
sha = menus.sha


class LiveViewport:
    """Read complete appended records only; offline admission checks all bodies."""
    def __init__(self, directory):
        self.directory, self.path, self.at, self.index = directory, None, 8, 0
        self.paint, self.last = None, None

    def poll(self):
        paths = list(self.directory.glob('viewport-*.bin'))
        require(len(paths) <= 1, 'Ambiguous live viewport collector')
        if not paths:
            return self.last
        path = paths[0]
        require(self.path is None or self.path == path, 'Viewport collector changed')
        self.path = path
        with path.open('rb') as stream:
            if path.stat().st_size < 8:
                return self.last
            require(stream.read(8) == b'MNMACE01', 'Wrong live collector')
            while path.stat().st_size >= self.at + 8:
                stream.seek(self.at)
                m, n = struct.unpack('<II', stream.read(8))
                require(0 < m <= 65536 and n <= 33 * 1024**2, 'Live capture record budget')
                if path.stat().st_size < self.at + 8 + m + n:
                    break
                row = json.loads(stream.read(m))
                require(row['index'] == self.index, 'Live capture sequence gap')
                self.index += 1
                self.at += 8 + m + n
                if row['kind'] == 'paint':
                    self.paint = row
                elif row['kind'] == 'swap' and int(row['paint']):
                    require(self.paint and row['paint'] == self.paint['paint'], 'Live swap/paint identity')
                    if int(self.paint['world']) > 0:
                        self.last = self.paint
        return self.last


class CameraJourney(menus.Journey):
    def __init__(self, child, out, env, size):
        super().__init__(child, out, env)
        self.size, self.camera_actions = size, []
        self.viewport = LiveViewport(out / 'capture')

    def click_id(self, name):
        nodes = [v for _, v in self.access.nodes() if v.get('accessible_id', '').endswith('.' + name)]
        require(len(nodes) == 1 and nodes[0]['enabled'], 'Missing/disabled physical control: ' + name)
        self.actions.append(dict(action='click', object=name, host_ns=time.monotonic_ns(),
                                 physical=self.input.click(nodes[0]['rect'])))

    def wait_progress(self, after=0, minimum=5, timeout=30):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            require(self.child.poll() is None, 'Native player exited before camera progress')
            row = self.viewport.poll()
            if row:
                require([row['frame_width'], row['frame_height']] == self.size, 'Actual World size differs')
                if int(row['world']) >= after + minimum:
                    return row
            time.sleep(.05)
        raise ValueError('New native source/presentation progress deadline')

    def mark(self, action, phase):
        row = self.viewport.poll()
        require(row and int(row['world']) > 0 and self.input.focus()['host_focused'], 'Camera input lacks native output/focus')
        point = dict(action=action, phase=phase, accepted=True, host_us=str(time.monotonic_ns() // 1000),
                     paint=row['paint'], world=row['world'], actual_size=self.size, host_focus=self.input.focus())
        path = self.session / 'input-admission.bin'
        if path.exists():
            raw = path.read_bytes()
            require(len(raw) >= 160 and raw[:8] == b'MNMCAI01' and (len(raw) - 16) % 144 == 0,
                    'Missing/torn original input admission receipt')
            fields = struct.unpack_from('<36I', raw, len(raw) - 144)
            point.update(original_row=fields[0], original_tick=str(fields[1]), original_thread=fields[2], world_return_ordinal=fields[32])
        self.camera_actions.append(point)
        return point

    def held_key(self, action, symbol, hold=.18, settle=2.5, advance=True):
        self.mark(action, 'begin')
        code = self.input.x.XKeysymToKeycode(self.input.display, symbol)
        require(code > 0, 'Physical camera key absent')
        for down, delay in ((1, hold), (0, 0)):
            require(self.input.xt.XTestFakeKeyEvent(self.input.display, code, down, 0), 'Physical camera key refused')
            self.input.x.XFlush(self.input.display)
            time.sleep(delay)
        released = self.mark(action, 'released')
        self.actions.append(dict(action=action, symbol=symbol, keycode=code, held_ms=round(hold * 1000),
                                 down_up=True, host_ns=time.monotonic_ns(), host_focus=self.input.focus()))
        time.sleep(settle)
        if advance:
            self.wait_progress(int(released['world']), minimum=5)
        self.mark(action, 'settled')

    def run(self):
        self.input.activate()
        choices = [(n, v) for n, v in self.access.nodes() if v.get('accessible_id', '').endswith('.viewportScaling')]
        require(len(choices) == 1 and choices[0][1]['enabled'], 'Existing viewport scaling control is unavailable')
        self.input.click(choices[0][1]['rect'])
        self.key('viewport-scaling-integer', 0xff57)  # End: existing third Integer item.
        self.key('viewport-scaling-confirm', 0xff0d)
        deadline = time.monotonic() + 3
        selected_scaling = None
        while time.monotonic() < deadline:
            current = [v for _, v in self.access.nodes() if v.get('accessible_id', '').endswith('.viewportScaling')]
            if len(current) == 1 and current[0]['name'] == 'Integer (sharp)':
                selected_scaling = current[0]; break
            time.sleep(.1)
        require(selected_scaling is not None, 'Physical toolbar did not select Integer')
        self.actions.append(dict(action='selected-existing-integer-scaling', host_ns=time.monotonic_ns(),
                                 control=selected_scaling, host_focus=self.input.focus()))
        self.click('Launch game')
        self.wait(3, 0, timeout=420)
        self.capture('main-ready-initial', 'Quick Battle')
        self.click('Preferences')
        preferences = self.wait(10, 1)['preferences']
        self.key('preferences-cancel', 0xff1b)
        self.wait(3, 2)
        self.click('Quick Battle')
        self.wait(22, 3)
        self.click('Create Single Player Game')
        self.wait(14, 4)
        self.click('Map')
        self.wait(25, 5)
        lists = [(n, v) for n, v in self.access.nodes() if v['role'] == 'list']
        require(len(lists) == 1, 'Ambiguous physical map list')
        self.input.click(lists[0][1]['rect'])
        self.key('map-first', 0xff50)
        selected_map = lists[0][0].get_selection_iface().get_selected_child(0)
        require(selected_map is not None and selected_map.get_name() == self.last['maps'][0], 'Actual public map selection differs')
        self.key('map-confirm-existing-keyboard-route', 0xff0d)
        selected = self.wait(14, 6)
        require(selected['setup']['map'] == 1 and selected['setup']['map_name'] == 'Celtic Forest One (small)', 'Wrong admitted Quick map')
        self.click_id('singlePlayerStart')
        self.wait(7, 7, timeout=60)
        self.click_id('spellboxOK')
        self.wait_progress()
        for action, symbol, hold, settle in (
            ('center-first', ord('w'), .18, 1.), ('center-second', ord('w'), .18, 2.5),
            ('rotate-1', ord('.'), .18, 2.5), ('rotate-2', ord('.'), .18, 2.5),
            ('rotate-3', ord('.'), .18, 2.5), ('rotate-4', ord('.'), .18, 2.5),
            ('pan-up', 0xff52, .6, 2.5)):
            self.held_key(action, symbol, hold, settle)
        self.held_key('open-results', 0xff1b, .6, .2, advance=False)
        # Public sessions may expose the ordinary battle Mini first. Observe
        # the real ready screen and use its existing physical Quit Battle.
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            value = self.read()
            if value and value[0]['ready'] and value[0]['screen'] in (17, 26):
                break
            time.sleep(.05)
        require(value and value[0]['ready'] and value[0]['screen'] in (17, 26), 'No native Mini/Results after Escape')
        if value[0]['screen'] == 17:
            self.wait(17)
            self.click_id('miniMenuButton7')
        self.wait(26)
        self.click_id('quickResultAction2')
        self.wait(22)
        self.key('quick-return', 0xff1b)
        self.wait(3)
        self.capture('main-ready-returned', 'Quick Battle')
        self.click('Quit')
        return dict(preferences=preferences, map=selected['setup'], camera_actions=self.camera_actions)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--software', action='store_true')
    p.add_argument('--low', action='store_true')
    p.add_argument('--repair-of')
    p.add_argument('--repair-reason')
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned report required')
    require(bool(args.repair_of) == bool(args.repair_reason), 'Repair needs retained failure and concrete reason')
    require(not (ROOT / PREFIX / 'batch-stop-01.json').exists(),
            'Camera batch stopped after physical input misses; review batch-stop-01 before any further original run')
    out = ROOT / 'working/camera-acceptance' / Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').touch()
    size = [640, 480] if args.low else [800, 600]
    names = set(native_runtime_source_paths(ROOT)) | {'tools/run-rendering-camera-acceptance.py',
        'tools/run-rendering-menu-acceptance.py', 'tools/rendering_menu_accessibility.py',
        'tools/native_only_contract.py', 'tools/native_prefix_template.py', 'tools/run-native-owned.py',
        'tools/rendering_build_binding.py', 'tools/menu-game-runner.py'}
    with Reader(ROOT) as reader:
        plan = reader.json(PLAN)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['compiler_sources'].items()), 'Accepted player source drift')
        names.update(plan['compiler_sources'])
        binary = ROOT / plan['player_binary']['path']
        require(sha(binary) == plan['player_binary']['sha256'], 'Accepted player binary differs')
    sources = {n: sha(ROOT / n) for n in sorted(names)}
    result = dict(schema=1, kind='public_native_camera_acquisition', success=False, sources=sources,
        session_identity=secrets.token_hex(16), configuration=dict(software_flag=args.software, high_resolution=not args.low,
        actual_size=size, skip_intros=False, terrain_lighting='native', compiled_test_driver_invoked=False,
        original_state_samplers=True, viewport_scaling='integer', viewport_capture_policy='campaign-lossless-viewport-v1'),
        original_game_runs=0, original_pixels_used_as_native_inputs=False, player_binary=plan['player_binary'],
        compiler_sources=plan['compiler_sources'], temporary_root=str(out), scope=__doc__)
    result.update(repair_of=args.repair_of, repair_reason=args.repair_reason)
    hs = ls = child = journey = lease = None
    before = False
    env = {}
    try:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        menus.module('run-opengl-game.py').patch_staged_preferences(ROOT / 'working/game-nocd', 'VIDEO', {'IsHighRes': 'FALSE' if args.low else 'TRUE'})
        result['installation_inputs'] = {str(f.relative_to(ROOT / 'working/game-nocd')): dict(sha256=sha(f), bytes=f.stat().st_size)
                                         for f in sorted((ROOT / 'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw = prepare_launch_manifest(ROOT / 'working/game-nocd', ROOT / 'working/manifests/game-nocd.tsv')
        n = args.report.removesuffix('.json') + '-launch-manifest.bin'
        result['launch_manifest'] = dict(record=n, sha256=write_report(ROOT, n, raw))
        host, owner = out / 'host', out / 'owner'
        host.mkdir(); owner.mkdir()
        launcher = menus.module('run-native-owned.py')
        hs, hr, _ = launcher.start_display(host, allow_glx=True)
        ls, lr, cookie = launcher.start_display(owner)
        env = configure({k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}, ['--software-rendering'] if args.software else [])
        env.update(DISPLAY=hr['display'], XAUTHORITY=hr['authority'], MNM_NATIVE_OUTPUT_OWNER=str(owner / 'owner.json'),
            MNM_LEGACY_DISPLAY=lr['display'], MNM_LEGACY_XCOOKIE=cookie.hex(), MNM_MENU_GAME_SOURCE=str(ROOT / 'working/game-nocd'),
            MNM_MENU_DIRECTPLAY_DX6='1', MNM_CACHE_ADAPTERS='1', MNM_VERIFY_ORIGINALS='0', MNM_VERIFY_ASSETS='0',
            MNM_TERRAIN_LIGHTING='native', QT_QPA_PLATFORM='xcb', QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_ACCESSIBILITY='1',
            LP_NUM_THREADS='4', QT_SCALE_FACTOR='1', WINEDEBUG='-all', XDG_CONFIG_HOME=str(out / 'config'),
            MNM_RESIDENT_STAGE_PROFILE='1', MNM_RESIDENT_PROFILE_CAPTURE=str(out / 'worlds.bin'), MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',
            MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out / 'capture'), MNM_ACCEPTANCE_CAPTURE_ID=result['session_identity'],
            MNM_ACCEPTANCE_NORMAL='1', MNM_ACCEPTANCE_CAMPAIGN='1', MNM_ACCEPTANCE_GAMEPLAY='1')
        lease, result['prefix_pool'] = borrow(ROOT / 'working/fixtures/prefix-seed', out / 'wineprefix', env)
        env['MNM_OWNED_WINE_PREFIX'] = str(out / 'wineprefix')
        with sqlite3.connect(ROOT / 'working/camera-attempts.sqlite3') as budget:
            budget.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,repair_of TEXT,reason TEXT)')
            budget.execute('BEGIN IMMEDIATE')
            old = list(budget.execute('SELECT report,repair_of FROM attempts'))
            with Reader(ROOT) as r:
                review = r.json(PREFIX + 'integer-policy-and-cell-review-01.json')
                excluded = {v['record']: v['sha256'] for v in review['bounds']['verified_zero_game_preparation']}
                toolbar = r.json(PREFIX + 'toolbar-policy-repair-01.json')
                extra = toolbar['bounds']['additional_exact_zero_game_preparation']; excluded[extra['record']] = extra['sha256']
                for n, h in excluded.items():
                    require(r.digest(n) == h and r.json(n)['original_game_runs'] == 0 and r.json(n)['success'] is False, 'Preparation exclusion differs')
                originals = sum(r.json(n)['original_game_runs'] for n, _ in old)
                repairs = sum(bool(repair) and repair not in excluded and r.json(n)['original_game_runs'] > 0 for n, repair in old)
                extra_review = r.json(PREFIX + 'keyboard-map-repair-01.json')
                limits = extra_review['bounds']
                require(len(old) < limits['maximum_public_requests'] and originals < limits['maximum_original_games']
                        and (not args.repair_of or args.repair_of in excluded or repairs < limits['maximum_original_game_repairs']), 'Bounded camera attempt budget exhausted')
                resolved = {repair for n, repair in old if repair and r.json(n)['success']}
                for n, repair in old:
                    data = r.json(n)
                    require(data['success'] or n in excluded or n in resolved or args.repair_of == n, 'Stop on first failure; exact repair required')
                if args.repair_of:
                    require(r.json(args.repair_of)['success'] is False, 'Repair target did not fail')
            budget.execute('INSERT INTO attempts(report,repair_of,reason) VALUES(?,?,?)', (args.report, args.repair_of, args.repair_reason))
        command = [str(binary), '--repo', str(ROOT), '--renderer', 'opengl', '--live-menus', '--native-commands', '--resident-world', '--native-only']
        if args.software:
            command.append('--software-rendering')
        result['command'] = command
        with (out / 'qt.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update(DISPLAY=env['DISPLAY'], XAUTHORITY=env['XAUTHORITY'])
            journey = CameraJourney(child, out, env, size)
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                require(child.poll() is None, 'Qt host failed to open')
                if journey.input.find_window() and any(v['name'] == 'Launch game' for _, v in journey.access.nodes()):
                    break
                time.sleep(.1)
            result['proof'] = journey.run()
            require(result['proof']['preferences'][2] == int(args.low), 'Original High/Low selector differs')
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                try:
                    result['retirement'] = outcome(journey.session)
                    break
                except (ValueError, FileNotFoundError):
                    time.sleep(.1)
            require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit/owner retirement incomplete')
            result['resident'] = json.loads((journey.session / 'resident-world-report.json').read_text())
            result['native_admission'] = json.loads((journey.session / 'native-output-admission.json').read_text())
            child.wait(timeout=10)
            result['exit_code'] = child.returncode
            require(child.returncode == 0 and int(result['resident']['completed']) > 0, 'Native host failed normal completion')
        subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), check=True, timeout=10)
        result['private_wine_stopped'] = True
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
        if journey:
            try: journey.capture('failure-last-observation')
            except Exception as capture_error: result['failure_capture_error'] = str(capture_error)
    finally:
        if journey:
            result.update(states=journey.states, actions=journey.actions, camera_actions=journey.camera_actions,
                          screenshots=journey.screenshots, session=str(journey.session) if journey.session else None)
            journey.input.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try: child.wait(timeout=12)
            except subprocess.TimeoutExpired: os.killpg(child.pid, signal.SIGKILL); child.wait(timeout=10)
        if lease and not result['success']:
            subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), timeout=10)
            subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), timeout=10)
            result['forced_private_prefix_stop'] = True
        for server in (hs, ls):
            if server and server.poll() is None:
                server.terminate(); server.wait(timeout=10)
        if lease:
            try:
                from experiment_environment import _idle
                deadline = time.monotonic() + 20
                while True:
                    try: _idle(out / 'wineprefix', 'wineprefix'); break
                    except (ValueError, subprocess.TimeoutExpired):
                        if time.monotonic() >= deadline: raise
                        time.sleep(.2)
                lease.close(); result['prefix_pool']['returned'] = True
            except Exception as error:
                result.update(success=False, prefix_return_error=str(error))
        if before:
            try:
                subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
                result['original_manifest_verified_before_after'] = True
            except Exception as error:
                result.update(success=False, original_verification_error=str(error))
        if journey and journey.session:
            f = journey.session / 'original-process-result.json'
            result['original_game_runs'] = int(type(json.loads(f.read_text()).get('original_exit_code')) is int) if f.exists() else (1 if journey.states else None)
        result['sources_stable'] = all(sha(ROOT / n) == h for n, h in sources.items())
        result['player_binary_stable'] = sha(binary) == plan['player_binary']['sha256']
        result['success'] &= result['sources_stable'] and result['player_binary_stable']
        candidates = [f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name != 'Xauthority']
        if journey and journey.session:
            candidates += [f for f in journey.session.rglob('*') if f.is_file() and ('game' not in f.relative_to(journey.session).parts
                or f.suffix.lower() in ('.dll', '.cfg', '.sav', '.ini') or f.name.startswith('native-') and f.suffix == '.bin')]
        result['artifacts'] = []
        for f in sorted(set(candidates)):
            limit = 2 * 1024**3 if f == out / 'worlds.bin' else 1024**3 if f.parent == out / 'capture' else 512 * 1024**2
            if f.is_symlink() or f.stat().st_size > limit:
                continue
            n = args.report.removesuffix('.json') + '-raw-' + hashlib.sha256(str(f.relative_to(ROOT)).encode()).hexdigest()[:20] + '.bin'
            raw = f.read_bytes()
            result['artifacts'].append(dict(path=str(f.relative_to(ROOT)), record=n, sha256=write_report(ROOT, n, raw), bytes=len(raw)))
        h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
        print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                              original_game_runs=result['original_game_runs'], camera_markers=len(result.get('camera_actions', [])))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
