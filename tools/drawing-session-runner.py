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
if pin:=metadata.get('wine_submission'):
    for name,digest in pin['module_sha256'].items():
        if hashlib.sha256(Path('/usr/lib/wine/i386-windows',name+'.dll').read_bytes()).hexdigest()!=digest:raise ValueError('Installed Wine module changed: '+name)
    os.environ['MNM_DRAW_WINE_SUBMISSION']='1'
else:os.environ.pop('MNM_DRAW_WINE_SUBMISSION',None)
if metadata.get('wine_texture_entry'):os.environ['MNM_DRAW_WINE_TEXTURE_ENTRY']='1'
else:os.environ.pop('MNM_DRAW_WINE_TEXTURE_ENTRY',None)
if metadata.get('wine_blit_worker'):os.environ['MNM_DRAW_WINE_BLIT_WORKER']='1'
else:os.environ.pop('MNM_DRAW_WINE_BLIT_WORKER',None)
if metadata.get('wine_blit_packet'):os.environ['MNM_DRAW_WINE_BLIT_PACKET']='1'
else:os.environ.pop('MNM_DRAW_WINE_BLIT_PACKET',None)
if metadata.get('wine_worker_context'):os.environ['MNM_DRAW_WINE_WORKER_CONTEXT']='1'
else:os.environ.pop('MNM_DRAW_WINE_WORKER_CONTEXT',None)
if metadata.get('source_byte_witness'):os.environ['MNM_DRAW_SOURCE_BYTES']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_BYTES',None)
if metadata.get('source_readonly_lease'):os.environ['MNM_DRAW_SOURCE_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_LEASE',None)
if metadata.get('source_worker_lease'):os.environ['MNM_DRAW_SOURCE_WORKER_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_WORKER_LEASE',None)
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
