#!/usr/bin/env python3
"""One bounded Low startup/World/input/normal-Quit check; no pixel comparison."""
import argparse
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import signal
import sqlite3
import subprocess
import sys
import time
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from native_only_contract import configure, outcome
from native_prefix_template import borrow
from rendering_pilot import require
from rendering_transition_low_live import TransitionJourney
from rendering_pause import input_state, stable

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/native-startup-display-results-20261108/'
BID = 'HOST.private-display-auth-startup'
SID = 'native.private-display-auth-startup'


def module(name):
    spec = importlib.util.spec_from_file_location('startup_' + name, ROOT / 'tools' / (name + '.py'))
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded


def digest(path):
    with Path(path).open('rb') as file:
        return hashlib.file_digest(file, 'sha256').hexdigest()


class StartupProgress:
    """Original return/input readiness only; no GPU paint or pixel witness."""
    def __init__(self, journey):
        self.journey = journey
        self.last = self.paint = None

    def poll(self):
        journey = self.journey
        if not journey.session:
            return self.last
        path = journey.session / 'frame-input.bin'
        if not path.exists() or path.stat().st_size < 400:
            return self.last
        try:
            frame = journey.latest()['frame']
        except StopIteration:
            return self.last
        state = input_state(stable(journey.session/'render-input.bin', 1088, (16,)))
        if not state['active']:
            return self.last
        require(state['size'] == [640, 480], 'Wrong actual native input extent')
        nodes = [v for _, v in journey.access.nodes() if v.get('accessible_id', '').endswith('.nativeGameViewport')]
        if not nodes or not nodes[0]['showing']:
            return self.last
        require(len(nodes) == 1, 'Ambiguous startup viewport')
        width, height = nodes[0]['rect'][2:]
        scale = min(width/640, height/480)
        w, h = math.floor(640*scale+.5), math.floor(480*scale+.5)
        self.last = dict(world=str(frame['generation']), host_us=str(time.monotonic_ns()//1000),
            frame_width=640, frame_height=480, left=(width-w)//2, top=height-h-(height-h)//2,
            image_width=w, image_height=h, scaling=0,
            readiness='original-return/native-input-only-not-a-paint-witness')
        return self.last


class StartupJourney(TransitionJourney):
    def __init__(self, *args):
        super().__init__(*args)
        self.viewport = StartupProgress(self)

    def pointer(self, point):
        # The inherited helper requires captured-paint fields. This diagnostic
        # deliberately has no paint capture: use the observed public rectangle
        # and the same Sharp/input mapping without inventing a paint identity.
        current = self.viewport.poll()
        require(current is not None, 'No original/input startup readiness')
        nodes = [v for _, v in self.access.nodes() if v.get('accessible_id', '').endswith('.nativeGameViewport')]
        require(len(nodes) == 1 and nodes[0]['showing'], 'No unique visible startup viewport')
        rect = nodes[0]['rect']
        x, y = point
        require(0 <= x < 640 and 0 <= y < 480, 'Pointer outside actual input extent')
        px = int((x+.5)*current['image_width']/640) + current['left']
        py = int((y+.5)*current['image_height']/480) + current['top']
        require(self.input.xt.XTestFakeMotionEvent(self.input.display, -1, rect[0]+px, rect[1]+py, 0),
                'Physical startup pointer refused')
        self.input.x.XFlush(self.input.display)
        self.actions.append(dict(action='startup-pointer', logical=list(point),
            screen=[rect[0]+px, rect[1]+py], viewport=rect, original_generation=current['world'],
            mapping='native-input-extent/Sharp-no-paint-witness', host_ns=time.monotonic_ns()))

    def run(self):
        self.input.activate()
        self.click('Launch game')
        self.enter_world('startup')
        self.startup_observation = dict(main_ready=True, first_world=self.epochs[0]['first_world'])
        before = self.sample('startup-before-input')
        self.focus_viewport()
        self.physical_key('startup-gameplay-up', 0xff52)
        self.wait_progress(int(self.viewport.poll()['world']), minimum=3)
        after = self.sample('startup-after-input')
        require(after['input']['active'] == 1 and not after['input']['held']
                and after['input']['keys'][0x26] != before['input']['keys'][0x26]
                and after['original_world_return'] > before['original_world_return'],
                'Original/input progress did not resume')
        self.startup_observation['positive_input'] = dict(before=before, after=after)
        self.normal_quit()
        self.samples.close()
        self.samples = None
        return dict(main_ready=True, actual_size=self.size, first_world=self.epochs[0]['first_world'],
                    positive_input=dict(before=before, after=after), normal_quit_actions_completed=True,
                    controlled_faults=0, physical_restart=False)


def prepare(report):
    with Reader(ROOT) as reader:
        previous = PREFIX + 'after-controls-01.json'
        controls = reader.json(previous)
        require(controls['success'] and not controls['expected_pre_fix_failure']
                and [v['authenticated'] for v in controls['cases']] == [True, True, False]
                and all(reader.digest(n) == h for n, h in controls['sources'].items()),
                'Current private display controls required')
        parent_name = 'research/runtime/rendering-transition-recovery-results-20261107/closeout08-input-plan-02.json'
        parent = reader.json(parent_name)
        require(all(reader.digest(n) == h for n, h in parent['compiler_sources'].items())
                and all(reader.digest(n) == h for n, h in parent['runtime_sources'].items()),
                'Tested host/runtime phase changed')
        require(digest(ROOT / parent['player_binary']['path']) == parent['player_binary']['sha256'],
                'Exact tested player required')
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(v for v in register['behaviors'] if v['id'] == BID)
        scenario = next(v for v in register['scenarios'] if v['id'] == SID)
        sources = {n: reader.digest(n) for n in sorted(required_sources(behavior))}
        result = dict(schema=1, kind='bounded_native_startup_verification_plan', success=True,
            sources=sources, compiler_sources=parent['compiler_sources'], runtime_sources=parent['runtime_sources'],
            player_binary=parent['player_binary'], parent=dict(record=parent_name, sha256=reader.digest(parent_name)),
            controls=dict(record=previous, sha256=reader.digest(previous)),
            claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {}),
                         scenarios={SID: scenario_contract(scenario)})],
            bounds=dict(maximum_requests=1, maximum_original_games=1, deadline_seconds=300, retry=False),
            configuration=dict(size=[640, 480], software=False, movies_enabled=True,
                               physical_intro_skip=True, full_frame_capture=False, world_source_capture=False),
            scope='One missing startup observation after a reproduced authority visibility repair. Reuse the existing public Low entry, positive Up and ordinary Quit controls. No fault/restart interval, High recollection, pixel oracle, renderer acceptance or balance change.')
        return write_report(ROOT, report, (json.dumps(result, indent=2) + '\n').encode())


