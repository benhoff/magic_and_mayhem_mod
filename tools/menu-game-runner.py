#!/usr/bin/env python3
"""Hash-checked disposable game runner used only through tools/run-game.sh."""
import hashlib
import json
from launch_asset_checks import verify_staged_assets
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
from terrain_light_session import authorize as authorize_lighting
authorize_lighting(root,metadata,os.environ)
from native_audio_launch import authorize as authorize_audio
authorize_audio(root,metadata,os.environ)
from native_music_launch import authorize as authorize_music
authorize_music(root,metadata,os.environ)
from native_media_launch import authorize as authorize_media
authorize_media(root,metadata,os.environ)
from wine_import_census import authorize as authorize_census
authorize_census(root,metadata,os.environ)
from directplay_dx6_launch import authorize as authorize_directplay
authorize_directplay(root,metadata,os.environ)
from panel_backend_launch import authorize as authorize_panels
authorize_panels(root,metadata,os.environ)
from pixel_backend_launch import authorize as authorize_pixels
authorize_pixels(root,metadata,os.environ)
from rts_controls_launch import authorize as authorize_rts
authorize_rts(root,metadata,os.environ)
from jpeg_backend_launch import authorize as authorize_jpeg
authorize_jpeg(root,metadata,os.environ)
from minimap_backend_launch import authorize as authorize_minimap
authorize_minimap(root,metadata,os.environ)
from borrowed_pixel_backend_launch import FEATURES,authorize as authorize_borrowed,reject_retired_dc_hook
reject_retired_dc_hook(os.environ,metadata)
for name in FEATURES:authorize_borrowed(root,metadata,os.environ,name)
os.environ.pop('MNM_OTHER_PIXEL_PRODUCERS',None)
os.environ.pop('MNM_NATIVE_PIXEL_COUNTERS',None)
if all(os.environ.get(flag)=='1' for flag in ('MNM_PANEL_TAKEOVER','MNM_PIXEL_TAKEOVER',
       'MNM_JPEG_TAKEOVER','MNM_MINIMAP_TAKEOVER',*(value[0] for value in FEATURES.values()))):
    counters=game/'native-pixel-producers.bin'
    if counters.exists() or counters.is_symlink():raise ValueError('Stale or redirected pixel producer counters')
    os.environ['MNM_NATIVE_PIXEL_COUNTERS']='1'
from pixel_forwarding_guard import authorize as authorize_forwarding_guard
authorize_forwarding_guard(root,metadata,os.environ)
if os.environ.get('MNM_MENU_CHANNEL'):
    expected='Z:'+str(root/'channel.bin').replace('/','\\')
    if metadata['origin']!='menu_action_bridge' or os.environ['MNM_MENU_CHANNEL']!=expected:raise ValueError('Refusing unapproved command channel')
if os.environ.get('MNM_MENU_CAMPAIGN_OBSERVE'):
    campaign=root/'campaign.bin'
    expected='Z:'+str(campaign).replace('/','\\')
    if not metadata.get('campaign_observe') or os.environ['MNM_MENU_CAMPAIGN_OBSERVE']!=expected or campaign.exists():
        raise ValueError('Refusing unapproved or stale campaign observation output')
