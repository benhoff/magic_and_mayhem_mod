#!/usr/bin/env python3
"""Hash-checked disposable scene observer runner for tools/run-game.sh."""
import hashlib
import json
import os
from pathlib import Path
import sys

root=Path(os.environ['MNM_SCENE_EXPERIMENT']).resolve()
metadata=json.loads((root/'manifest.json').read_text())
if metadata['origin']!='scene_observation_only':
    raise ValueError('Unsupported experiment')
game=root/'game'
for name,key in [('Chaos.exe','staged_sha256'),('MnmScene.dll','scene_dll_sha256')]+([('MnmMenu.dll','menu_dll_sha256')] if metadata['menu_driver'] else []):
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:
        raise ValueError('Staged binary hash mismatch: '+name)
if any((root/'capture').iterdir()):
    raise ValueError('Stage a fresh scene observation directory')
os.environ['MNM_SCENE_DIR']='Z:'+str(root/'capture').replace('/','\\')
if metadata['menu_driver']:
    os.environ['MNM_MENU_OBSERVE']='Z:'+str(root/'events.bin').replace('/','\\')
    os.environ['MNM_MENU_CHANNEL']='Z:'+str(root/'channel.bin').replace('/','\\')
    if (root/'events.bin').exists() or not (root/'channel.bin').exists():
        raise ValueError('Stale or absent menu observation channel')
os.chdir(game)
os.execvp('wine',['wine','explorer','/desktop=SceneObserver,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
