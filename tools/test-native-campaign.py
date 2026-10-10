#!/usr/bin/env python3
"""Require native menus and native command presentation in one public-shell campaign.

A rejected mode, menu retirement, original-window fallback, missing native frames,
or incomplete click-through is a failed test. --check-only builds the public shell
and probes the combined flags without launching a game or reading game media.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import struct
import subprocess
import time
from campaign_gameplay import read_rows, creatures, damage_rows

ROOT = Path(__file__).resolve().parents[1]
STEPS = ['main', 'region', 'campaign-handoff', 'gameplay', 'campaign-menu', 'gameplay-resumed']
SOURCES = ['tools/test-native-campaign.py', 'tools/run-qt-shell.sh',
           'apps/qt-shell/campaign_smoke_test.cpp', 'apps/qt-shell/campaign_smoke_test.hpp',
           'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt',
           'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/menu_bridge.cpp',
           'tools/test-live-campaign-mini.py', 'tools/campaign-gameplay-input.py',
           'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_command_session.cpp',
           'apps/qt-shell/live_command_session.hpp', 'tools/prepare-menu-observer.py',
           'tools/run-menu-observer.py', 'tools/menu-game-runner.py', 'tools/profile-render-stream.py',
           'runtime/menu/campaign_gameplay_observe.h', 'tools/campaign_gameplay.py', 'tools/campaign-combat-input.py']


def validate_casting_combat(experiment, inputs):
    rows=read_rows(experiment/'gameplay-events.bin', complete=True)
    summon=inputs['summon']; before=summon['before_sequence']; after=summon['after_sequence']; slot=summon['slot']; owner=inputs['player_owner']
    initial=creatures([r for r in rows if r[0]<=before])
    later=creatures([r for r in rows if before<r[0]<=after])
    if slot in initial and initial[slot]['active']:
        raise ValueError('Summon slot was already active before the cast')
    zombie=later.get(slot)
    if not zombie or zombie['type']!=14 or zombie['owner']!=owner or not zombie['active'] or zombie['health']<=0:
        raise ValueError('No newly living player Zombie after native summon')
    captures={c['name']:c for c in inputs['captures']}
    for key,count in [('before_capture','0/15'),('after_capture','1/15')]:
        capture=captures[summon[key]]
        if capture['ocr']!={'native':count,'original':count} or not capture['metadata']['native_saved'] or not capture['metadata']['original_saved'] or capture['metadata']['fallback']:
            raise ValueError('Missing independent native/original summon count')
    hits=damage_rows(rows,owner,[slot],after)
    if not hits:
        raise ValueError('No observed original player Zombie versus enemy melee damage')
    return dict(casting_verified=True,combat_verified=True,summoned_slot=slot,player_owner=owner,
                melee_damage_events=len(hits),first_melee_damage=list(hits[0]),trace_sha256=sha(experiment/'gameplay-events.bin'))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_flow(flow, cycles=1, stress_seconds=0, min_fps=20):
    if flow.get('success') is not True:
        raise ValueError(flow.get('error') or 'Campaign driver did not complete')
    if flow.get('native_command_fallback') is not False:
        raise ValueError('Native rendering fallback or missing fallback evidence')
    if type(flow.get('native_command_frames')) is not int or flow['native_command_frames'] < 6:
        raise ValueError('Insufficient native command presentations')
    steps = flow.get('steps', [])
    expected = STEPS + ['campaign-menu', 'gameplay-resumed']*(cycles-1)
    if [s.get('step') for s in steps] != expected:
        raise ValueError('Incomplete campaign click-through')
    if any(s.get('screenshot_saved') is not True for s in steps if s['step'] != 'campaign-handoff'):
        raise ValueError('Missing campaign screenshots')
    if stress_seconds:
        rates = flow.get('gameplay_rates', [])
        for cycle in range(cycles+1):
            samples = [r for r in rates if r.get('cycle') == cycle]
            seconds = sum(r.get('seconds', 0) for r in samples)
            if seconds < stress_seconds-2:raise ValueError('Incomplete gameplay performance sample')
            native_fps = sum(r['native_frames'] for r in samples)/seconds
            paint_fps = sum(r['painted_frames'] for r in samples)/seconds
            if min(native_fps, paint_fps) < min_fps or any(min(r['native_fps'], r['paint_fps']) < min_fps/2 or r['seconds'] > 2 for r in samples):
                raise ValueError(f'Gameplay too slow in cycle {cycle}: native {native_fps:.1f}, paints {paint_fps:.1f} FPS; minimum {min_fps}')


def world_ready(experiment):
    path = experiment / 'events.bin'
    if not path.exists():return False
    data = path.read_bytes()
    if len(data) < 16 or data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64):return False
    # A writer may be appending its final record; only inspect complete rows.
    end = 16 + ((len(data)-16)//64)*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    ticks = [r for r in rows if r[1] == 12 and r[3] == 2 and r[6] == 1 and r[11] == 0x6cbb78]
    return len(ticks) >= 3 and len({r[2] for r in ticks}) == 1


def validate_events(experiment, cycles=1, difficulty=0):
    """Independent engine-side checks; window appearance alone cannot pass."""
    data = (experiment / 'events.bin').read_bytes()
    if data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64) or (len(data)-16) % 64:
        raise ValueError('Invalid original callback observation')
    rows = list(struct.iter_unpack('<16I', data[16:]))
    if len(rows) > 256 or [r[0] for r in rows] != list(range(1, len(rows)+1)):
        raise ValueError('Invalid original callback sequence')
    actions = [(r[3], r[9]) for r in rows if r[1] == 3]
    expected=[(3, 0)] + ([(18,65536+difficulty)] if difficulty else []) + [(18,0)] + [(17,4)]*cycles
    if actions != expected:
        raise ValueError('Unexpected New Game/Enter/Cancel callback trace: ' + str(actions))
    ticks = [r for r in rows if r[1] == 12 and r[3] == 2 and r[6] == 1 and r[11] == 0x6cbb78]
    cancels = [r for r in rows if r[1] == 3 and r[3] == 17 and r[9] == 4]
    resumes = [r for r in rows if r[1] == 20 and r[11] == 0x6cbb78 and r[0] > cancels[0][0]]
    if len(ticks) < 3 or len(resumes)<cycles or len({r[2] for r in rows if r[1] in (3, 12, 20)}) != 1:
        raise ValueError('Missing same-thread original World updates/Cancel resume')
    channel = (experiment / 'channel.bin').read_bytes()
    if channel[:16] != b'MNMMCM12' + struct.pack('<II', 12, 106496) or len(channel) != 106496:
        raise ValueError('Campaign test did not use the ordinary V12 menu protocol')
    if struct.unpack_from('<I', channel, 36*4)[0] != 2+cycles+bool(difficulty):
        raise ValueError('Missing third menu acknowledgement')
    return {'actions': actions, 'world_ticks': len(ticks), 'world_resumes': len(resumes),
            'protocol_version': 12, 'events_sha256': sha(experiment / 'events.bin')}


def stop_group(process):
    if process is None or process.poll() is not None:
        return
    try:
        os.killpg(process.pid, signal.SIGTERM)
        process.wait(timeout=5)
    except ProcessLookupError:
        return
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait(timeout=5)


def clean_environment(root):
    # Diagnostic overrides can silently select another rendering route.
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(XDG_CONFIG_HOME=str(root / 'config'), WINEPREFIX=str(root / 'wineprefix'),
               QT_QPA_PLATFORM='xcb', PYTHONDONTWRITEBYTECODE='1')
    env.pop('WAYLAND_DISPLAY', None)
    return env


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stress-seconds', type=int, default=0, help='Exercise physical gameplay input per phase (0 or 10..120)')
    parser.add_argument('--menu-cycles', type=int, choices=range(1,6), default=1, help='Number of native Escape/Mini/Cancel returns')
    parser.add_argument('--min-fps', type=float, default=20, help='Minimum average native publications and visible Qt paints during stress')
    parser.add_argument('--difficulty', type=int, choices=range(4), default=0, help='Choose the normal Region Entry difficulty via its native radio (0 Initiate; 1 Apprentice for camera/combat stress)')
    parser.add_argument('--require-casting-combat', action='store_true', help='Require successful native-viewport summon and player-versus-enemy melee damage observation')
    parser.add_argument('--claims', type=Path, help='Prospective coverage scope/source declaration prepared before this run')
    parser.add_argument('--check-only', action='store_true', help='Build and check combined-mode admission; no game launch')
    parser.add_argument('--display', help='Use this X11/XWayland display instead of a private Xvfb (moves focus and sends Escape)')
    parser.add_argument('--timeout', type=int, default=330, help='Bound live automation, in seconds (30..600)')
    args = parser.parse_args()
    if args.require_casting_combat:
        if args.difficulty not in (1,3):parser.error('Casting/combat requires --difficulty 1 or 3 (other campaign journeys remain separate)')
        if not args.stress_seconds:args.stress_seconds=60
    if not 30 <= args.timeout <= 600:
        parser.error('--timeout must be between 30 and 600')
    if args.stress_seconds and not 10 <= args.stress_seconds <= 120:parser.error('--stress-seconds must be 0 or 10..120')
    if not 1 <= args.min_fps <= 120:parser.error('--min-fps must be 1..120')
    parent = ROOT / 'working/tests/native-campaign'
    parent.mkdir(parents=True, exist_ok=True)
    import tempfile
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('Campaign test evidence: ' + str(out), flush=True)
    report = {'schema': 1, 'success': False, 'status': 'running', 'stage': 'build',
              'required_mode': ['native-menus', 'native-command-presentation'],
              'launch_requested': False, 'game_entry_verified': False, 'flow_completed': False, 'movement_verified': False,
              'complete_drawing_replacement': False, 'cleanup': 'bounded private-session termination',
              'casting_combat_required':args.require_casting_combat,'casting_verified':False,'combat_verified':False,
              'difficulty': args.difficulty, 'stress_seconds_per_phase': args.stress_seconds, 'menu_cycles': args.menu_cycles, 'minimum_fps': args.min_fps,
              'sources': {p: sha(ROOT / p) for p in SOURCES}}
    if args.claims:
        declaration=json.loads(args.claims.read_text())
        for path,expected in declaration['sources'].items():
            if Path(path).is_absolute() or not (ROOT/path).resolve().is_relative_to(ROOT):raise ValueError('Unsafe declared source')
            if sha(ROOT/path)!=expected:raise ValueError('Declared source changed before execution: '+path)
        report['claims']=declaration['claims'];report['sources'].update(declaration['sources'])
    env = clean_environment(out)
    shell = server = monitor = None
    verified = False
    exit_code = 1
    try:
        with (out / 'build.log').open('w') as log:
            subprocess.run([str(ROOT / 'tools/run-qt-shell.sh'), '--help'], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        binary = ROOT / 'working/build/qt-shell/mnm-qt-shell'
        report['shell_sha256'] = sha(binary)
        cache=(ROOT/'working/build/qt-shell/CMakeCache.txt').read_text().splitlines()
        report['native_build_type']=next((v.split('=',1)[1] for v in cache if v.startswith('CMAKE_BUILD_TYPE:STRING=')),None)
        report['stage'] = 'combined-mode-admission'
        # A smoke probe opens/closes the shell without starting the game. The
        # invalid combination currently exits 2; never silently retry one mode.
        probe_env = dict(env, QT_QPA_PLATFORM='offscreen')
        command = [str(binary), '--repo', str(ROOT), '--live-menus', '--native-commands', '--smoke-test']
        with (out / 'mode-probe.log').open('w') as log:
            probe = subprocess.run(command, cwd=ROOT, env=probe_env, stdout=log,
                                   stderr=subprocess.STDOUT, timeout=20)
        report['mode_probe'] = {'command': command, 'exit_code': probe.returncode}
        if probe.returncode:
            report['status'] = 'blocked'
            raise RuntimeError('Public shell rejects native menus + native commands (exit 2).' if probe.returncode == 2
                               else 'Combined-mode smoke probe failed; inspect mode-probe.log.')
        if args.check_only:
            report.update(status='admitted-not-exercised', stage='admission-complete')
            exit_code = 0
        else:
            report['stage'] = 'runtime-prerequisites'
            for tool in ['wine', 'wineserver'] + (['tesseract'] if args.require_casting_combat else []) + ([] if args.display else ['Xvfb']):
                if not shutil.which(tool):
                    report['status'] = 'blocked'
                    raise RuntimeError('Missing runtime prerequisite: ' + tool)
            if args.display:
                env['DISPLAY'] = args.display
            else:
                with (out / 'display').open('w+') as display, (out / 'xvfb.log').open('w') as log:
                    server = subprocess.Popen(['Xvfb', '-displayfd', str(display.fileno()), '-screen', '0',
                                               '1280x1024x24', '-nolisten', 'tcp'], pass_fds=(display.fileno(),),
                                              stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                    deadline = time.monotonic() + 10
                    while time.monotonic() < deadline:
                        display.seek(0)
                        number = display.read().strip()
                        if number:
                            env['DISPLAY'] = ':' + number
                            env['LIBGL_ALWAYS_SOFTWARE'] = '1'
                            break
                        if server.poll() is not None:
                            raise RuntimeError('Private X display exited')
                        time.sleep(.1)
                    else:
                        raise RuntimeError('Private X display did not start')
            report['stage'] = 'original-manifest-before'
            with (out / 'original-before.log').open('w') as log:
                subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], cwd=ROOT,
                               stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
            verified = True
            report['stage'] = 'campaign-flow'
            env['MNM_CAMPAIGN_SMOKE_TEST'] = '1'
            env['MNM_CAMPAIGN_DIFFICULTY'] = str(args.difficulty)
            env['MNM_CAMPAIGN_CASTING_COMBAT'] = '1' if args.require_casting_combat else '0'
            env['MNM_CAMPAIGN_STRESS_SECONDS'] = str(args.stress_seconds)
            env['MNM_CAMPAIGN_MENU_CYCLES'] = str(args.menu_cycles)
            env['MNM_CAMPAIGN_TIMEOUT_MS'] = str((args.timeout-5)*1000)
            command = [str(ROOT / 'tools/run-qt-shell.sh'), '--live-menus', '--native-commands',
                       '--live-menu-test', str(out / 'flow.json')]
            report['launch_command'] = command
            with (out / 'shell.log').open('w') as log:
                shell = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log,
                                         stderr=subprocess.STDOUT, start_new_session=True)
                report['launch_requested'] = True
                deadline = time.monotonic() + args.timeout
                while time.monotonic() < deadline:
                    lines = (out / 'shell.log').read_text(errors='replace').splitlines()
                    roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
                    if monitor is None and len(roots)==1 and roots[0].is_relative_to(ROOT/'working/experiments/menu-observer') and (roots[0]/'render-frame.bin').exists():
                        monitor=subprocess.Popen(['python3',str(ROOT/'tools/profile-render-stream.py'),str(roots[0]/'render-frame.bin'),'--output',str(out/'publication-rate.jsonl'),'--seconds',str(args.timeout)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,start_new_session=True)
                    if len(roots) == 1 and roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer') and world_ready(roots[0]):
                        (out / 'flow.json.world-ready').touch(exist_ok=True)
                    if (out / 'flow.json').exists():
                        break
                    if shell.poll() is not None:
                        raise RuntimeError('Public shell exited before writing campaign evidence')
                    time.sleep(.2)
                else:
                    raise RuntimeError('Public campaign flow exceeded its deadline')
            flow = json.loads((out / 'flow.json').read_text())
            report['flow'] = flow
            validate_flow(flow, args.menu_cycles)
            if args.stress_seconds:
                for cycle in range(args.menu_cycles+1):
                    inputs=json.loads((out/f'input-{cycle}.json').read_text())
                    if inputs.get('success') is not True or inputs['seconds']<args.stress_seconds or len(inputs['actions'])<20:raise RuntimeError('Incomplete gameplay input sequence')
                report['gameplay_input_exercised']=True
            for step in flow['steps']:
                if step['step'] != 'campaign-handoff':
                    if step['step'] in ('gameplay','gameplay-resumed') and (step.get('native_image_saved') is not True or step.get('original_image_saved') is not True):raise RuntimeError('Missing independent native/original gameplay image')
                    shot = Path(step['screenshot']).resolve()
                    if shot.parent != out or not shot.is_file() or shot.stat().st_size == 0:
                        raise RuntimeError('Missing or redirected screenshot: ' + str(shot))
            lines = (out / 'shell.log').read_text(errors='replace').splitlines()
            roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
            if len(roots) != 1 or not roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer'):
                raise RuntimeError('Missing unique menu experiment identity')
            report['experiment'] = str(roots[0])
            report['engine'] = validate_events(roots[0], args.menu_cycles, args.difficulty)
            if args.require_casting_combat:
                report['gameplay']=validate_casting_combat(roots[0],json.loads((out/'input-0.json').read_text()))
                report.update(casting_verified=True,combat_verified=True)
            validate_flow(flow,args.menu_cycles,args.stress_seconds,args.min_fps)
            if any(sha(ROOT / p) != h for p,h in report['sources'].items()):raise RuntimeError('Source changed during execution')
            report.update(success=True, status='passed', stage='campaign-complete', flow_completed=True, game_entry_verified=True)
            exit_code = 0
    except (KeyboardInterrupt, OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error) or 'Campaign test interrupted'
        if report['status'] == 'running':
            report['status'] = 'failed'
    finally:
        try:
            stop_group(monitor)
            profile=out/"publication-rate.jsonl"
            if profile.exists():report["original_owned_publication_profile"]=[json.loads(line) for line in profile.read_text().splitlines()]
            report["sources_unchanged"]=all(sha(ROOT/path)==expected for path,expected in report["sources"].items())
            stop_group(shell)
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, status='failed', cleanup_error=str(error))
            exit_code = 1
        if (out / 'wineprefix').exists():
            try:
                subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, timeout=10, check=True)
            except (OSError, subprocess.SubprocessError) as error:
                report.update(success=False, status='failed', cleanup_error=str(error))
                exit_code = 1
        try:
            stop_group(server)
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, status='failed', display_cleanup_error=str(error))
            exit_code = 1
        if verified:
            try:
                with (out / 'original-after.log').open('w') as log:
                    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], cwd=ROOT,
                                   stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
                report['original_manifest_verified_after'] = True
            except (OSError, subprocess.SubprocessError) as error:
                report.update(success=False, status='failed', original_manifest_error=str(error))
                exit_code = 1
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(report['status'] + ': ' + report.get('error', report['stage']), flush=True)
    print('Report: ' + str(out / 'report.json'), flush=True)
    return exit_code


if __name__ == '__main__':
    raise SystemExit(main())