from native_cursor_launch import authorize as authorize_cursor
authorize_cursor(root,metadata,os.environ)
os.environ.pop('MNM_NATIVE_DRAW_STATUS',None)
os.environ.pop('MNM_NATIVE_DRAW_RETIREMENT',None)
native_draw=metadata.get('native_draw',False)
if type(native_draw) is not bool:raise ValueError('Invalid native graphics policy')
if native_draw:
    if not metadata.get('native_render') or os.environ.get('MNM_MENU_NATIVE_RENDER')!='1':raise ValueError('Native graphics requires native command presentation')
    if hashlib.sha256((game/'MnmDraw.dll').read_bytes()).hexdigest()!=metadata['native_draw_sha256']:raise ValueError('Native graphics DLL hash mismatch')
    status=root/'native-draw-status.bin'
    if metadata.get('native_draw_status')!=str(status) or status.exists():raise ValueError('Refusing redirected or stale native graphics status')
    if verify_staged_assets(metadata):
        inputs=json.loads((root/'input-fingerprints.json').read_text())
        for name,digest in inputs.items():
            path=game/name
            if Path(name).is_absolute() or '..' in Path(name).parts or hashlib.sha256(path.read_bytes()).hexdigest()!=digest:raise ValueError('Native graphics input changed before launch: '+name)
    os.environ['MNM_NATIVE_DRAW_STATUS']='Z:'+str(status).replace('/','\\')
    retirement=root/'native-draw-retirement.bin'
    if retirement.exists():raise ValueError('Refusing stale native graphics retirement output')
    os.environ['MNM_NATIVE_DRAW_RETIREMENT']='Z:'+str(retirement).replace('/','\\')
    overrides=os.environ.get('WINEDLLOVERRIDES','')
    os.environ['WINEDLLOVERRIDES']=(overrides+';' if overrides else '')+'ddraw='
os.environ.pop('MNM_ENGINE_CADENCE_FILE',None)
if metadata.get('engine_cadence'):
    if os.environ.get('MNM_ENGINE_CADENCE_TRACE')!='1' or not metadata.get('resident_world'):raise ValueError('Unapproved engine cadence tracing')
    if hashlib.sha256((game/'MnmEngineCadence.dll').read_bytes()).hexdigest()!=metadata['engine_cadence_sha256']:raise ValueError('Engine cadence DLL changed')
    trace=root/'engine-cadence.bin'
    if trace.exists() or trace.is_symlink():raise ValueError('Stale or redirected engine cadence trace')
    os.environ['MNM_ENGINE_CADENCE_FILE']='Z:'+str(trace).replace('/','\\')
elif os.environ.get('MNM_ENGINE_CADENCE_TRACE') is not None:
    raise ValueError('Engine cadence observer was not staged')
os.environ.pop('MNM_SCENE_PRODUCTION_FILE',None);os.environ.pop('MNM_SCENE_PRODUCTION_MODE',None)
if metadata.get('scene_production_mode') is not None:
    mode=metadata['scene_production_mode']
    if mode not in ('0','1','2','3','4','5') or os.environ.get('MNM_SCENE_PRODUCTION_PROBE')!=mode or not metadata.get('resident_world'):raise ValueError('Unapproved scene production probe')
    if hashlib.sha256((game/'MnmSceneProduction.dll').read_bytes()).hexdigest()!=metadata['scene_production_sha256']:raise ValueError('Scene production DLL changed')
    trace=root/'scene-production.bin'
    if trace.exists() or trace.is_symlink():raise ValueError('Stale or redirected scene production output')
    os.environ['MNM_SCENE_PRODUCTION_FILE']='Z:'+str(trace).replace('/','\\');os.environ['MNM_SCENE_PRODUCTION_MODE']=mode
elif os.environ.get('MNM_SCENE_PRODUCTION_PROBE') is not None:
    raise ValueError('Scene production observer was not staged')
os.environ['MNM_MENU_OBSERVE']='Z:'+str(events).replace('/','\\')
original_gameplay=metadata.get('original_gameplay',False)
if type(original_gameplay) is not bool:raise ValueError('Invalid original gameplay observation policy')
if original_gameplay:
    if (os.environ.get('MNM_MENU_ORIGINAL_GAMEPLAY')!='1' or metadata['origin']!='menu_observation_only'
        or metadata.get('campaign_observe') is not True or metadata.get('native_render') is not False or native_draw
        or metadata.get('native_voices') is not False or metadata.get('resident_world',False) or os.environ.get('MNM_MENU_CHANNEL')
        or os.environ.get('MNM_MENU_CAMPAIGN_OBSERVE')!='Z:'+str(root/'campaign.bin').replace('/','\\')
        or os.environ.get('MNM_MENU_NATIVE_RENDER')=='1' or os.environ.get('MNM_CAMPAIGN_CASTING_COMBAT')=='1'):
        raise ValueError('Refusing unapproved original gameplay observation')
