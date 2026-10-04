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
    parser.add_argument('--seconds',type=int,help='Bounded smoke duration (1..120 seconds)')
    parser.add_argument('--prefix',type=Path,default=REPO/'working/tests/menu-live-wine')
    args=parser.parse_args()
    if args.seconds is not None and not 1<=args.seconds<=120:parser.error('--seconds must be 1..120')
    root=args.experiment.resolve();metadata=json.loads((root/'manifest.json').read_text())
    if metadata['origin']!='menu_observation_only' or metadata['events']!=str(root/'events.bin'):raise ValueError('Unsupported experiment')
    if (root/'events.bin').exists():raise ValueError('Stage a fresh experiment; observation evidence already exists')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((root/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
    env=dict(os.environ,MNM_MENU_EXPERIMENT=str(root),WINEDEBUG=os.environ.get('WINEDEBUG','fixme-all'))
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
