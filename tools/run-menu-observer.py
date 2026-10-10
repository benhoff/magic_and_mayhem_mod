#!/usr/bin/env python3
"""Run a fresh menu observer experiment through the normal game launcher."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment',type=Path)
    parser.add_argument('--menu-channel',type=Path,help='Opt in to the versioned menu action bridge')
    parser.add_argument('--native-render',action='store_true',help='Launch the staged native menu/rendering session')
    parser.add_argument('--seconds',type=int,help='Bounded smoke duration (1..120 seconds)')
    parser.add_argument('--prefix',type=Path,default=REPO/'working/tests/menu-live-wine')
    args=parser.parse_args()
    if args.seconds is not None and not 1<=args.seconds<=120:parser.error('--seconds must be 1..120')
    root=args.experiment.resolve();metadata=json.loads((root/'manifest.json').read_text())
    if metadata['origin'] not in ('menu_observation_only','menu_action_bridge') or metadata['events']!=str(root/'events.bin'):raise ValueError('Unsupported experiment')
    if (root/'events.bin').exists():raise ValueError('Stage a fresh experiment; observation evidence already exists')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((root/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
    if args.native_render and not metadata.get('native_render'):raise ValueError('Native rendering was not staged')
    if args.native_render:
        if hashlib.sha256((root/'game/MnmRender.dll').read_bytes()).hexdigest()!=metadata['render_dll_sha256']:raise ValueError('Render DLL hash mismatch')
    if args.menu_channel:
        if metadata['origin']!='menu_action_bridge':raise ValueError('Stage with --actions to enable commands')
        channel=args.menu_channel.resolve()
        if channel!=root/'channel.bin' or channel.stat().st_size not in (128,32768,65536,69632,73728,77824,81920,86016,90112,94208,102400,106496):raise ValueError('Invalid menu channel')
        if channel.stat().st_size==69632 and not metadata.get('experimental_mini',False):raise ValueError('V4 requires explicitly staged experimental Mini Menu hooks')
    env=dict(os.environ,MNM_MENU_EXPERIMENT=str(root),WINEDEBUG=os.environ.get('WINEDEBUG','fixme-all'))
    env.pop('MNM_MENU_CHANNEL',None)
    env['MNM_MENU_NATIVE_RENDER']='1' if args.native_render else '0'
    if args.menu_channel:env['MNM_MENU_CHANNEL']='Z:'+str(channel).replace('/','\\')
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        command=[str(REPO/'tools/run-game.sh'),'smoke' if args.seconds else 'launch','--no-gamescope',
                 '--prefix',str(args.prefix.resolve()),'--runner',str(REPO/'tools/menu-game-runner.py')]
        if args.seconds:command+=['--seconds',str(args.seconds)]
        result=subprocess.run(command,env=env)
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
            if hashlib.sha256((root/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} changed')
        return result.returncode
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':raise SystemExit(main())
