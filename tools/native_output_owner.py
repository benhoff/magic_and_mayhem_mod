"""Private legacy display boundary for explicitly owned native sessions.

This contains legacy presentation; it does not certify original-reference source
atomicity or let an ownership counter promote a complete-renderer claim.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import stat

FLAG = 'MNM_NATIVE_OUTPUT_OWNER'
REQUIRED = ('native_render', 'native_draw', 'resident_world',
            'native_pixel_no_forward', 'native_panel_takeover',
            'native_pixel_takeover', 'native_jpeg_takeover',
            'native_minimap_takeover', 'native_colour_rect_takeover',
            'native_raster_takeover')


def load_receipt(path):
    path = Path(path)
    if path.is_symlink() or not path.is_file():
        raise ValueError('Missing private display owner receipt')
    receipt = json.loads(path.read_text())
    if type(receipt.get('owner_pid')) is not int or receipt['owner_pid'] <= 1:
        raise ValueError('Invalid private display owner identity')
    ancestor = os.getppid()
    for _ in range(64):
        if ancestor == receipt.get('owner_pid'):
            break
        if ancestor <= 1:
            raise ValueError('Private display owner is not a live ancestor')
        fields = dict(line.split(':', 1) for line in Path(f'/proc/{ancestor}/status').read_text().splitlines() if ':' in line)
        ancestor = int(fields['PPid'].strip())
    else:
        raise ValueError('Private display owner ancestry exhausted')
    if (receipt.get('schema') != 1
            or receipt.get('policy') != 'native-only-private-x11-v1'):
        raise ValueError('Foreign private display owner')
    return receipt


def admit(metadata, environment, receipt):
    if metadata.get('origin') != 'menu_action_bridge' or any(
            metadata.get(key) is not True for key in REQUIRED):
        raise ValueError('Private output requires resident owned graphics and every no-forward producer')
    if environment.get('MNM_NATIVE_CPU_GUARD') != '3':
        raise ValueError('Private output requires protected owned backing mode3')
    if receipt.get('disabled_extensions') != ['MIT-SHM','DRI3','GLX']:
        raise ValueError('Private server must disable shared-memory and GPU presentation extensions')
    display = receipt.get('display')
    host = re.fullmatch(r'(?:(?:[^:/]+/)?unix/?)?:(\d+)(?:\.\d+)?', environment.get('DISPLAY', ''))
    if (not isinstance(display, str) or not display.startswith(':')
            or not display[1:].isdigit() or not 0 <= int(display[1:]) <= 65535
            or display[1:] != str(int(display[1:]))
            or (host is not None and int(display[1:]) == int(host[1]))):
        raise ValueError('Legacy display must be a distinct local server')
    socket = Path('/tmp/.X11-unix') / ('X' + display[1:])
    info = socket.stat()
    if (not stat.S_ISSOCK(info.st_mode) or info.st_uid != os.getuid()
            or [info.st_dev, info.st_ino] != receipt.get('socket_identity')):
        raise ValueError('Private display socket changed')
    authority = Path(receipt.get('authority', ''))
    if (authority.is_symlink() or not authority.is_file()
            or authority.stat().st_uid != os.getuid()
            or authority.stat().st_mode & 0o077
            or hashlib.sha256(authority.read_bytes()).hexdigest() != receipt.get('authority_sha256')):
        raise ValueError('Private display authority changed')
    return socket, authority


def command(root, prefix, metadata, environment, receipt, *, repository=None):
    """Build the child boundary, with no host display/socket/device descriptors."""
    socket, authority = admit(metadata, environment, receipt)
    root, prefix = Path(root).resolve(), Path(prefix).resolve()
    if ('original' in root.parts or 'original' in prefix.parts
            or root.is_relative_to(prefix) or prefix.is_relative_to(root)):
        raise ValueError('Require separate disposable session and prefix paths')
    if not root.is_dir() or not prefix.is_dir():
        raise ValueError('Create the owned session and prefix before isolation')
    masked=('/tmp','/var/tmp','/dev','/run/user')
    hidden=any(p.is_relative_to(base) for p in (root,prefix) for base in masked)
    source=None
    if hidden:
        if repository is None:raise ValueError('Create the owned session and prefix before isolation')
        source=Path(repository).resolve()
        if (str(source) in ('/','/tmp','/var/tmp','/dev','/dev/shm','/run/user')
                or source.stat().st_uid!=os.getuid()
                or not (source/'.git').exists() or not (source/'AGENTS.md').is_file()
                or not (source/'tools/menu-game-runner.py').is_file()
                or not all(p.is_relative_to((source/'working').resolve()) for p in (root,prefix))):
            raise ValueError('Masked paths require the exact owned repository and its working targets')
    # The validated file may live beneath a masked /tmp (including a working/
    # alias into RAM). Mount that file explicitly; its host pathname need not
    # remain reachable after replacing the child's temporary directories.
    child_authority = '/tmp/mnm-private-xauthority'
    args = ['bwrap', '--die-with-parent', '--new-session', '--unshare-user',
            '--unshare-pid', '--unshare-net', '--unshare-ipc', '--unshare-uts',
            '--cap-drop', 'ALL', '--ro-bind', '/', '/', '--dev', '/dev',
            '--proc', '/proc', '--tmpfs', '/tmp', '--tmpfs', '/var/tmp',
            '--dir', '/tmp/.X11-unix', '--ro-bind', str(socket), str(socket),
            '--ro-bind', str(authority), child_authority]
    # Re-expose only this validated source checkout after masking temporary
    # directories; it stays read-only. Mutable session/prefix mounts follow.
    if source is not None:args += ['--ro-bind',str(source),str(source)]
    args += ['--bind', str(root), str(root), '--bind', str(prefix), str(prefix)]
    runtime = Path('/run/user')
    if runtime.is_dir():
        args += ['--tmpfs', str(runtime)]
    args += ['--setenv', 'DISPLAY', receipt['display'], '--setenv', 'XAUTHORITY', child_authority,
             '--setenv', 'TMPDIR', '/tmp', '--setenv', 'MNM_NATIVE_OWNERSHIP_REQUIRED', '1']
    for key in ('WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR', 'DBUS_SESSION_BUS_ADDRESS',
                'PULSE_SERVER', 'LD_PRELOAD', 'LD_AUDIT', 'LD_LIBRARY_PATH', 'MNM_LEGACY_XCOOKIE'):
        args += ['--unsetenv', key]
    return args + ['--']
