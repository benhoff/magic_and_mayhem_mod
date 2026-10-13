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
    if metadata['producer_samples'] in (32,120):os.environ['MNM_CANVAS_ORACLE_MIB']='3072'
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
if metadata.get('source_pending_copy'):os.environ['MNM_DRAW_SOURCE_PENDING_COPY']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_PENDING_COPY',None)
if metadata.get('source_readonly_lease'):os.environ['MNM_DRAW_SOURCE_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_LEASE',None)
if metadata.get('source_worker_lease'):os.environ['MNM_DRAW_SOURCE_WORKER_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_WORKER_LEASE',None)
if metadata.get('source_held_snapshot'):os.environ['MNM_DRAW_SOURCE_HELD_SNAPSHOT']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_HELD_SNAPSHOT',None)
if metadata.get('hud_input_point'):os.environ.update(MNM_DRAW_INPUT_POINT='1',MNM_DRAW_HUD_INPUT_POINT='1')
if metadata.get('hud_state'):os.environ['MNM_DRAW_HUD_STATE_FILE']='Z:'+str(root/'capture/hud-state.bin').replace('/','\\')
if metadata.get('actor_latches'):os.environ['MNM_DRAW_ACTOR_LATCH_FILE']='Z:'+str(root/'capture/actor-latches.bin').replace('/','\\')
if metadata.get('region_input'):os.environ['MNM_DRAW_REGION_INPUT_FILE']='Z:'+str(root/'capture/region-input.bin').replace('/','\\')
if metadata.get('cursor_mutex'):os.environ['MNM_DRAW_CURSOR_MUTEX_FILE']='Z:'+str(root/'capture/cursor-mutex.bin').replace('/','\\')
if metadata.get('semantic_state'):os.environ['MNM_DRAW_SEMANTIC_STATE_FILE']='Z:'+str(root/'capture/semantic-state.bin').replace('/','\\')
else:os.environ.pop('MNM_DRAW_SEMANTIC_STATE_FILE',None)
if metadata.get('engine_cadence'):os.environ['MNM_ENGINE_CADENCE_FILE']='Z:'+str(root/'capture/engine-cadence.bin').replace('/','\\')
else:os.environ.pop('MNM_ENGINE_CADENCE_FILE',None)
if metadata.get('canvas_storage_transitions'):os.environ['MNM_DRAW_STORAGE_TRANSITIONS']='1'
else:os.environ.pop('MNM_DRAW_STORAGE_TRANSITIONS',None)
if metadata.get('producer_update_binding'):os.environ['MNM_DRAW_UPDATE_BINDING']='1'
else:os.environ.pop('MNM_DRAW_UPDATE_BINDING',None)
if metadata.get('whole_updates'):os.environ['MNM_DRAW_WHOLE_UPDATES']='1'
else:os.environ.pop('MNM_DRAW_WHOLE_UPDATES',None)
if metadata.get('wine_retained_map_source'):os.environ['MNM_DRAW_WINE_RETAINED_MAP']='1'
else:os.environ.pop('MNM_DRAW_WINE_RETAINED_MAP',None)
if metadata.get('wine_client_memory'):os.environ['MNM_DRAW_WINE_CLIENT_MEMORY']='1'
else:os.environ.pop('MNM_DRAW_WINE_CLIENT_MEMORY',None)
if metadata.get('wine_map_storage_point'):os.environ['MNM_DRAW_WINE_MAP_STORAGE']='1'
else:os.environ.pop('MNM_DRAW_WINE_MAP_STORAGE',None)
if metadata.get('wine_resource_copy'):os.environ['MNM_DRAW_WINE_RESOURCE_COPY']='1'
else:os.environ.pop('MNM_DRAW_WINE_RESOURCE_COPY',None)
os.environ.pop('MNM_DRAW_WINE_SOURCE_MAP_ATTEMPTS',None)
os.environ.pop('MNM_DRAW_WINE_SOURCE_MAP_BYTES',None)
if metadata.get('wine_source_version'):os.environ['MNM_DRAW_WINE_SOURCE_VERSION']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_VERSION',None)
if metadata.get('wine_source_map'):os.environ['MNM_DRAW_WINE_SOURCE_MAP']='2' if metadata.get('wine_source_map_bounded') else '1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_MAP',None)
if metadata.get('wine_source_descriptor'):os.environ['MNM_DRAW_WINE_SOURCE_DESCRIPTOR']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_DESCRIPTOR',None)
if metadata.get('wine_resource_map'):os.environ['MNM_DRAW_WINE_RESOURCE_MAP']='1'
else:os.environ.pop('MNM_DRAW_WINE_RESOURCE_MAP',None)
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
