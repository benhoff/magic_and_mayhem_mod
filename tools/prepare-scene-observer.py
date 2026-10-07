#!/usr/bin/env python3
"""Stage a pinned disposable scene observation copy; original rendering remains."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    module=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(menu=False):
    source=ROOT/'working/game-nocd'
    data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:
        raise ValueError('Unsupported source executable; no patch applied')
    exporter=load('scene_pe','tools/export-menu-support.py')
    at=exporter.image_offset(data,0x5002a0,9)
    if data[at:at+9]!=bytes.fromhex('83 ec 18 53 b8 00 6c ca 88'):
        raise ValueError('Unsupported queue consumer entry')
    dll=load('scene_build','tools/build-scene-observer.py').build()
    parent=ROOT/'working/experiments/scene-observer'
    parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    game=output/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if sha(game/'Chaos.exe')!=HASH:
        raise ValueError('Disposable copy hash mismatch')
    patch=load('scene_import','tools/prepare-shadow-experiment.py').add_import
    staged=patch(data,'MnmScene.dll','SceneAnchor',b'.mnscene')
    shutil.copy2(dll,game/dll.name)
    manifest={'origin':'scene_observation_only','source_sha256':HASH,'scene_dll_sha256':sha(dll),
              'game_copy':str(game),'scope':'Opt-in queue observation; original simulation and drawing remain in control',
              'capture':str(output/'capture'),'menu_driver':menu}
    if menu:
        menu_dll=load('scene_menu_build','tools/build-menu-observer.py').build()
        staged=patch(staged,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
        shutil.copy2(menu_dll,game/menu_dll.name)
        manifest['menu_dll_sha256']=sha(menu_dll)
    # Every old file-backed code section remains byte-for-byte identical.
    import struct
    pe=struct.unpack_from('<I',data,60)[0];count=struct.unpack_from('<H',data,pe+6)[0]
    table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
    for i in range(count):
        size,raw=struct.unpack_from('<II',data,table+i*40+16)
        if staged[raw:raw+size]!=data[raw:raw+size]:
            raise ValueError('An original section changed during import staging')
    (game/'Chaos.exe').write_bytes(staged)
    settings=load('scene_settings','tools/run-opengl-game.py')
    manifest['preferences']=settings.disable_cd_music(game)+settings.skip_movies(game)
    manifest['staged_sha256']=sha(game/'Chaos.exe')
    (output/'capture').mkdir()
    shutil.copy2(dll.parent/'manifest.json',output/'bridge-build.json')
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    if sha(source/'Chaos.exe')!=HASH:
        raise ValueError('Source executable changed')
    return output


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--menu-driver',action='store_true',help='Include the established opt-in menu adapter for bounded automatic battle startup')
    args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        print(prepare(args.menu_driver))
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
