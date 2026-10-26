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
if metadata.get('aux_save_readback'):os.environ['MNM_DRAW_AUX_SAVE_READBACK']='1'
if metadata.get('wine_worker_producer_points'):os.environ['MNM_DRAW_WINE_WORKER_PRODUCER']='1'
else:os.environ.pop('MNM_DRAW_WINE_WORKER_PRODUCER',None)
if metadata.get('wine_blit_packet'):os.environ['MNM_DRAW_WINE_BLIT_PACKET']='1'
else:os.environ.pop('MNM_DRAW_WINE_BLIT_PACKET',None)
if metadata.get('wine_worker_context'):os.environ['MNM_DRAW_WINE_WORKER_CONTEXT']='1'
else:os.environ.pop('MNM_DRAW_WINE_WORKER_CONTEXT',None)
if metadata.get('source_byte_witness'):os.environ['MNM_DRAW_SOURCE_BYTES']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_BYTES',None)
if metadata.get('source_pending_copy'):os.environ['MNM_DRAW_SOURCE_PENDING_COPY']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_PENDING_COPY',None)
if metadata.get('session_producers'):os.environ['MNM_CANVAS_SESSION_PRODUCERS']='1'
else:os.environ.pop('MNM_CANVAS_SESSION_PRODUCERS',None)
if metadata.get('session_close_diagnostics'):os.environ['MNM_CANVAS_SESSION_CLOSE_DIAGNOSTICS']='1'
else:os.environ.pop('MNM_CANVAS_SESSION_CLOSE_DIAGNOSTICS',None)
if metadata.get('source_write_trace'):os.environ['MNM_DRAW_SOURCE_WRITE_TRACE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_WRITE_TRACE',None)
if metadata.get('source_readonly_lease'):os.environ['MNM_DRAW_SOURCE_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_LEASE',None)
if metadata.get('source_worker_lease'):os.environ['MNM_DRAW_SOURCE_WORKER_LEASE']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_WORKER_LEASE',None)
if metadata.get('source_held_snapshot'):os.environ['MNM_DRAW_SOURCE_HELD_SNAPSHOT']='1'
else:os.environ.pop('MNM_DRAW_SOURCE_HELD_SNAPSHOT',None)
if metadata.get('hud_input_point'):os.environ.update(MNM_DRAW_INPUT_POINT='1',MNM_DRAW_HUD_INPUT_POINT='1')
if metadata.get('hud_state'):os.environ['MNM_DRAW_HUD_STATE_FILE']='Z:'+str(root/'capture/hud-state.bin').replace('/','\\')
if metadata.get('copy_order'):os.environ['MNM_DRAW_COPY_ORDER']='1'
else:os.environ.pop('MNM_DRAW_COPY_ORDER',None)
if metadata.get('actor_latches'):os.environ['MNM_DRAW_ACTOR_LATCH_FILE']='Z:'+str(root/'capture/actor-latches.bin').replace('/','\\')
if metadata.get('region_input'):os.environ['MNM_DRAW_REGION_INPUT_FILE']='Z:'+str(root/'capture/region-input.bin').replace('/','\\')
if metadata.get('region_track_slot') is not None:os.environ['MNM_DRAW_REGION_TRACK_SLOT']=str(metadata['region_track_slot'])
else:os.environ.pop('MNM_DRAW_REGION_TRACK_SLOT',None)
if metadata.get('cursor_source_gate'):os.environ.update(MNM_DRAW_CURSOR_SOURCE_GATE='1',MNM_DRAW_CURSOR_SOURCE_GATE_FILE='Z:'+str(root/'capture/cursor-gate.bin').replace('/','\\'))
else:os.environ.pop('MNM_DRAW_CURSOR_SOURCE_GATE',None);os.environ.pop('MNM_DRAW_CURSOR_SOURCE_GATE_FILE',None)
if metadata.get('cursor_mutex'):os.environ['MNM_DRAW_CURSOR_MUTEX_FILE']='Z:'+str(root/'capture/cursor-mutex.bin').replace('/','\\')
if metadata.get('semantic_state'):os.environ['MNM_DRAW_SEMANTIC_STATE_FILE']='Z:'+str(root/'capture/semantic-state.bin').replace('/','\\')
else:os.environ.pop('MNM_DRAW_SEMANTIC_STATE_FILE',None)
if metadata.get('update_routes'):os.environ['MNM_DRAW_UPDATE_ROUTES']='1'
else:os.environ.pop('MNM_DRAW_UPDATE_ROUTES',None)
if metadata.get('engine_cadence'):os.environ['MNM_ENGINE_CADENCE_FILE']='Z:'+str(root/'capture/engine-cadence.bin').replace('/','\\')
else:os.environ.pop('MNM_ENGINE_CADENCE_FILE',None)
if metadata.get('canvas_storage_transitions'):os.environ['MNM_DRAW_STORAGE_TRANSITIONS']='1'
else:os.environ.pop('MNM_DRAW_STORAGE_TRANSITIONS',None)
if metadata.get('producer_update_binding'):os.environ['MNM_DRAW_UPDATE_BINDING']='1'
else:os.environ.pop('MNM_DRAW_UPDATE_BINDING',None)
if metadata.get('primitive_lifetimes'):os.environ['MNM_DRAW_PRIMITIVE_LIFETIMES']='1'
else:os.environ.pop('MNM_DRAW_PRIMITIVE_LIFETIMES',None)
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
if metadata.get('wine_gpu_completion'):
    if hashlib.sha256(Path('/usr/lib/wine/i386-windows/opengl32.dll').read_bytes()).hexdigest()!=metadata['opengl32_sha256']:raise ValueError('Installed OpenGL module changed')
    os.environ['MNM_DRAW_WINE_GPU_COMPLETION']='1'
