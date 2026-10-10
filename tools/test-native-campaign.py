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

ROOT = Path(__file__).resolve().parents[1]
STEPS = ['main', 'region', 'campaign-handoff', 'gameplay', 'campaign-menu', 'gameplay-resumed']
SOURCES = ['tools/test-native-campaign.py', 'tools/run-qt-shell.sh',
           'apps/qt-shell/campaign_smoke_test.cpp', 'apps/qt-shell/campaign_smoke_test.hpp',
           'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt',
           'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/menu_bridge.cpp',
           'tools/test-live-campaign-mini.py']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_flow(flow):
    if flow.get('success') is not True:
        raise ValueError(flow.get('error') or 'Campaign driver did not complete')
    if flow.get('native_command_fallback') is not False:
        raise ValueError('Native rendering fallback or missing fallback evidence')
    if type(flow.get('native_command_frames')) is not int or flow['native_command_frames'] < 6:
        raise ValueError('Insufficient native command presentations')
    steps = flow.get('steps', [])
    if [s.get('step') for s in steps] != STEPS:
        raise ValueError('Incomplete campaign click-through')
    if any(s.get('screenshot_saved') is not True for s in steps if s['step'] != 'campaign-handoff'):
        raise ValueError('Missing campaign screenshots')


def validate_events(experiment):
    """Independent engine-side checks; window appearance alone cannot pass."""
    data = (experiment / 'events.bin').read_bytes()
    if data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64) or (len(data)-16) % 64:
        raise ValueError('Invalid original callback observation')
    rows = list(struct.iter_unpack('<16I', data[16:]))
    if len(rows) > 256 or [r[0] for r in rows] != list(range(1, len(rows)+1)):
        raise ValueError('Invalid original callback sequence')
    actions = [(r[3], r[9]) for r in rows if r[1] == 3]
    if actions != [(3, 0), (18, 0), (17, 4)]:
        raise ValueError('Unexpected New Game/Enter/Cancel callback trace: ' + str(actions))
    ticks = [r for r in rows if r[1] == 12 and r[3] == 2 and r[6] == 1 and r[11] == 0x6cbb78]
    cancels = [r for r in rows if r[1] == 3 and r[3] == 17 and r[9] == 4]
    resumes = [r for r in rows if r[1] == 20 and r[11] == 0x6cbb78 and r[0] > cancels[0][0]]
    if len(ticks) < 3 or not resumes or len({r[2] for r in rows if r[1] in (3, 12, 20)}) != 1:
        raise ValueError('Missing same-thread original World updates/Cancel resume')
    channel = (experiment / 'channel.bin').read_bytes()
    if channel[:16] != b'MNMMCM12' + struct.pack('<II', 12, 106496) or len(channel) != 106496:
        raise ValueError('Campaign test did not use the ordinary V12 menu protocol')
    if struct.unpack_from('<I', channel, 36*4)[0] != 3:
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
    parser.add_argument('--check-only', action='store_true', help='Build and check combined-mode admission; no game launch')
    parser.add_argument('--display', help='Use this X11/XWayland display instead of a private Xvfb (moves focus and sends Escape)')
    parser.add_argument('--timeout', type=int, default=330, help='Bound live automation, in seconds (30..600)')
    args = parser.parse_args()
    if not 30 <= args.timeout <= 600:
        parser.error('--timeout must be between 30 and 600')
    parent = ROOT / 'working/tests/native-campaign'
    parent.mkdir(parents=True, exist_ok=True)
    import tempfile
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('Campaign test evidence: ' + str(out), flush=True)
    report = {'schema': 1, 'success': False, 'status': 'running', 'stage': 'build',
              'required_mode': ['native-menus', 'native-command-presentation'],
              'launch_requested': False, 'game_entry_verified': False, 'flow_completed': False, 'movement_verified': False,
              'complete_drawing_replacement': False, 'cleanup': 'bounded private-session termination',
              'sources': {p: sha(ROOT / p) for p in SOURCES}}
    env = clean_environment(out)
    shell = server = None
    verified = False
    exit_code = 1
    try:
        with (out / 'build.log').open('w') as log:
            subprocess.run([str(ROOT / 'tools/run-qt-shell.sh'), '--help'], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        binary = ROOT / 'working/build/qt-shell/mnm-qt-shell'
        report['shell_sha256'] = sha(binary)
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
            for tool in ['wine', 'wineserver'] + ([] if args.display else ['Xvfb']):
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
            command = [str(ROOT / 'tools/run-qt-shell.sh'), '--live-menus', '--native-commands',
                       '--live-menu-test', str(out / 'flow.json')]
            report['launch_command'] = command
            with (out / 'shell.log').open('w') as log:
                shell = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log,
                                         stderr=subprocess.STDOUT, start_new_session=True)
                report['launch_requested'] = True
                deadline = time.monotonic() + args.timeout
                while time.monotonic() < deadline:
                    if (out / 'flow.json').exists():
                        break
                    if shell.poll() is not None:
                        raise RuntimeError('Public shell exited before writing campaign evidence')
                    time.sleep(.2)
                else:
                    raise RuntimeError('Public campaign flow exceeded its deadline')
            flow = json.loads((out / 'flow.json').read_text())
            report['flow'] = flow
            validate_flow(flow)
            for step in flow['steps']:
                if step['step'] != 'campaign-handoff':
                    shot = Path(step['screenshot']).resolve()
                    if shot.parent != out or not shot.is_file() or shot.stat().st_size == 0:
                        raise RuntimeError('Missing or redirected screenshot: ' + str(shot))
            lines = (out / 'shell.log').read_text(errors='replace').splitlines()
            roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
            if len(roots) != 1 or not roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer'):
                raise RuntimeError('Missing unique menu experiment identity')
            report['experiment'] = str(roots[0])
            report['engine'] = validate_events(roots[0])
            report.update(success=True, status='passed', stage='campaign-complete', flow_completed=True, game_entry_verified=True)
            exit_code = 0
    except (KeyboardInterrupt, OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error) or 'Campaign test interrupted'
        if report['status'] == 'running':
            report['status'] = 'failed'
    finally:
        try:
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