def run(plan_name, report):
    if os.environ.get('MNM_STARTUP_CHECK_BUS') != '1':
        runtime = ROOT / 'working/startup-check-runtime' / Path(report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        isolated = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_STARTUP_CHECK_BUS='1',
                        ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        isolated.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__,
                                      'run', '--plan', plan_name, '--report', report], isolated)
    with Reader(ROOT) as reader:
        plan = reader.json(plan_name)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['sources'].items()),
                'Frozen startup sources changed')
    binary = ROOT / plan['player_binary']['path']
    require(digest(binary) == plan['player_binary']['sha256'], 'Tested player bytes changed')
    out = ROOT / 'working/startup-display-check' / Path(report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').touch()
    # Single owned admission across zero-game setup failures too. This does not
    # reopen either stopped RT07/RT08 recovery budget.
    with sqlite3.connect(ROOT / 'working/startup-check-attempts.sqlite3') as db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY, plan TEXT)')
        db.execute('BEGIN IMMEDIATE')
        require(db.execute('SELECT COUNT(*) FROM requests').fetchone()[0] == 0, 'Startup request bound exhausted')
        db.execute('INSERT INTO requests VALUES(?,?)', (report, plan_name))
    result = dict(schema=1, kind='bounded_native_startup_verification', success=False,
        sources=plan['sources'], claims=plan['claims'], sources_stable=False, original_game_runs=0,
        plan=dict(record=plan_name), player_binary=plan['player_binary'], configuration=plan['configuration'],
        new_pixel_comparison=False, newly_passed_requirements=0, normal_quit=False)
    before = False
    servers = []
    child = journey = lease = None
    env = {}
    log_root = ROOT / 'working/logs'
    old_logs = set(log_root.glob('run-*'))
    def deadline(signum, frame):
        raise TimeoutError('Frozen300second startup check deadline')
    signal.signal(signal.SIGALRM, deadline)
    signal.alarm(300)
    try:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        module('run-opengl-game').patch_staged_preferences(ROOT / 'working/game-nocd', 'VIDEO', {'IsHighRes': 'FALSE'})
        menus = module('run-rendering-menu-acceptance')
        launcher = module('run-native-owned')
        for name, glx in (('host', True), ('owner', False)):
            directory = out / name
            directory.mkdir()
            server, receipt, cookie = launcher.start_display(directory, allow_glx=glx)
            servers.append(server)
            if name == 'host':
                host = receipt
            else:
                legacy = receipt
                legacy_cookie = cookie
        env = configure({k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}, [])
        env.update(DISPLAY=host['display'], XAUTHORITY=host['authority'], MNM_NATIVE_OUTPUT_OWNER=str(out/'owner/owner.json'),
            MNM_LEGACY_DISPLAY=legacy['display'], MNM_LEGACY_XCOOKIE=legacy_cookie.hex(),
            MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'), MNM_MENU_DIRECTPLAY_DX6='1', MNM_CACHE_ADAPTERS='1',
            MNM_VERIFY_ORIGINALS='0', MNM_VERIFY_ASSETS='0', MNM_TERRAIN_LIGHTING='native',
            QT_QPA_PLATFORM='xcb', QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_ACCESSIBILITY='1',
            LP_NUM_THREADS='4', QT_SCALE_FACTOR='1', WINEDEBUG='-all',
            XDG_CONFIG_HOME=str(out/'config'), XDG_CACHE_HOME=str(out/'cache'), XDG_RUNTIME_DIR=os.environ['XDG_RUNTIME_DIR'],
            MNM_RESIDENT_STAGE_PROFILE='1', MNM_ACCEPTANCE_GAMEPLAY='1', MNM_ACCEPTANCE_FRAME_INPUT='1',
            LIBGL_ALWAYS_SOFTWARE='1', __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
        prefix = ROOT / 'working/startup-check-prefix/wineprefix'
        prefix.parent.mkdir(parents=True, exist_ok=True)
        lease, result['prefix_pool'] = borrow(ROOT/'working/fixtures/prefix-seed', prefix, env)
        env['MNM_OWNED_WINE_PREFIX'] = str(prefix)
        argv = [str(binary), '--repo', str(ROOT), '--renderer', 'opengl', '--live-menus',
                '--native-commands', '--resident-world', '--native-only']
        result['command'] = argv
        with (out/'qt.log').open('xb') as log:
            child = subprocess.Popen(argv, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update({k: env[k] for k in ('DISPLAY', 'XAUTHORITY', 'XDG_RUNTIME_DIR', 'XDG_CONFIG_HOME', 'XDG_CACHE_HOME')})
            with (out/'accessibility-preflight.log').open('xb') as log:
                subprocess.run([sys.executable, '-c', "import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();assert Atspi.get_desktop(0) is not None"],
                               env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=20)
            journey = StartupJourney(child, out, env, [640, 480], None)
            end = time.monotonic() + 20
            while time.monotonic() < end:
                require(child.poll() is None, 'Qt startup failed')
                if journey.input.find_window() and any(v['name'] == 'Launch game' for _, v in journey.access.nodes()):
                    break
                time.sleep(.1)
            result['proof'] = journey.run()
            child.wait(timeout=30)
            result['exit_code'] = child.returncode
            result['outcome'] = outcome(journey.session)
            require(child.returncode == 0 and result['outcome']['state'] == 'completed', 'Ordinary Quit did not finish')
            result['resident'] = json.loads((journey.session/'resident-world-report.json').read_bytes())
            require(int(result['resident']['completed']) > 0, 'No native World progress')
            subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), check=True, timeout=10)
            result.update(success=True, normal_quit=True, private_wine_stopped=True)
    except Exception as error:
        result['error'] = str(error)
        if journey:
            try: journey.capture('startup-failure')
            except Exception as failure: result['failure_capture_error'] = str(failure)
        # A later cleanup refusal must never erase the primary observation.
        snapshot = dict(result, collection_complete=False,
            startup_observation=getattr(journey, 'startup_observation', None),
            states=journey.states if journey else [], actions=journey.actions if journey else [])
        record = report.removesuffix('.json') + '-failure-before-cleanup.json'
        result['failure_checkpoint'] = dict(record=record,
            sha256=write_report(ROOT, record, (json.dumps(snapshot, indent=2) + '\n').encode()))
    finally:
        signal.alarm(0)
        if journey:
            if journey.samples: journey.samples.close()
            result.update(states=journey.states, actions=journey.actions, screenshots=journey.screenshots,
                          session=str(journey.session) if journey.session else None,
                          startup_observation=getattr(journey, 'startup_observation', None))
            journey.input.close()
            journey.source_frames.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try: child.wait(timeout=12)
            except subprocess.TimeoutExpired: os.killpg(child.pid, signal.SIGKILL); child.wait(timeout=10)
        if lease and not result['success']:
            subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
            subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
            result['forced_prefix_stop'] = True
        for server in reversed(servers):
            if server.poll() is None: server.terminate(); server.wait(timeout=10)
        if lease:
            from experiment_environment import _idle
            try:
                end = time.monotonic() + 20
                while True:
                    try:
                        _idle(prefix, 'wineprefix')
                        break
                    except (ValueError, subprocess.TimeoutExpired):
                        if time.monotonic() >= end:
                            raise
                        time.sleep(.2)
                lease.close()
                result['prefix_pool']['returned'] = True
            except Exception as error:
                result.update(success=False, prefix_return_error=str(error))
        if before:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            result['original_manifest_verified_before_after'] = True
        admissions = [json.loads(f.read_bytes()) for f in (out/'owner/sessions').glob('*.json')]
        result['original_game_runs'] = len(admissions)
        require(result['original_game_runs'] <= 1, 'Original startup bound exceeded')
        candidates = [f for f in out.rglob('*') if f.is_file() and f.name != 'Xauthority']
        for a in admissions:
            session = Path(a['session'])
            candidates += [f for f in session.iterdir() if f.is_file() and f.suffix in ('.json', '.log', '.txt', '.cmd')]
            candidates += [f for f in session.iterdir() if f.is_file() and f.suffix == '.bin' and f.stat().st_size <= 4*1024**2]
            candidates += [f for f in (session/'game').iterdir() if f.is_file() and f.suffix.lower() in ('.dll', '.exe')]
        for directory in set(log_root.glob('run-*')) - old_logs:
            candidates += [f for f in directory.iterdir() if f.is_file()]
        result['artifacts'] = []
        for file in sorted(set(candidates)):
            require(file.stat().st_size <= 8*1024**2, 'Startup diagnostic exceeded its byte bound')
            record = report.removesuffix('.json') + '-raw-' + hashlib.sha256(str(file.relative_to(ROOT)).encode()).hexdigest()[:20] + '.bin'
            raw = file.read_bytes()
            result['artifacts'].append(dict(path=str(file.relative_to(ROOT)), record=record,
                sha256=write_report(ROOT, record, raw), bytes=len(raw)))
        with Reader(ROOT) as reader:
            result['sources_stable'] = all(reader.digest(n) == h for n, h in plan['sources'].items())
        result['success'] &= result['sources_stable']
        sha = write_report(ROOT, report, (json.dumps(result, indent=2) + '\n').encode())
        print(json.dumps(dict(success=result['success'], error=result.get('error'), original_game_runs=result['original_game_runs'], sha256=sha)), flush=True)
    return int(not result['success'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('prepare', 'run'))
    parser.add_argument('--plan')
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'Fresh owned report required')
    if args.mode == 'prepare':
        print(prepare(args.report), flush=True)
        return 0
    require(args.plan is not None, 'A frozen plan is required')
    return run(args.plan, args.report)


if __name__ == '__main__':
    raise SystemExit(main())
