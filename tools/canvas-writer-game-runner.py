#!/usr/bin/env python3
"""Hash-checked private menu/fade experiment runner for run-game.sh."""
import hashlib
import json
import os
from pathlib import Path
import sys
root=Path(os.environ['MNM_CANVAS_WRITER_EXPERIMENT']).resolve();metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
if metadata['origin']!='fade_writer_takeover':raise ValueError('Unapproved canvas writer experiment')
for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','menu_dll_sha256'),('MnmCanvasWriters.dll','writer_dll_sha256')]:
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError('Staged binary changed: '+name)
for name in ('events.bin','fades.bin'):
    if (root/name).exists():raise ValueError('Stale diagnostics')
if not (root/'channel.bin').is_file():raise ValueError('Absent menu channel')
for name in list(os.environ):
    if name.startswith('MNM_'):os.environ.pop(name)
def wire(name):return 'Z:'+str(root/name).replace('/','\\')
os.environ.update(MNM_MENU_OBSERVE=wire('events.bin'),MNM_MENU_CHANNEL=wire('channel.bin'),MNM_FADE_TAKEOVER='1',MNM_FADE_CAPTURE=wire('fades.bin'))
os.chdir(game);os.execvp('wine',['wine','explorer','/desktop=CanvasWriters,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
