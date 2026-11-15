#!/usr/bin/env python3
"""Launch native-only graphics with a dedicated, reusable Wine simulation prefix."""
import argparse
import fcntl
import json
import os
import platform
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from directplay_dx6_launch import admit_prefix, check_prefix, install_prefix, prefix_identity, stage
from native_only_contract import configure
from native_prefix_template import clone
from terrain_light_session import selection as lighting_selection

ROOT = Path(__file__).resolve().parents[1]


def player_settings(root):
    """Read optional checkout-local defaults without moving player data."""
    path = root / 'working/native-player/launch.json'
    if path.is_symlink() or not path.resolve().is_relative_to(root / 'working'):
        raise ValueError('Player settings must be owned under working/: ' + str(path))
    try:
        with path.open('rb') as source:
            data = source.read(8193)
    except FileNotFoundError:
        return {}
    if len(data) > 8192:
        raise ValueError('Player settings exceed 8 KiB: ' + str(path))
    try:
        settings = json.loads(data)
    except ValueError as error:
        raise ValueError('Invalid player settings: ' + str(path)) from error
    if (not isinstance(settings, dict) or type(settings.get('schema')) is not int
            or settings['schema'] != 1 or set(settings) - {'schema', 'prefix', 'save_store'}):
        raise ValueError('Unsupported player settings: ' + str(path))
    for key in ('prefix', 'save_store'):
        if key in settings and (not isinstance(settings[key], str) or not settings[key].strip()):
            raise ValueError('Invalid player setting: ' + key)
    return settings


def owned_prefix(prefix):
    marker = prefix / '.mnm-directplay-dx6.json'
    if marker.is_symlink() or not marker.is_file() or json.loads(marker.read_text()) != prefix_identity(prefix):
        raise ValueError('Prefix lacks matching native-session ownership: ' + str(prefix))
    check_prefix(prefix)


def prefix_path(root, value):
    path = value if value.is_absolute() else root / value
    resolved = path.resolve()
    working = root / 'working'
    if not resolved.is_relative_to(working) or resolved == working:
        raise ValueError('Use a dedicated prefix below this checkout\'s working/ directory')
    if any(p.is_symlink() for p in (path, *path.parents) if p != root and root in p.parents):
        raise ValueError('Prefix paths must not traverse symlinks')
    return resolved


