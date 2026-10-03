#!/usr/bin/env python3
"""Launch a staged logging-only game copy after checking both binary hashes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parent.parent

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment',type=Path);parser.add_argument('--samples',type=int,default=1)
    args=parser.parse_args()
    if not 1<=args.samples<=100:parser.error('--samples must be 1..100')
    root=args.experiment.resolve();metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
    for file,key in [('Chaos.exe','staged_sha256'),('MnmShadow.dll','dll_sha256')]:
        if hashlib.sha256((game/file).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'Staged {file} hash mismatch')
    if list((game/'shadow').glob('*.bin')):raise ValueError('Capture directory already contains evidence; stage a fresh experiment')
    env=os.environ.copy();env['MNM_SHADOW_CALLS']=str(args.samples);env['MNM_SHADOW_DIR']='shadow'
    env['WINEPREFIX']=env.get('WINEPREFIX',str(REPO/'working/wineprefix-x86_64'))
    env['MNM_SHADOW_EXPERIMENT']=str(root)
    env.setdefault('WINEDEBUG','fixme-all')
    result=subprocess.run([str(REPO/'tools/run-game.sh'),'launch','--no-gamescope',
                           '--runner',str(REPO/'tools/shadow-game-runner.py')],env=env,check=False)
    for file,key in [('Chaos.exe','staged_sha256'),('MnmShadow.dll','dll_sha256')]:
        if hashlib.sha256((game/file).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'Staged {file} changed')
    return result.returncode
if __name__=='__main__':raise SystemExit(main())
