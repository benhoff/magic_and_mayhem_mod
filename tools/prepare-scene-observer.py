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


def copy_installation(source, game):
    # cp -a preserves a directory symlink instead of making an isolated copy.
    source=Path(source).resolve()
    if game.exists() or game.is_symlink():
        raise ValueError('Disposable destination must be new')
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if game.is_symlink() or game.resolve()==source:
        raise ValueError('Disposable game copy aliases its source')
    for name in ['Chaos.exe','MnmScene.dll','MnmMenu.dll','MnmWord.dll','MnmDraw.dll','MnmCreatureDrawing.dll','MnmActorLifecycle.dll','MnmWorldCellProducer.dll','CFG/prefs.cfg']:
        if not (game/name).resolve().is_relative_to(game.resolve()):
            raise ValueError('Disposable write target escapes the game copy: '+name)


def prepare(menu=False,word_mode=None,world_frames=False,world_live=False,source_game=None,glyph_mode=None,native_draw=False,native_creature_drawing=False,native_actor_lifecycle=False,native_world_cell_producer=False):
    if world_live and not world_frames:raise ValueError('Continuous World needs the World input observer')
    if world_frames and word_mode:raise ValueError('World observation and word takeover use separate experiments')
    if word_mode not in (None,'shadow','takeover','clip-shadow','clip-takeover'):raise ValueError('Unsupported word-sprite mode')
    if glyph_mode and (world_frames or world_live or word_mode):raise ValueError('Glyph mode requires a separate experiment')
    source=(Path(source_game) if source_game else ROOT/'working/game-nocd').resolve()
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
    copy_installation(source,game)
    if sha(game/'Chaos.exe')!=HASH:
        raise ValueError('Disposable copy hash mismatch')
    patch=load('scene_import','tools/prepare-shadow-experiment.py').add_import
    staged=patch(data,'MnmScene.dll','SceneAnchor',b'.mnscene')
    shutil.copy2(dll,game/dll.name)
    manifest={'origin':'scene_observation_only','source_sha256':HASH,'scene_dll_sha256':sha(dll),
              'game_copy':str(game),'scope':'Opt-in queue observation; original simulation and drawing remain in control',
              'capture':str(output/'capture'),'menu_driver':menu}
    if world_frames:manifest.update(world_frames=True,world_scope='Owned effective raster inputs and separate full-canvas original oracle; original drawing remains active')
    if world_live:manifest.update(world_live=True,world_channel=str(output/'world-channel.bin'))
    if word_mode:
        for address in (0x597086,0x596cb8):
            at=exporter.image_offset(data,address,6)
            if data[at:at+6]!=bytes.fromhex('558bec565753'):raise ValueError('Unsupported direct-word backend entry')
        word_dll=load('scene_word_build','tools/build-word-sprites.py').build()
        staged=patch(staged,'MnmWord.dll','WordAnchor',b'.mnword')
        shutil.copy2(word_dll,game/word_dll.name)
        shutil.copy2(word_dll.parent/'manifest.json',output/'word-bridge-build.json')
        (output/'word-sprites').mkdir()
        manifest.update(word_sprites_mode=word_mode,word_dll_sha256=sha(word_dll),word_directory=str(output/'word-sprites'))
        manifest['scope']+='; explicit partial direct-word '+word_mode+' route'
    if menu:
        menu_dll=load('scene_menu_build','tools/build-menu-observer.py').build()
        staged=patch(staged,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
        shutil.copy2(menu_dll,game/menu_dll.name)
        manifest['menu_dll_sha256']=sha(menu_dll)
    if native_creature_drawing:
        creature_dll=load('scene_creature_build','tools/build-creature-drawing.py').build(output/'creature-drawing-build')
        staged=patch(staged,'MnmCreatureDrawing.dll','CreatureDrawingAnchor',b'.mncreat')
        shutil.copy2(creature_dll,game/creature_dll.name)
        manifest.update(native_creature_drawing=True,native_creature_drawing_sha256=sha(creature_dll))
        manifest['scope']+='; native creature preparation/history/queue retirement/post-draw cleanup under guarded original orchestration'
    if native_actor_lifecycle:
        actor_dll=load('scene_actor_build','tools/build-actor-lifecycle.py').build(output/'actor-lifecycle-build')
        staged=patch(staged,'MnmActorLifecycle.dll','ActorLifecycleAnchor',b'.mnactor')
        shutil.copy2(actor_dll,game/actor_dll.name)
        manifest.update(native_actor_lifecycle=True,native_actor_lifecycle_sha256=sha(actor_dll))
        manifest['scope']+='; native actor storage/activation/cleanup/release under guarded external services'
    if native_world_cell_producer:
        cell_dll=load('scene_world_cell_build','tools/build-world-cell-producer.py').build(output/'world-cell-build')
        staged=patch(staged,'MnmWorldCellProducer.dll','WorldCellProducerAnchor',b'.mnwcell')
        shutil.copy2(cell_dll,game/cell_dll.name)
        manifest.update(native_world_cell_producer=True,native_world_cell_producer_sha256=sha(cell_dll))
        manifest['scope']+='; native complete world cell production and queue publication under named external services'
    from glyph_backend_launch import stage as stage_glyph
    staged,glyph_metadata=stage_glyph(output,data,staged,glyph_mode,scene_imported=True)
    manifest.update(glyph_metadata)
    if native_draw:
        draw_dll=load('scene_native_draw_build','tools/build-native-draw.py').build(output/'native-draw-build')
        staged=load('scene_native_draw_stage','tools/prepare-native-draw.py').redirect(staged,enumeration=True)
        shutil.copy2(draw_dll,game/draw_dll.name)
        manifest.update(native_draw=True,native_draw_sha256=sha(draw_dll),native_draw_status=str(output/'native-draw-status.bin'),native_draw_startup_retirement=True)
        manifest['scope']+='; native owned graphics services, real DirectDraw must be disabled by the runner'
    # Native graphics changes two guarded loader strings and the capability
    # check's two final Release sites. All other original section bytes match.
    import struct
    pe=struct.unpack_from('<I',data,60)[0];count=struct.unpack_from('<H',data,pe+6)[0]
    table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
    expected=load('scene_native_draw_expected','tools/prepare-native-draw.py').expected_original_sections(data,staged) if native_draw else data
    for i in range(count):
        size,raw=struct.unpack_from('<II',data,table+i*40+16)
        if staged[raw:raw+size]!=expected[raw:raw+size]:
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
    parser.add_argument('--word-sprites',choices=['shadow','takeover','clip-shadow','clip-takeover'],help='Explicit partial direct-word raster experiment')
    parser.add_argument('--native-creature-drawing',action='store_true',help='Use guarded native creature drawing lifecycle helpers')
    parser.add_argument('--native-actor-lifecycle',action='store_true',help='Use guarded native actor allocation, activation, cleanup and release')
    parser.add_argument('--native-world-cell-producer',action='store_true',help='Use native terrain/water/effect/artifact/creature world row production')
    args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        print(prepare(args.menu_driver,args.word_sprites,native_creature_drawing=args.native_creature_drawing,native_actor_lifecycle=args.native_actor_lifecycle,native_world_cell_producer=args.native_world_cell_producer))
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
