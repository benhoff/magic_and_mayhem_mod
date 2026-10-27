#!/usr/bin/env python3
"""Launch native-only graphics with a dedicated, reusable Wine simulation prefix."""
import argparse
import fcntl
import json
import os
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
    parser.add_argument('--prefix', type=Path, default=Path('working/native-player/wineprefix'),
                        help='Dedicated owned prefix, initialized once and reused (default: %(default)s)')
    parser.add_argument('--prefix-template', type=Path,
                        help='Optional stopped, admitted prefix to clone on first launch')
    parser.add_argument('arguments', nargs=argparse.REMAINDER,
                        help='Native presentation options after --, e.g. -- --skip-movies --no-native-music')
    args = parser.parse_args(argv)
    extra = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
    try:
        environment = configure(os.environ, extra)
        environment.setdefault('MNM_TERRAIN_LIGHTING', 'native')
        lighting, _ = lighting_selection(environment)
        if environment.get('MNM_MENU_DIRECTPLAY_DX6', '1') != '1':
            raise ValueError('Native player launch requires MNM_MENU_DIRECTPLAY_DX6=1')
        environment['MNM_MENU_DIRECTPLAY_DX6'] = '1'
        prefix = prefix_path(root, args.prefix)
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
        if not (root / 'working/game-nocd/Chaos.exe').is_file():
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
                  'Wine runs the original simulation. Prefix: ' + str(prefix), flush=True)
            return subprocess.run([sys.executable, '-B', str(root / 'tools/run-native-owned.py'),
                                   '--native-only', '--prefix', str(prefix), '--', *extra],
                                  env=environment).returncode
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print('Native launch refused: ' + str(error), file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == '__main__':
    raise SystemExit(main())
