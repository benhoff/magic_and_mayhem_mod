#!/usr/bin/env python3
"""Hash-checked disposable scene observer runner for tools/run-game.sh."""
import hashlib
import json
import os
from pathlib import Path
import sys
from world_channel import validate_fresh

root=Path(os.environ['MNM_SCENE_EXPERIMENT']).resolve()
metadata=json.loads((root/'manifest.json').read_text())
if metadata['origin']!='scene_observation_only':
    raise ValueError('Unsupported experiment')
game=root/'game'
for name,key in [('Chaos.exe','staged_sha256'),('MnmScene.dll','scene_dll_sha256')]+([('MnmMenu.dll','menu_dll_sha256')] if metadata['menu_driver'] else [])+([('MnmWord.dll','word_dll_sha256')] if metadata.get('word_dll_sha256') else []):
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:
        raise ValueError('Staged binary hash mismatch: '+name)
os.environ.pop('MNM_WORD_DIRECTORY',None)
os.environ.pop('MNM_WORD_SPRITES',None)
os.environ.pop('MNM_SCENE_WORLD',None)
os.environ.pop('MNM_WORLD_CHANNEL',None)
if metadata.get('world_frames'):os.environ['MNM_SCENE_WORLD']='1'
if metadata.get('world_live'):
    channel=Path(metadata['world_channel']).resolve()
    if channel!=root/'world-channel.bin' or not channel.is_file():raise ValueError('Invalid continuous World channel')
    validate_fresh(channel)
    os.environ['MNM_WORLD_CHANNEL']='Z:'+str(channel).replace('/','\\')
if metadata.get('word_dll_sha256'):
    directory=Path(metadata['word_directory']).resolve()
    if metadata.get('word_sprites_mode') not in ('shadow','takeover','clip-shadow','clip-takeover') or directory!=root/'word-sprites' or any(directory.iterdir()):raise ValueError('Invalid word-sprite experiment')
    os.environ['MNM_WORD_SPRITES']=metadata['word_sprites_mode']
    os.environ['MNM_WORD_DIRECTORY']='Z:'+str(directory).replace('/','\\')
os.environ.pop('MNM_WORLD_CONTINUOUS',None)
if metadata.get('world_continuous'):
    channel=Path(metadata['world_continuous']).resolve()
    if channel!=root/'world-continuous.bin' or not channel.is_file() or not 1<=metadata.get('continuous_queues',0)<=4096:raise ValueError('Invalid continuous producer session')
    import struct
    with channel.open('rb') as f:header=f.read(128)
    if len(header)!=128 or header[:8]!=b'MNMCON02' or struct.unpack_from('<II',header,8)!=(2,128) or struct.unpack_from('<4I',header,32)!=(1,0,0,0) or struct.unpack_from('<Q',header,48)[0]!=metadata['continuous_queues']:raise ValueError('Stale or mismatched continuous session')
    if channel.stat().st_size!=struct.unpack_from('<I',header,16)[0]:raise ValueError('Continuous session extent')
    os.environ['MNM_WORLD_CONTINUOUS']='Z:'+str(channel).replace('/','\\')
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
