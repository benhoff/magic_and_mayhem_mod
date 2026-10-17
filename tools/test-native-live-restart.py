#!/usr/bin/env python3
"""Actual-game resident producer failure, terminal cleanup and fresh Hard restart.

Run under working-retention.py. Requires a prospective coverage declaration and
an owned isolated checkout/source installation. Retains failed attempts too.
"""
import argparse
import hashlib
import json
import os
import resource
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time

from test_native_restart_support import validate_flow

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--source-game', type=Path, required=True)
    parser.add_argument('--prefix-template', type=Path, help='Stopped owned prefix; copied into this run')
    parser.add_argument('--independent-world', action='store_true', help='Exercise the dedicated native render worker owner')
    parser.add_argument('--debugger', action='store_true', help='Retain a batch GDB backtrace for a failing diagnostic attempt')
    args = parser.parse_args()
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    declaration = json.loads(args.claims.read_text())
    source = args.source_game.resolve()
    if not source.is_relative_to(ROOT/'working') or not source.is_dir():
        raise ValueError('Require isolated source installation under working/')
    parent = ROOT/'working/tests/native-live-restart'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (out/'.retain').write_text('Actual-game failure/restart evidence; retain all attempts.\n')
    print(out, flush=True)
    report = dict(success=False, claims=declaration['claims'], sources=dict(declaration['sources']), scope=__doc__)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE', 'WINEDLLOVERRIDES'))}
    env.pop('WAYLAND_DISPLAY', None)
    env.update(QT_QPA_PLATFORM='xcb', XDG_CONFIG_HOME=str(out/'config'), WINEDEBUG='-all',
               WINEPREFIX=str(out/'wineprefix'), LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4',
               MNM_RESIDENT_RESTART_TEST='1', MNM_MENU_GAME_SOURCE=str(source),
               MNM_VERIFY_ORIGINALS='0', MNM_CACHE_ADAPTERS='1', MNM_GLYPH_BACKEND='takeover',
               MNM_CAMPAIGN_PRECISE_CLOCK='1', MNM_RENDER_COMMAND_WIRE_VERSION='3')
    server = shell = None
    verified = False

    def run(name, command, timeout=180):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, check=True, timeout=timeout)

    try:
        run('build', [ROOT/'tools/run-qt-shell.sh', '--help'], 600)
        # Preserve the actual compiler closure plus launcher/adapter sources.
        import importlib.util
        spec = importlib.util.spec_from_file_location('campaign', ROOT/'tools/test-native-campaign.py')
        campaign = importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
        paths = campaign.linked_compiler_sources(ROOT/'working/build/qt-shell', ROOT)
        for folder in ('runtime', 'protocols', 'tools'):
            paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/folder).rglob('*')
                         if p.is_file() and p.suffix in ('.c', '.cpp', '.h', '.hpp', '.py', '.sh', '.S', '.json'))
        for p in sorted(paths):
            fingerprint = sha(ROOT/p)
            if p in report['sources'] and report['sources'][p] != fingerprint:
                raise ValueError('Prospective source changed before execution: '+p)
            report['sources'][p] = fingerprint
        if any(sha(ROOT/p) != h for p, h in report['sources'].items()):
            raise ValueError('Sources changed before execution')
        report['shell_sha256'] = sha(ROOT/'working/build/qt-shell/mnm-qt-shell')
        (out/'acquisition.json').write_text(json.dumps(report, indent=2)+'\n')
        if args.prefix_template:
            from directplay_dx6_launch import prefix_identity, check_prefix
            prefix = args.prefix_template.resolve()
            if not prefix.is_dir() or 'working/tests' not in prefix.as_posix():
                raise ValueError('Require an owned stopped test prefix')
            marker = prefix/'.mnm-directplay-dx6.json'
            if marker.is_symlink() or json.loads(marker.read_text()) != prefix_identity(prefix):
                raise ValueError('Template DirectPlay ownership differs')
            check_prefix(prefix)
            subprocess.run(['wineserver', '-w'], env={**env, 'WINEPREFIX': str(prefix)}, check=True, timeout=15)
            shutil.copytree(prefix, out/'wineprefix', symlinks=True)
            check_prefix(out/'wineprefix')
            # Only the verified private copy receives a new owner identity.
            (out/'wineprefix/.mnm-directplay-dx6.json').write_text(json.dumps(prefix_identity(out/'wineprefix'), indent=2)+'\n')
            report['prefix_template'] = dict(path=str(prefix), marker_sha256=sha(marker), copied_dll_hashes_verified=True,
                                             cloned_marker_sha256=sha(out/'wineprefix/.mnm-directplay-dx6.json'))
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify']);verified = True
        with (out/'display').open('w+') as display, (out/'xvfb.log').open('x') as log:
            server = subprocess.Popen(['Xvfb', '-displayfd', str(display.fileno()), '-screen', '0', '1280x1024x24', '-nolisten', 'tcp'],
                                      pass_fds=(display.fileno(),), stdout=log, stderr=log, start_new_session=True)
            deadline = time.monotonic()+15
            while time.monotonic() < deadline:
                display.seek(0);number = display.read().strip()
                if number:env['DISPLAY'] = ':'+number;break
                if server.poll() is not None:raise ValueError('Private display exited')
                time.sleep(.1)
            else:raise ValueError('Private display deadline')
        command = [ROOT/'working/build/qt-shell/mnm-qt-shell', '--repo', ROOT, '--live-menus', '--native-commands',
                   '--resident-world', '--skip-movies', '--legacy-audio', '--legacy-media', '--no-native-music',
                   '--live-menu-test', out/'flow.json']
        if args.independent_world:command.append('--independent-world')
        report['world_owner'] = 'dedicated-worker' if args.independent_world else 'gui-resident'
        report['command'] = [str(x) for x in command]
        if args.debugger:command = ['gdb', '--batch', '--return-child-result', '-ex', 'run', '-ex', 'thread apply all bt', '--args', *command]
        report['executed_command'] = [str(x) for x in command]
        with (out/'shell.log').open('x') as log:
            shell = subprocess.Popen([str(x) for x in command], cwd=ROOT, env=env, stdout=log, stderr=log, start_new_session=True)
            shell.wait(timeout=600)
        report['shell_returncode'] = shell.returncode
        flow = json.loads((out/'flow.json').read_text())
        report['flow'] = flow
        report['validation'] = validate_flow(flow)
        if shell.returncode:raise ValueError('Shell failed: '+str(shell.returncode))
        worlds = [e for e in flow['events'] if e.get('step') == 'live-world']
        report['sessions'] = []
        for index, world in enumerate(worlds):
            root = Path(world['root']).resolve()
            if not root.is_relative_to(ROOT/'working/experiments/menu-observer'):
                raise ValueError('Unowned observed installation')
            (root/'.retain').write_text('Actual-game restart session; retain failure and fresh generation.\n')
            resident = json.loads((root/'resident-world-report.json').read_text())
            manifest = json.loads((root/'manifest.json').read_text())
            if not manifest.get('native_draw') or not manifest.get('resident_world'):
                raise ValueError('Original session did not select resident owned drawing')
            if min(int(resident[k]) for k in ('completed', 'imports', 'verified_returns')) < 3:
                raise ValueError('Insufficient actual World return/import evidence')
            if index and (resident['error'] or not resident['normal_quit_retired'] or resident['pending'] or
                          len({resident[k] for k in ('completed', 'imports', 'verified_returns')}) != 1):
                raise ValueError('Fresh resident session did not retire balanced normal Quit')
            if index:
                process = json.loads((root/'original-process-result.json').read_text())
                if process['original_exit_code'] or process['desktop_exit_code'] or not process['cleanup_after_original_exit']:
                    raise ValueError('Fresh original process did not exit normally')
            else:
                words = struct.unpack('<16I', (root/'render-commands.bin').read_bytes()[:64])
                if words[4] != world['identity'] & 0xffffffff or words[6:9] != (3, 5, 1):
                    raise ValueError('Failed command channel was not cancelled/retired')
            artifacts = {p.relative_to(root).as_posix(): sha(p) for p in root.rglob('*')
                         if p.is_file() and 'game' not in p.relative_to(root).parts and p.name != '.retain'}
            report['sessions'].append(dict(root=str(root), resident=resident, manifest_sha256=sha(root/'manifest.json'), artifacts=artifacts))
        if report['sessions'][0]['resident']['nonce'] == report['sessions'][1]['resident']['nonce']:
            raise ValueError('Fresh World reused the failed nonce')
        quit_input = json.loads((out/'quit-input.json').read_text())
        if quit_input.get('success') is not True or quit_input.get('answer') != 'yes':
            raise ValueError('Original normal Quit answer was not observed')
        report['quit_input'] = quit_input
        report['success'] = True
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        # Bounded cleanup targets only this run's launcher group and Wine prefix.
        import signal
        if shell and shell.poll() is None:
            os.killpg(shell.pid, signal.SIGKILL);shell.wait(timeout=10)
        try:
            with (out/'wine-stop.log').open('x') as log:
                stopped = subprocess.run(['wineserver', '-k'], env=env, stdout=log, stderr=log, timeout=15)
            if stopped.returncode not in (0, 1):raise ValueError('Private Wine stop failed')
            run('wine-wait', ['wineserver', '-w'], 15)
            report['private_wine_stopped'] = True
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            report['cleanup_error'] = str(error);report['success'] = False
        if server and server.poll() is None:
            server.terminate();server.wait(timeout=10)
        if verified:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_verified_after'] = True
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in report['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        report['artifacts'] = {p.relative_to(out).as_posix(): sha(p) for p in out.iterdir() if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
