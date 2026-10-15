#!/usr/bin/env python3
"""Run a hash-checked render experiment inside the launcher's Wine desktop."""
import hashlib
import json
from launch_asset_checks import verify_staged_assets
import os
from pathlib import Path
import sys
import subprocess
root=Path(os.environ['MNM_RENDER_EXPERIMENT']);metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
binaries=[('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]
if metadata.get('audio_dll_sha256'):binaries.append(('MnmAudio.dll','audio_dll_sha256'))
if metadata.get('word_dll_sha256'):binaries.append(('MnmWord.dll','word_dll_sha256'))
native_draw=metadata.get('native_draw',False)
if type(native_draw) is not bool:raise ValueError('Invalid native graphics policy')
if native_draw:binaries.append(('MnmDraw.dll','native_draw_sha256'))
for name,key in binaries:
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
os.environ.pop('MNM_NATIVE_DRAW_STATUS',None)
if native_draw:
    status=root/'native-draw-status.bin'
    if metadata.get('native_draw_status')!=str(status) or status.exists():raise ValueError('Refusing redirected or stale native graphics status')
    if verify_staged_assets(metadata):
        inputs=json.loads((root/'input-fingerprints.json').read_text())
        for name,digest in inputs.items():
            path=game/name
            if Path(name).is_absolute() or '..' in Path(name).parts or hashlib.sha256(path.read_bytes()).hexdigest()!=digest:raise ValueError('Native graphics input changed before launch: '+name)
    os.environ['MNM_NATIVE_DRAW_STATUS']='Z:'+str(status).replace('/','\\')
    overrides=os.environ.get('WINEDLLOVERRIDES','')
    os.environ['WINEDLLOVERRIDES']=(overrides+';' if overrides else '')+'ddraw='
os.environ.pop('MNM_WORD_DIRECTORY',None)
os.environ.pop('MNM_WORD_SPRITES',None)
if metadata.get('word_dll_sha256'):
    if metadata.get('word_sprites_mode') not in ('shadow','takeover','clip-shadow','clip-takeover'):raise ValueError('Unsupported word-sprite mode')
    directory=Path(metadata['word_directory']).resolve()
    if directory!=root.resolve()/'word-sprites' or any(directory.iterdir()):raise ValueError('Word-sprite capture directory must be fresh')
    os.environ['MNM_WORD_SPRITES']=metadata['word_sprites_mode']
    os.environ['MNM_WORD_DIRECTORY']='Z:'+str(directory).replace('/','\\')
from glyph_backend_launch import authorize as authorize_glyph
authorize_glyph(root,metadata,os.environ)
from rts_controls_launch import authorize as authorize_rts
authorize_rts(root,metadata,os.environ)
os.environ.pop('MNM_SCENE_DIR',None)
os.chdir(game)
if metadata.get('capture_locks'):
    timing=root/'presentation-rate.jsonl'
    try:
        subprocess.Popen([sys.executable,str(Path(__file__).resolve().with_name('profile-render-stream.py')),
                          metadata['stream'],'--output',str(timing),'--parent-pid',str(os.getpid())],
                         stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,close_fds=True)
        print(f'Render timing: {timing}',flush=True)
    except OSError as error:
        print(f'Render timing monitor could not start: {error}',flush=True)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
