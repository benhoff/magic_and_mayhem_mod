#!/usr/bin/env python3
"""Launch opt-in continuous native World shadow presentation beside the original game."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import secrets
import signal
import struct
import sys
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify', action='store_true', help='Compare each presented native World frame with a separately owned original oracle')
    parser.add_argument('--interval', type=int, default=3, help='Original World queues between publication attempts (1..3600)')
    parser.add_argument('--skip-queues', type=int, default=120, help='Original startup queues to skip (0..3600)')
    parser.add_argument('--prefix-template', type=Path, default=ROOT/'working/wineprefix-x86_64')
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('An X11 display is required')
    if not 1 <= args.interval <= 3600 or not 0 <= args.skip_queues <= 3600:parser.error('Queue bounds exceeded')
    build = ROOT/'working/build/world-frame'
    subprocess.run(['cmake', '-S', str(ROOT/'compat/legacy'), '-B', str(build), '-DCMAKE_BUILD_TYPE=Debug'], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-world-live', '-j4'], check=True)
    spec = importlib.util.spec_from_file_location('world_prepare', ROOT/'tools/prepare-scene-observer.py')
    staging = importlib.util.module_from_spec(spec);spec.loader.exec_module(staging)
    game = None;viewer = None;env = None
    subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        root = staging.prepare(world_frames=True, world_live=True)
        env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(MNM_SCENE_EXPERIMENT=str(root), MNM_SCENE_SKIP=str(args.skip_queues), MNM_SCENE_INTERVAL=str(args.interval),
                   WINEPREFIX=str(root/'wineprefix'), WINEDEBUG='-all')
        env.pop('WAYLAND_DISPLAY', None)
        subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()), env['WINEPREFIX']], check=True)
        with (root/'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90, check=True)
        size = 128 + 2*(16 + 32*1024*1024 + 8*1024*1024)
        header = bytearray(128);header[:8] = b'MNMWCH01'
        struct.pack_into('<15I', header, 8, 1, size, secrets.randbelow(0xffffffff)+1, 0, 0, 0, 0, 0, 0,
                         int(args.verify), 0, 2, 32*1024*1024, 8*1024*1024, 0x40209ca7)
        with (root/'world-channel.bin').open('xb') as f:f.write(header);f.truncate(size)
        print('Native World shadow session:', root, flush=True)
        print('Use the original game window for menus and battle input. Close either window to end this isolated session.', flush=True)
        with (root/'native-world.log').open('x') as native_log, (root/'game.log').open('x') as game_log:
            viewer = subprocess.Popen([str(build/'mnm-world-live'), '--root', str(root/'game'), '--channel', str(root/'world-channel.bin'),
                                       '--report', str(root/'native-world.json')], env={**env, 'QT_QPA_PLATFORM':'xcb'}, stdout=native_log, stderr=subprocess.STDOUT)
            game = subprocess.Popen([str(ROOT/'tools/run-game.sh'), 'launch', '--no-gamescope', '--prefix', env['WINEPREFIX'],
                                     '--runner', str(ROOT/'tools/scene-game-runner.py')], env=env, stdout=game_log, stderr=subprocess.STDOUT, start_new_session=True)
            while game.poll() is None and viewer.poll() is None:time.sleep(.1)
            if viewer.poll() not in (None, 0):
                report_path=root/'native-world.json';reason='Native World viewer exited with code '+str(viewer.returncode)
                if report_path.exists():
                    report=json.loads(report_path.read_text());reason=report.get('error') or reason
                    if report.get('producer_reason'):reason+=' (capture reason '+str(report['producer_reason'])+')'
                raise RuntimeError(reason+'; details: '+str(root/'native-world.log'))
    finally:
        if viewer and viewer.poll() is None:
            viewer.terminate()
            try:viewer.wait(timeout=5)
            except subprocess.TimeoutExpired:viewer.kill();viewer.wait(timeout=5)
        if game and game.poll() is None:
            os.killpg(game.pid, signal.SIGTERM)
            try:game.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(game.pid, signal.SIGKILL);game.wait(timeout=5)
        if env:subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    try:main()
    except (RuntimeError,subprocess.CalledProcessError) as error:
        print('Native World session failed: '+str(error),file=sys.stderr);sys.exit(1)
