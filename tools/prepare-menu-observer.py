#!/usr/bin/env python3
"""Stage a hash-checked disposable installation with the opt-in menu observer."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def prepare():
    source=REPO/'working/game-nocd';data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported executable; refusing staging')
    dll=module('menu_build','tools/build-menu-observer.py').build()
    parent=REPO/'working/experiments/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Copy hash mismatch')
    patched=module('menu_import','tools/prepare-shadow-experiment.py').add_import(data,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
    (game/'Chaos.exe').write_bytes(patched);shutil.copy2(dll,game/dll.name)
    preferences=module('menu_preferences','tools/run-opengl-game.py')
    edits=preferences.disable_cd_music(game)+preferences.skip_movies(game)
    # Source engine code stays byte-identical; only the additional import is staged.
    exporter=module('menu_export','tools/export-menu-support.py')
    for start,end in exporter.CALLBACKS.values():
        at=exporter.image_offset(data,start,end-start)
        if patched[at:at+end-start]!=data[at:at+end-start]:raise ValueError('Callback code changed during staging')
    metadata={'origin':'menu_observation_only','source_sha256':HASH,'staged_sha256':hashlib.sha256(patched).hexdigest(),
              'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'game_copy':str(game),'events':str(root/'events.bin'),
              'preferences':edits,'scope':'Bounded callback/tick observation; original menu/drawing/actions retained',
              'patch':'Additional DLL import; original callback bytes unchanged; in-memory guarded observation hooks'}
    shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    (root/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    if hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Source executable changed')
    return root
if __name__=='__main__':
    argparse.ArgumentParser(description=__doc__).parse_args()
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:print(f'Evidence directory: {prepare()}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
