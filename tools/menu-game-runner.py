#!/usr/bin/env python3
"""Hash-checked disposable game runner used only through tools/run-game.sh."""
import hashlib
import json
import os
from pathlib import Path
import sys
root=Path(os.environ['MNM_MENU_EXPERIMENT']).resolve();game=root/'game'
metadata=json.loads((root/'manifest.json').read_text())
if metadata['origin'] not in ('menu_observation_only','menu_action_bridge'):raise ValueError('Unsupported experiment')
for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
events=Path(metadata['events']).resolve()
if events!=root/'events.bin' or events.exists():raise ValueError('Refusing stale or redirected observation output')
if os.environ.get('MNM_MENU_CHANNEL'):
    expected='Z:'+str(root/'channel.bin').replace('/','\\')
    if metadata['origin']!='menu_action_bridge' or os.environ['MNM_MENU_CHANNEL']!=expected:raise ValueError('Refusing unapproved command channel')
os.environ['MNM_MENU_OBSERVE']='Z:'+str(events).replace('/','\\')
os.chdir(game)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