else:os.environ.pop('MNM_DRAW_WINE_GPU_COMPLETION',None)
if metadata.get('wine_source_reservations'):os.environ['MNM_DRAW_WINE_SOURCE_RESERVATIONS']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_RESERVATIONS',None)
if metadata.get('wine_physical_owners'):
    if not metadata.get('wine_source_handoff'):raise ValueError('Live physical ownership requires private source handoff')
    from drawing_physical_owners import pin_prefix_ntdll
    nt_pin=pin_prefix_ntdll(Path(os.environ['WINEPREFIX']))
    (root/'physical-owner-ntdll.json').write_text(json.dumps(nt_pin,indent=2)+'\n')
    os.environ['MNM_DRAW_WINE_PHYSICAL_OWNERS']='1'
else:os.environ.pop('MNM_DRAW_WINE_PHYSICAL_OWNERS',None)
if metadata.get('wine_source_ownership'):
    from drawing_source_ownership import validate_manifest
    validate_manifest(metadata)
    os.environ['MNM_DRAW_WINE_SOURCE_OWNERSHIP']='2' if metadata.get('wine_external_source_leases') else '1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_OWNERSHIP',None)
if metadata.get('wine_source_handoff'):os.environ['MNM_DRAW_WINE_SOURCE_HANDOFF']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_HANDOFF',None)
if metadata.get('wine_source_version'):os.environ['MNM_DRAW_WINE_SOURCE_VERSION']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_VERSION',None)
if metadata.get('wine_source_map'):os.environ['MNM_DRAW_WINE_SOURCE_MAP']='2' if metadata.get('wine_source_map_bounded') else '1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_MAP',None)
if metadata.get('wine_source_descriptor'):os.environ['MNM_DRAW_WINE_SOURCE_DESCRIPTOR']='1'
else:os.environ.pop('MNM_DRAW_WINE_SOURCE_DESCRIPTOR',None)
if metadata.get('wine_resource_map'):os.environ['MNM_DRAW_WINE_RESOURCE_MAP']='1'
else:os.environ.pop('MNM_DRAW_WINE_RESOURCE_MAP',None)
if metadata.get('campaign_storage'):os.environ['MNM_DRAW_ACTION_BOUNDARIES']='1'
else:os.environ.pop('MNM_DRAW_ACTION_BOUNDARIES',None)
os.environ['MNM_DRAW_SESSION_MODE']=metadata['observer_mode']
os.environ['MNM_MENU_OBSERVE']='Z:'+str(root/'events.bin').replace('/','\\')
os.environ['MNM_MENU_CHANNEL']='Z:'+str(root/'channel.bin').replace('/','\\')
os.environ['MNM_MENU_GAMEPLAY_OBSERVE']='Z:'+str(root/'capture/gameplay.bin').replace('/','\\')
if metadata.get('strict_presentation_routes'):
    receipt=(metadata.get('x11_delivery_library') or {}).get('strict_routes_build_manifest_sha256','')
    if not isinstance(receipt,str) or len(receipt)!=64 or any(c not in '0123456789abcdef' for c in receipt):raise ValueError('Strict routes lack a pinned policy-capable observer build')
    os.environ['MNM_DRAW_STRICT_ROUTES']='1'
else:os.environ.pop('MNM_DRAW_STRICT_ROUTES',None)
if observer:=metadata.get('x11_delivery_library'):
    library=Path(observer['path'])
    if hashlib.sha256(library.read_bytes()).hexdigest()!=observer['sha256']:raise ValueError('Unix delivery observer changed')
    if os.environ.get('LD_PRELOAD'):raise ValueError('Unpinned Unix preload')
    os.environ.update(LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(root/'capture'))
    if metadata.get('egl_physical'):os.environ['MNM_DRAW_EGL_PHYSICAL']='1'
    else:os.environ.pop('MNM_DRAW_EGL_PHYSICAL',None)
os.chdir(game);os.execvp('wine',['wine','explorer','/desktop=DrawingSession,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
