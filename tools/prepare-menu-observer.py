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
def prepare(actions=False,experimental_mini=False,preferences_store=None,campaign_observe=False):
    if experimental_mini and not actions:raise ValueError("Experimental Mini Menu requires action staging")
    source=REPO/'working/game-nocd';data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported executable; refusing staging')
    dll=module('menu_build','tools/build-menu-observer.py').build(experimental_mini=experimental_mini)
    parent=REPO/'working/experiments/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Copy hash mismatch')
    patched=module('menu_import','tools/prepare-shadow-experiment.py').add_import(data,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
    (game/'Chaos.exe').write_bytes(patched);shutil.copy2(dll,game/dll.name)
    preferences=module('menu_preferences','tools/run-opengl-game.py')
    stored=None
    if preferences_store:
        encoder=preferences.load('preferences_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(REPO)
        stored=module('preferences_store','tools/menu_preferences_store.py').stage(game,preferences_store,encoder,decoder)
        if stored['warning']:print('Preferences store ignored: '+stored['warning'],flush=True)
    edits=preferences.disable_cd_music(game)+preferences.skip_movies(game)
    # Source engine code stays byte-identical; only the additional import is staged.
    exporter=module('menu_export','tools/export-menu-support.py')
    for start,end in [*exporter.CALLBACKS.values(),(0x4a9700,0x4a97c7),(0x4a9840,0x4a9b00),(0x4ce730,0x4ce798),(0x4cdf40,0x4cdfe4),(0x4a88f0,0x4a8ae6),(0x4a8b60,0x4a8bcd)]:
        at=exporter.image_offset(data,start,end-start)
        if patched[at:at+end-start]!=data[at:at+end-start]:raise ValueError('Callback code changed during staging')
    metadata={'origin':'menu_action_bridge' if actions else 'menu_observation_only','source_sha256':HASH,'staged_sha256':hashlib.sha256(patched).hexdigest(),
              'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'game_copy':str(game),'events':str(root/'events.bin'),
              'campaign_observe':campaign_observe,'experimental_mini':experimental_mini,'preference_store':stored,'preferences':edits,'scope':('UNVALIDATED Mini Menu bridge; dedicated validation staging; original confirmation/drawing retained' if experimental_mini else 'Bounded Main/Quick/Single Player/Map/Spell/Quick results/Main Preferences engine-thread action bridge; original menu logic/drawing retained') if actions else 'Bounded callback/tick observation; original menu/drawing/actions retained',
              'patch':'Additional DLL import; original callback bytes unchanged; in-memory guarded observation hooks'}
    shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    (root/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    if hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Source executable changed')
    return root
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--actions',action='store_true',help='Stage the opt-in bounded menu action bridge')
    parser.add_argument("--experimental-mini",action="store_true",help="UNVALIDATED: build Mini Menu hooks for a dedicated future validation run")
    parser.add_argument("--preferences-store",type=Path,help="Import only exposed Preferences settings into the disposable copy")
    parser.add_argument('--campaign-observe',action='store_true',help='Observe original Realm custom tick in a separate bounded diagnostic log')
    args=parser.parse_args()
    if args.experimental_mini and not args.actions:parser.error("--experimental-mini requires --actions")
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:print(f'Evidence directory: {prepare(args.actions,args.experimental_mini,args.preferences_store,args.campaign_observe)}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
