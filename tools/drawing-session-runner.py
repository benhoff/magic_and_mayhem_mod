#!/usr/bin/env python3
"""Hash-checked runner for the isolated original-active drawing/input session."""
import hashlib,json,os,sys
from pathlib import Path
root=Path(os.environ['MNM_DRAW_SESSION_EXPERIMENT']).resolve();metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
for name,digest in metadata['binaries'].items():
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=digest:raise ValueError('Staged binary changed: '+name)
for name in ('MNM_DRAW02_PUBLICATION','MNM_RENDER_COMMAND_CHANNEL','MNM_RENDER_INPUT','MNM_SCENE_DIR','MNM_WORD_SPRITES'):os.environ.pop(name,None)
if metadata.get('producer_samples'):
    os.environ.update(MNM_DRAW02_PRODUCER_BIND='1',MNM_CANVAS_PRODUCERS='1',MNM_MINIMAP_OWNED='1',MNM_SCENE_SAMPLES=str(metadata['producer_samples']),MNM_SCENE_DIR='Z:'+str(root/'capture').replace('/','\\'))
    if metadata['producer_samples']==32:os.environ['MNM_CANVAS_ORACLE_MIB']='3072'
else:os.environ.pop('MNM_DRAW02_PRODUCER_BIND',None)
if metadata['api_observer_enabled']:os.environ['MNM_DRAW_SESSION_DIRECTORY']='Z:'+str(root/'capture').replace('/','\\')
else:os.environ.pop('MNM_DRAW_SESSION_DIRECTORY',None)
os.environ['MNM_DRAW_SESSION_MODE']=metadata['observer_mode']
os.environ['MNM_MENU_OBSERVE']='Z:'+str(root/'events.bin').replace('/','\\')
os.environ['MNM_MENU_CHANNEL']='Z:'+str(root/'channel.bin').replace('/','\\')
os.environ['MNM_MENU_GAMEPLAY_OBSERVE']='Z:'+str(root/'capture/gameplay.bin').replace('/','\\')
if observer:=metadata.get('x11_delivery_library'):
    library=Path(observer['path'])
    if hashlib.sha256(library.read_bytes()).hexdigest()!=observer['sha256']:raise ValueError('Unix delivery observer changed')
    if os.environ.get('LD_PRELOAD'):raise ValueError('Unpinned Unix preload')
    os.environ.update(LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(root/'capture'))
    if metadata.get('egl_physical'):os.environ['MNM_DRAW_EGL_PHYSICAL']='1'
    else:os.environ.pop('MNM_DRAW_EGL_PHYSICAL',None)
os.chdir(game);os.execvp('wine',['wine','explorer','/desktop=DrawingSession,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