elif os.environ.get('MNM_MENU_ORIGINAL_GAMEPLAY') is not None:
    raise ValueError('Original gameplay observation was not staged')
if original_gameplay or os.environ.get('MNM_CAMPAIGN_CASTING_COMBAT')=='1':
    trace=root/'gameplay-events.bin'
    if (not original_gameplay and not metadata.get('native_render')) or trace.exists() or trace.is_symlink():raise ValueError('Refusing unapproved or stale gameplay observation')
    os.environ['MNM_MENU_GAMEPLAY_OBSERVE']='Z:'+str(trace).replace('/','\\')
    spells=root/'spell-events.bin'
    if spells.exists() or spells.is_symlink():raise ValueError('Refusing stale spell observation')
    os.environ['MNM_MENU_SPELL_OBSERVE']='Z:'+str(spells).replace('/','\\')
os.environ.pop('MNM_MENU_INPUT_ADMISSION_OBSERVE',None)
if os.environ.get('MNM_CAMPAIGN_INPUT_ADMISSION')=='1':
    if os.environ.get('MNM_CAMPAIGN_CASTING_COMBAT')!='1' or not metadata.get('native_render'):
        raise ValueError('Input admission observation requires the private casting diagnostic')
    admission=root/'input-admission.bin'
    if admission.exists() or admission.is_symlink():raise ValueError('Stale input admission observation')
    os.environ['MNM_MENU_INPUT_ADMISSION_OBSERVE']='Z:'+str(admission).replace('/','\\')
    region=root/'input-admission-region.bin'
    if region.exists() or region.is_symlink():raise ValueError('Stale Region input observation')
    os.environ['MNM_DRAW_REGION_INPUT_FILE']='Z:'+str(region).replace('/','\\')
if os.environ.get('MNM_MENU_NATIVE_RENDER')=='1':
    if not metadata.get('native_render'):raise ValueError('Native rendering was not staged')
    if hashlib.sha256((game/'MnmRender.dll').read_bytes()).hexdigest()!=metadata['render_dll_sha256']:raise ValueError('Render DLL hash mismatch')
    cadence=metadata.get('native_draw_cadence',{})
    if cadence.get('settings')!={'SkipFrameEvery':0,'SkipXFrames':0,'MaxSkipXFrames':0} or not cadence.get('files'):raise ValueError('Native draw cadence was not staged')
    for entry in cadence['files']:
        if entry['path'] not in ('CFG/chaos.cfg','CFG/Encrypted/chaos.cfg') or hashlib.sha256((game/entry['path']).read_bytes()).hexdigest()!=entry['after_sha256']:raise ValueError('Native draw-cadence config changed before launch')
    import struct
    sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
    from mnm_protocols import frame_v1,input_v1,render_commands_v2,render_commands_v3,render_control_v1
    for name,wire in [('render-frame.bin',frame_v1),('render-input.bin',input_v1),('render-commands.bin',render_commands_v3),('render-commands.bin.control',render_control_v1)]:
        path=root/name
        with path.open('rb') as file:header=file.read(wire.HEADER_SIZE)
        if not (wire.valid_header(header,path.stat().st_size) or (name=='render-commands.bin' and render_commands_v2.valid_header(header,path.stat().st_size))):raise ValueError('Invalid native channel: '+name)
    commands=(root/'render-commands.bin').read_bytes()[:64]
    control=(root/'render-commands.bin.control').read_bytes()[:512]
    if not struct.unpack_from('<I',commands,16)[0] or any(commands[20:]) or control[16:20]!=commands[16:20]:raise ValueError('Native command channels are stale or mismatched')
    paths={'MNM_RENDER_STREAM':'render-frame.bin','MNM_RENDER_INPUT':'render-input.bin','MNM_RENDER_COMMAND_CHANNEL':'render-commands.bin','MNM_RENDER_CONTROL':'render-commands.bin.control','MNM_RENDER_LOCK_CAPTURE_DIR':'lock-capture','MNM_RENDER_FAILURE_LOG':'surface-failures.log'}
    for key,name in paths.items():os.environ[key]='Z:'+str(root/name).replace('/','\\')
    if os.environ.get('MNM_CAMPAIGN_PACER_YIELD','0')=='1':os.environ['MNM_RENDER_PACER_YIELD']='1'
    if os.environ.get('MNM_CAMPAIGN_PRECISE_CLOCK','1')=='1':os.environ['MNM_RENDER_PRECISE_CLOCK']='1'
    os.environ.update(MNM_RENDER_CONTINUOUS='1',MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_NO_READBACK='1',MNM_RENDER_ORDERED_COPIES='1')
