#!/usr/bin/env python3
"""Run native Qt presentation with legacy graphics confined to a private X server.

Opt-in integration experiment. This does not grant complete renderer acceptance
or close the original-reference ownership gates.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import select
import signal
import struct
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def authority_bytes(cookie):
    fields = (b'', b'', b'MIT-MAGIC-COOKIE-1', cookie)
    return struct.pack('>H', 65535) + b''.join(struct.pack('>H', len(v))+v for v in fields)


def start_display(directory, allow_glx=False):
    cookie = os.urandom(16)
    authority = directory/'Xauthority'
    with authority.open('xb') as out:
        os.chmod(authority, 0o600)
        out.write(authority_bytes(cookie))
    read, write = os.pipe()
    log = (directory/'xvfb.log').open('xb')
    try:
        args=['Xvfb', '-displayfd', str(write), '-screen', '0', '1280x1024x24',
            '-nolisten', 'tcp', '-noreset', '-auth', str(authority), '-extension', 'MIT-SHM',
            '-extension', 'DRI3']
        if not allow_glx:args+=['-extension','GLX']
        server = subprocess.Popen(args, pass_fds=(write,),
            stdin=subprocess.DEVNULL, stdout=log, stderr=log, start_new_session=True)
    finally:
        os.close(write)
        log.close()
    try:
        if not select.select([read], [], [], 10)[0]:
            raise RuntimeError('Private X server did not publish a display')
        number = os.read(read, 32).decode().strip()
        if not number.isdigit() or server.poll() is not None:
            raise RuntimeError('Private X server failed initialization')
        socket = Path('/tmp/.X11-unix')/('X'+number)
        info = socket.stat()
        receipt = dict(schema=1, policy='native-only-private-x11-v1', owner_pid=os.getpid(),
            server_pid=server.pid, display=':'+number, socket_identity=[info.st_dev, info.st_ino],
            authority=str(authority), authority_sha256=hashlib.sha256(authority.read_bytes()).hexdigest(),
            disabled_extensions=['MIT-SHM','DRI3']+([] if allow_glx else ['GLX']))
        (directory/'owner.json').write_text(json.dumps(receipt, indent=2)+'\n')
        return server, receipt, cookie
    except BaseException:
        if server.poll() is None:server.terminate()
        server.wait(timeout=10)
        raise
    finally:
        os.close(read)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-only', action='store_true', help='Require the versioned native-only graphics contract; conflicts and fallback terminate')
    parser.add_argument('--device-recovery', action='store_true', help='Isolate native graphics with replay/retry and raster presentation; adds IPC and GPU readbacks')
    parser.add_argument('--prefix', type=Path, required=True, help='Stopped, admitted private Wine prefix under working/')
    parser.add_argument('--output', type=Path, help='New ownership evidence directory under working/')
    parser.add_argument('arguments', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    def interrupted(signum, _frame):
        raise SystemExit(128+signum)
    signal.signal(signal.SIGTERM, interrupted)
    from native_only_contract import configure, selected, outcome
    extra = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
    strict = args.native_only or selected(os.environ)
    try:
        policy_env = configure(os.environ,extra) if strict else dict(os.environ)
        if args.device_recovery:policy_env['MNM_NATIVE_DEVICE_RECOVERY']='1'
    except ValueError as error:
        parser.error(str(error))
    prefix = args.prefix.resolve()
    if not prefix.is_relative_to(ROOT/'working') or not prefix.is_dir():
        parser.error('Require an existing owned prefix under working/')
    from directplay_dx6_launch import prefix_identity, check_prefix
    marker = prefix/'.mnm-directplay-dx6.json'
    if marker.is_symlink() or json.loads(marker.read_text()) != prefix_identity(prefix):
        parser.error('Prefix lacks matching native-session ownership')
    check_prefix(prefix)
    subprocess.run(['wineserver', '-w'], env=dict(os.environ, WINEPREFIX=str(prefix)), check=True, timeout=5)
    parent = ROOT/'working/tests/native-output-owner'
    parent.mkdir(parents=True, exist_ok=True)
    if args.output:
        directory=args.output.resolve()
        if args.output.is_symlink() or not directory.is_relative_to(ROOT/'working') or directory.exists():
            parser.error('Require a new ownership output directory under working/')
        directory.mkdir(parents=True)
    else:
        directory = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (directory/'.retain').write_text('Native ownership launch evidence; preserve failures and receipts.\n')
    server, receipt, cookie = start_display(directory)
    child = None
    result = dict(success=False, complete_replacement=False, owner=receipt)
    try:
        env = dict(policy_env, MNM_NATIVE_OUTPUT_OWNER=str(directory/'owner.json'),
            MNM_LEGACY_DISPLAY=receipt['display'], MNM_LEGACY_XCOOKIE=cookie.hex(),
            MNM_OWNED_WINE_PREFIX=str(prefix), MNM_NATIVE_CPU_GUARD='3',
            MNM_PUBLIC_RESIDENT_WORLD='1', MNM_OTHER_PIXEL_PRODUCERS='1', MNM_NATIVE_PIXEL_NO_FORWARD='1')
        for flag in ('MNM_PANEL_TAKEOVER', 'MNM_PIXEL_TAKEOVER', 'MNM_JPEG_TAKEOVER',
                     'MNM_MINIMAP_TAKEOVER', 'MNM_COLOUR_RECT_TAKEOVER', 'MNM_RASTER_TAKEOVER'):
            env[flag] = '1'
        child = subprocess.Popen([str(ROOT/'tools/run-qt-shell.sh'), '--repo', str(ROOT),
            '--renderer', 'opengl', '--live-menus', '--native-commands', '--resident-world',
            *(['--native-only'] if strict else []), *extra],
            env=env, close_fds=True, start_new_session=True)
        while child.poll() is None:
            if server.poll() is not None:
                raise RuntimeError('Private display owner exited during native session')
            time.sleep(.1)
        result.update(success=child.returncode == 0, shell_exit=child.returncode, native_only_requested=strict)
        if strict and result['success']:
            sessions=sorted((directory/'sessions').glob('*.json'))
            if not sessions:
                raise RuntimeError('Native-only launch has no admitted session')
            result['native_only_sessions']=[outcome(json.loads(path.read_text())['session']) for path in sessions]
            if sum(s['state']=='completed' for s in result['native_only_sessions'])!=1:
                raise RuntimeError('Native-only launch requires one normally completed final generation')
        return child.returncode
    except Exception as error:
        result.update(success=False,error=str(error))
        return 1
    finally:
        if child is not None:
            try:
                os.killpg(child.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            if child.poll() is None:
                try:child.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid, signal.SIGKILL);child.wait(timeout=10)
        if server.poll() is None:
            os.killpg(server.pid, signal.SIGTERM)
            try:server.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(server.pid, signal.SIGKILL);server.wait(timeout=10)
        (directory/'launch-result.json').write_text(json.dumps(result, indent=2)+'\n')


if __name__ == '__main__':
    import sys
    if '--help' in sys.argv or '-h' in sys.argv:
        raise SystemExit(main())
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        raise SystemExit(main())
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
