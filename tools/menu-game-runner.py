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
os.environ['MNM_MENU_OBSERVE']='Z:'+str(events).replace('/','\\')
if os.environ.get('MNM_MENU_NATIVE_RENDER')=='1':
    if not metadata.get('native_render'):raise ValueError('Native rendering was not staged')
    if hashlib.sha256((game/'MnmRender.dll').read_bytes()).hexdigest()!=metadata['render_dll_sha256']:raise ValueError('Render DLL hash mismatch')
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
    os.environ.update(MNM_RENDER_CONTINUOUS='1',MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_NO_READBACK='1',MNM_RENDER_ORDERED_COPIES='1')
os.chdir(game)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