from glyph_backend_launch import authorize as authorize_glyph
authorize_glyph(root,metadata,os.environ)
os.environ.pop('MNM_SCENE_DIR',None)
if metadata.get('resident_world'):
    if not native_draw or not metadata.get('native_render'):raise ValueError('Resident World requires owned native presentation')
    if hashlib.sha256((game/'MnmScene.dll').read_bytes()).hexdigest()!=metadata['resident_scene_sha256']:raise ValueError('Resident scene DLL changed')
    directory=root/'resident-world'
    if directory.is_symlink() or not directory.is_dir() or any(directory.iterdir()):raise ValueError('Resident World outputs must be fresh and owned')
    world=root/'world-continuous.bin'
    import struct
    with world.open('rb') as f:header=f.read(128)
    snapshot=metadata.get('owned_world_snapshot') is True
    if snapshot!=(os.environ.get('MNM_WORLD_SNAPSHOT')=='1'):raise ValueError('Owned World snapshot activation mismatch')
    version=5 if snapshot else 6 if header[:8]==b'MNMCON06' else 4
    if len(header)!=128 or header[:8]!=f'MNMCON0{version}'.encode() or struct.unpack_from('<I',header,8)[0]!=version or struct.unpack_from('<I',header,20)[0]!={4:1,5:2,6:3}[version] or any(header[36:]):raise ValueError('Resident World channel must be fresh negotiated V4/V5/V6')
    def winepath(path):return 'Z:'+str(path).replace('/',chr(92))
    os.environ.update(MNM_PUBLIC_RESIDENT_WORLD='1',MNM_CANVAS_PRODUCERS='1',MNM_SCENE_SAMPLES='32',MNM_WORLD_RASTER_PREFIX='32',MNM_WORLD_RASTER_BATCH='1',MNM_WORLD_CONTINUOUS=winepath(world),MNM_SCENE_DIR=winepath(directory))
    # Native glyphs share the resident producer entry and publish through its
    # normal return hook. Capture-disabled takeover has no finite call budget.
else:
    os.environ.pop('MNM_PUBLIC_RESIDENT_WORLD',None)
from native_audio_launch import directsound_disabled
with (root/'native-audio-launch-policy.json').open('x') as policy:
    json.dump(dict(schema=1,native_audio_factory=metadata.get('native_audio_factory',False),
                   native_media=metadata.get('native_media',False),
                   native_media_startup_probe=metadata.get('native_media_startup_probe',False),
                   wine_directsound_disabled=directsound_disabled(os.environ)),policy,indent=2)
    policy.write('\n')
os.chdir(game)
if metadata.get('directplay_dx6') is not None:
    from directplay_dx6_launch import launch as launch_directplay
    raise SystemExit(launch_directplay(root,metadata,sys.argv[2:]))
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
