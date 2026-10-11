#!/usr/bin/env python3
"""Hash-checked runner for the isolated original-active drawing/input session."""
import hashlib,json,os,sys
from pathlib import Path
root=Path(os.environ['MNM_DRAW_SESSION_EXPERIMENT']).resolve();metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
for name,digest in metadata['binaries'].items():
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=digest:raise ValueError('Staged binary changed: '+name)
for name in ('MNM_DRAW02_PUBLICATION','MNM_RENDER_COMMAND_CHANNEL','MNM_RENDER_INPUT','MNM_SCENE_DIR','MNM_WORD_SPRITES'):os.environ.pop(name,None)
if metadata['api_observer_enabled']:os.environ['MNM_DRAW_SESSION_DIRECTORY']='Z:'+str(root/'capture').replace('/','\\')
else:os.environ.pop('MNM_DRAW_SESSION_DIRECTORY',None)
os.environ['MNM_DRAW_SESSION_MODE']=metadata['observer_mode']
os.environ['MNM_MENU_OBSERVE']='Z:'+str(root/'events.bin').replace('/','\\')
os.environ['MNM_MENU_CHANNEL']='Z:'+str(root/'channel.bin').replace('/','\\')
os.environ['MNM_MENU_GAMEPLAY_OBSERVE']='Z:'+str(root/'capture/gameplay.bin').replace('/','\\')
os.chdir(game);os.execvp('wine',['wine','explorer','/desktop=DrawingSession,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
