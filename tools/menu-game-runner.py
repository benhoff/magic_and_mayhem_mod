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
if os.environ.get('MNM_MENU_CAMPAIGN_OBSERVE'):
    campaign=root/'campaign.bin'
    expected='Z:'+str(campaign).replace('/','\\')
    if not metadata.get('campaign_observe') or os.environ['MNM_MENU_CAMPAIGN_OBSERVE']!=expected or campaign.exists():
        raise ValueError('Refusing unapproved or stale campaign observation output')
os.environ.pop('MNM_NATIVE_DRAW_STATUS',None)
native_draw=metadata.get('native_draw',False)
if type(native_draw) is not bool:raise ValueError('Invalid native graphics policy')
if native_draw:
    if not metadata.get('native_render') or os.environ.get('MNM_MENU_NATIVE_RENDER')!='1':raise ValueError('Native graphics requires native command presentation')
    if hashlib.sha256((game/'MnmDraw.dll').read_bytes()).hexdigest()!=metadata['native_draw_sha256']:raise ValueError('Native graphics DLL hash mismatch')
    status=root/'native-draw-status.bin'
    if metadata.get('native_draw_status')!=str(status) or status.exists():raise ValueError('Refusing redirected or stale native graphics status')
    inputs=json.loads((root/'input-fingerprints.json').read_text())
    for name,digest in inputs.items():
        path=game/name
        if Path(name).is_absolute() or '..' in Path(name).parts or hashlib.sha256(path.read_bytes()).hexdigest()!=digest:raise ValueError('Native graphics input changed before launch: '+name)
    os.environ['MNM_NATIVE_DRAW_STATUS']='Z:'+str(status).replace('/','\\')
    overrides=os.environ.get('WINEDLLOVERRIDES','')
    os.environ['WINEDLLOVERRIDES']=(overrides+';' if overrides else '')+'ddraw='
os.environ['MNM_MENU_OBSERVE']='Z:'+str(events).replace('/','\\')
if os.environ.get('MNM_CAMPAIGN_CASTING_COMBAT')=='1':
    trace=root/'gameplay-events.bin'
    if not metadata.get('native_render') or trace.exists():raise ValueError('Refusing unapproved or stale gameplay observation')
    os.environ['MNM_MENU_GAMEPLAY_OBSERVE']='Z:'+str(trace).replace('/','\\')
    spells=root/'spell-events.bin'
    if spells.exists():raise ValueError('Refusing stale spell observation')
    os.environ['MNM_MENU_SPELL_OBSERVE']='Z:'+str(spells).replace('/','\\')
if os.environ.get('MNM_MENU_NATIVE_RENDER')=='1':
    if not metadata.get('native_render'):raise ValueError('Native rendering was not staged')
    if hashlib.sha256((game/'MnmRender.dll').read_bytes()).hexdigest()!=metadata['render_dll_sha256']:raise ValueError('Render DLL hash mismatch')
    cadence=metadata.get('native_draw_cadence',{})
    if cadence.get('settings')!={'SkipFrameEvery':0,'SkipXFrames':0,'MaxSkipXFrames':0} or not cadence.get('files'):raise ValueError('Native draw cadence was not staged')
    for entry in cadence['files']:
        if entry['path'] not in ('CFG/chaos.cfg','CFG/Encrypted/chaos.cfg') or hashlib.sha256((game/entry['path']).read_bytes()).hexdigest()!=entry['after_sha256']:raise ValueError('Native draw-cadence config changed before launch')
    import struct
    sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
    from mnm_protocols import frame_v1,input_v1,render_commands_v2,render_control_v1
    for name,wire in [('render-frame.bin',frame_v1),('render-input.bin',input_v1),('render-commands.bin',render_commands_v2),('render-commands.bin.control',render_control_v1)]:
        path=root/name
        with path.open('rb') as file:header=file.read(wire.HEADER_SIZE)
        if not wire.valid_header(header,path.stat().st_size):raise ValueError('Invalid native channel: '+name)
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
    if len(header)!=128 or header[:8]!=b'MNMCON04' or struct.unpack_from('<I',header,8)[0]!=4 or any(header[36:]):raise ValueError('Resident World channel must be fresh V4')
    def winepath(path):return 'Z:'+str(path).replace('/',chr(92))
    os.environ.update(MNM_PUBLIC_RESIDENT_WORLD='1',MNM_CANVAS_PRODUCERS='1',MNM_SCENE_SAMPLES='32',MNM_WORLD_RASTER_PREFIX='32',MNM_WORLD_RASTER_BATCH='1',MNM_WORLD_CONTINUOUS=winepath(world),MNM_SCENE_DIR=winepath(directory))
    # Keep post-World hooks distinct from the finite legacy glyph observer.
    os.environ.pop('MNM_GLYPH_BACKEND',None);os.environ.pop('MNM_GLYPH_DIR',None)
else:
    os.environ.pop('MNM_PUBLIC_RESIDENT_WORLD',None)
os.chdir(game)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