def prepare_prefix(root, prefix, template, environment):
    if prefix.exists():
        if template is not None:
            raise ValueError('--prefix-template requires a new destination prefix')
        owned_prefix(prefix)
        return
    subprocess.run([str(root / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        if template is not None:
            clone(template, prefix, environment)
        else:
            parent = root / 'working/native-player/bootstrap'
            parent.mkdir(parents=True, exist_ok=True)
            output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
            (output / '.retain').write_text('Native player prefix initialization; retain inputs, backups and failures.\n')
            (output / 'game').mkdir()
            # This physical manifest is required input to the existing DirectPlay installer.
            # Its exact profile and original/replaced DLL hashes remain checked there.
            metadata = {'directplay_dx6': stage(output)}
            admit_prefix(output, metadata, prefix)
            install_prefix(output, metadata, dict(environment, WINEPREFIX=str(prefix)))
        owned_prefix(prefix)
    finally:
        subprocess.run([str(root / 'tools/original-manifest.sh'), 'verify'], check=True)


def main(argv=None, root=ROOT):
    parser = argparse.ArgumentParser(description=__doc__, epilog=(
        'Graphics use our native renderer without original presentation fallback. '
        'Native terrain lighting is the default; set MNM_TERRAIN_LIGHTING=original to override. '
        'Wine still runs the original simulation. Full-game rendering equivalence remains pending. '
        'Use ./run-x86.sh --original-renderer for the original game renderer.'))
    parser.add_argument('--prefix', type=Path,
                        help='Dedicated owned prefix; uses local settings, otherwise working/native-player/wineprefix')
    parser.add_argument('--output-owner', type=Path, help='New explicitly assigned ownership directory under working/')
    parser.add_argument('--prefix-template', type=Path,
                        help='Optional stopped, admitted prefix to clone on first launch')
    parser.add_argument('--world-delivery', choices=('auto', 'synchronous', 'independent'), default='auto',
                        help='World delivery: auto selects independent rendering on Linux ARM64; '
                             'synchronous retains the original launch policy')
    parser.add_argument('arguments', nargs=argparse.REMAINDER,
                        help='Native presentation options after --, e.g. -- --skip-movies --no-native-music')
    args = parser.parse_args(argv)
    extra = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
    try:
        settings = player_settings(root)
        if args.world_delivery == 'synchronous' and '--independent-world' in extra:
            raise ValueError('--world-delivery synchronous conflicts with --independent-world')
        linux_arm = sys.platform == 'linux' and platform.machine() == 'aarch64'
        independent = args.world_delivery == 'independent' or (args.world_delivery == 'auto' and linux_arm)
        if independent and '--independent-world' not in extra:
            extra = [*extra, '--independent-world']
        environment = configure(os.environ, extra)
        # Default only the two selection conveniences. The wider RTS shortcut
        # pack remains explicit (1); 0 preserves the original selection policy.
        environment.setdefault('MNM_RTS_CONTROLS', 'selection')
        from rts_controls_launch import enabled_from_environment
        enabled_from_environment(environment)
        if 'save_store' in settings and 'MNM_MENU_SAVE_STORE' not in environment:
            # Use the same owned-working-directory rules as the player prefix.
            environment['MNM_MENU_SAVE_STORE'] = str(prefix_path(root, Path(settings['save_store'])))
        if linux_arm and '--independent-world' in extra:
            # Replay lists omit native surface writes between World subqueues.
            # Preserve the completed World for presentations between updates.
            environment.setdefault('MNM_WORLD_IMAGE_REUSE', '1')
            # Moving native UI/occlusion must use this World's completed UI,
            # rather than presenting the preceding generation's literal pixels.
            environment.setdefault('MNM_WORLD_FIRST_SEAL_BASELINE', '1')
        environment.setdefault('MNM_TERRAIN_LIGHTING', 'native')
        lighting, _ = lighting_selection(environment)
        if environment.get('MNM_MENU_DIRECTPLAY_DX6', '1') != '1':
            raise ValueError('Native player launch requires MNM_MENU_DIRECTPLAY_DX6=1')
        environment['MNM_MENU_DIRECTPLAY_DX6'] = '1'
        owner_output=args.output_owner.resolve() if args.output_owner is not None else None
        if owner_output is not None and (not owner_output.is_relative_to(root/'working') or args.output_owner.is_symlink() or owner_output.exists()):
            raise ValueError('Require a new explicit ownership directory under working/')
        prefix = prefix_path(root, args.prefix if args.prefix is not None else
                             Path(settings.get('prefix', 'working/native-player/wineprefix')))
        if prefix.exists():
            if args.prefix_template is not None:
                raise ValueError('--prefix-template requires a new destination prefix')
            owned_prefix(prefix)
        required = ['wine', 'wineserver', 'Xvfb', 'cmake']
        if not prefix.exists() and args.prefix_template is None:
            required += ['wineboot', '7z']
        missing = [name for name in required if shutil.which(name) is None]
        if missing:
            raise ValueError('Missing required programs: ' + ', '.join(missing))
        source_game=Path(environment.get('MNM_MENU_GAME_SOURCE',root/'working/game-nocd')).resolve()
        if not source_game.is_relative_to(root/'working'):
            from check_inputs import paths
            if paths(root).get('game')!=source_game:
                raise ValueError('Native game source must be owned or a verified shared input')
        if not (source_game/'Chaos.exe').is_file():
            raise ValueError('Prepare the working installation with tools/prepare-working.sh first')
        prefix.parent.mkdir(parents=True, exist_ok=True)
        lock = prefix.with_name(prefix.name + '.native-player.lock')
        if lock.is_symlink():
            raise ValueError('Refusing a symlinked player prefix lock')
        with lock.open('a') as lease:
            try:
                fcntl.flock(lease, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                raise ValueError('Another native player launch owns this prefix') from None
            # Never kill another user of a prefix or copy a running template.
            if prefix.exists():
                subprocess.run(['wineserver', '-w'], env=dict(environment, WINEPREFIX=str(prefix)),
                               check=True, timeout=5)
            previous = os.environ.get('MNM_MENU_DIRECTPLAY_DX6')
            os.environ['MNM_MENU_DIRECTPLAY_DX6'] = '1'
            try:
                prepare_prefix(root, prefix, args.prefix_template, environment)
            finally:
                if previous is None:
                    os.environ.pop('MNM_MENU_DIRECTPLAY_DX6', None)
                else:
                    os.environ['MNM_MENU_DIRECTPLAY_DX6'] = previous
            print('Native renderer required; original renderer fallback disabled.\n'
                  'Terrain lighting: ' + lighting + '.\n'
                  'World delivery: ' + ('independent' if '--independent-world' in extra else 'synchronous') + '.\n'
                  'Wine runs the original simulation. Prefix: ' + str(prefix), flush=True)
            return subprocess.run([sys.executable, '-B', str(root / 'tools/run-native-owned.py'),
                                   '--native-only', '--prefix', str(prefix),
                                   *(['--output',str(owner_output)] if owner_output is not None else []), '--', *extra],
                                  env=environment).returncode
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print('Native launch refused: ' + str(error), file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == '__main__':
    raise SystemExit(main())
