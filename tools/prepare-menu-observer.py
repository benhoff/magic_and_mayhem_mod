#!/usr/bin/env python3
"""Stage a hash-checked disposable installation with the opt-in menu observer."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def prepare(actions=False,experimental_mini=False,preferences_store=None,campaign_observe=False,native_render=False,native_draw=False,resident_world=False,native_voices=False,source_game=None,native_media=False,native_music=False,music_assets=None,skip_movies=False):
    if native_voices and not (native_render and actions):raise ValueError('Native voices require native menus and command presentation')
    if native_media and not (native_render and actions):raise ValueError('Native media requires native menus and command presentation')
    if native_music and not (native_voices and native_render and actions):raise ValueError('Native music requires native voice/menu/render staging')
    if resident_world and not (native_render and native_draw and actions):raise ValueError("Resident World requires native menus, commands and owned drawing")
    if native_draw and not (native_render and actions):raise ValueError('Native graphics requires native menus and command presentation')
    if experimental_mini and not actions:raise ValueError("Experimental Mini Menu requires action staging")
    source=(Path(source_game) if source_game else REPO/'working/game-nocd').resolve()
    if not source.is_relative_to(REPO/'working') or not source.is_dir():raise ValueError('Menu source game must be a directory under working/')
    data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported executable; refusing staging')
    dll=module('menu_build','tools/build-menu-observer.py').build(experimental_mini=experimental_mini)
    parent=REPO/'working/experiments/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if game.is_symlink() or game.resolve()==source:
        raise ValueError('Disposable staging aliases its source')
    for name in ['Chaos.exe','MnmRender.dll','MnmMenu.dll','MnmScene.dll','MnmWord.dll','MnmAudio.dll','MnmDraw.dll','MnmEngineCadence.dll','MnmSceneProduction.dll','MnmApiCensus.dll','CFG/prefs.cfg','CFG/Encrypted/prefs.cfg']:
        if not (game/name).resolve().is_relative_to(game.resolve()):
            raise ValueError('Disposable write target escapes game copy: '+name)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Copy hash mismatch')
    patched=data;draw_dll=None
    if native_draw:
        draw_dll=module('menu_native_draw_build','tools/build-native-draw.py').build(root/'native-draw-build')
        patched=module('menu_native_draw_import','tools/prepare-native-draw.py').redirect(patched,enumeration=True)
        shutil.copy2(draw_dll,game/'MnmDraw.dll')
        shutil.copy2(draw_dll.parent/'manifest.json',root/'native-draw-build.json')
    patched=module('menu_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
    render_dll=None
    if native_render:
        if not actions:raise ValueError('Native rendering requires the menu action bridge')
        render_dll=module('menu_render_build','tools/build-render-bridge.py').build()
        patched=module('menu_render_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmRender.dll','RenderAnchor',b'.mnmgl')
        shutil.copy2(render_dll,game/render_dll.name)
        shutil.copy2(render_dll.parent/'manifest.json',root/'render-bridge-build.json')
        (root/'lock-capture').mkdir()
    audio_dll=None
    if native_voices:
        audio_dll=module('menu_audio_build','tools/build-audio-bridge.py').build()
        patched=module('menu_audio_import','tools/prepare-native-audio.py').redirect(data,patched)
        shutil.copy2(audio_dll,game/'MnmAudio.dll')
        shutil.copy2(audio_dll.parent/'manifest.json',root/'audio-bridge-build.json')
    from glyph_backend_launch import stage as stage_glyph,mode_from_environment
    patched,glyph_metadata=stage_glyph(root,data,patched,None if resident_world else mode_from_environment())
    resident_metadata={}
    if resident_world:
        scene=module('public_world_scene_build','tools/build-scene-observer.py').build()
        patched=module('public_world_scene_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmScene.dll','SceneAnchor',b'.mnworld')
        shutil.copy2(scene,game/'MnmScene.dll');shutil.copy2(scene.parent/'manifest.json',root/'scene-bridge-build.json')
        (root/'resident-world').mkdir()
        resident_metadata=dict(resident_world=True,resident_scene_sha256=hashlib.sha256(scene.read_bytes()).hexdigest())
    cadence_metadata={}
    cadence=os.environ.get('MNM_ENGINE_CADENCE_TRACE')
    if cadence is not None:
        if cadence!='1' or not resident_world:raise ValueError('Engine cadence requires explicit resident diagnostic staging')
        cadence_dll=module('engine_cadence_build','tools/build-engine-cadence.py').build(root/'engine-cadence-build')
        patched=module('engine_cadence_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmEngineCadence.dll','EngineCadenceAnchor',b'.mncad')
        shutil.copy2(cadence_dll,game/cadence_dll.name)
        cadence_metadata=dict(engine_cadence=True,engine_cadence_sha256=hashlib.sha256(cadence_dll.read_bytes()).hexdigest())
    production_metadata={}
    production=os.environ.get('MNM_SCENE_PRODUCTION_PROBE')
    if production is not None:
        if production not in ('0','1','2','3','4') or not resident_world or cadence is not None:raise ValueError('Scene production requires isolated resident diagnostic staging')
        production_dll=module('scene_production_build','tools/build-scene-production.py').build(root/'scene-production-build')
        patched=module('scene_production_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmSceneProduction.dll','SceneProductionAnchor',b'.mnprod')
        shutil.copy2(production_dll,game/production_dll.name)
        production_metadata=dict(scene_production_mode=production,scene_production_sha256=hashlib.sha256(production_dll.read_bytes()).hexdigest())
    census_metadata={}
    census=os.environ.get('MNM_WINE_IMPORT_CENSUS')
    if census is not None:
        if census!='1':raise ValueError('Import census requires explicit diagnostic1')
        from wine_import_census import catalog
        expected=module('census_import_export','tools/export-wine-imports.py').entries(data)
        if [(entry['rva'],entry['module'],entry['symbol']) for entry in expected]!=catalog():raise ValueError('Original import catalog drift')
        census_dll=module('census_build','tools/build-api-census.py').build(root/'api-census-build')
        # Install after all other imported adapters, so their guarded IAT
        # installation sees original targets and the observer counts final ones.
        patched=module('census_import_stage','tools/prepare-shadow-experiment.py').add_import(patched,'MnmApiCensus.dll','ApiCensusAnchor',b'.mnapi')
        shutil.copy2(census_dll,game/census_dll.name);shutil.copy2(census_dll.parent/'manifest.json',root/'api-census-build.json')
        census_metadata=dict(wine_import_census=True,wine_import_census_file=str(root/'wine-import-census.bin'),wine_import_census_dll_sha256=hashlib.sha256(census_dll.read_bytes()).hexdigest())
    (game/'Chaos.exe').write_bytes(patched);shutil.copy2(dll,game/dll.name)
    preferences=module('menu_preferences','tools/run-opengl-game.py')
    stored=None
    if preferences_store:
        encoder=preferences.load('preferences_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(REPO)
        stored=module('preferences_store','tools/menu_preferences_store.py').stage(game,preferences_store,encoder,decoder)
        if stored['warning']:print('Preferences store ignored: '+stored['warning'],flush=True)
    edits=(preferences.patch_staged_preferences(game,'SOUND',{'CDMusicEnabled':'TRUE'}) if native_music else preferences.disable_cd_music(game))+(preferences.patch_staged_preferences(game,'VIDEO',{'PlayFMV':'TRUE','PlayFMVOut':'TRUE'}) if native_media and not skip_movies else preferences.skip_movies(game))
    native_cadence=None
    if native_render:
        encoder=preferences.load('native_cfg_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(REPO)
        native_cadence=module('native_render_config','tools/native_render_config.py').stage_native_draw_cadence(
            game,lambda text:encoder.encode(text,decoder)[0],lambda packed:decoder.decode(packed)[1])
    # Source engine code stays byte-identical; only the additional import is staged.
    exporter=module('menu_export','tools/export-menu-support.py')
    for start,end in [*exporter.CALLBACKS.values(),(0x4a9700,0x4a97c7),(0x4a9840,0x4a9b00),(0x4ce730,0x4ce798),(0x4cdf40,0x4cdfe4),(0x4a88f0,0x4a8ae6),(0x4a8b60,0x4a8bcd)]:
        at=exporter.image_offset(data,start,end-start)
        if patched[at:at+end-start]!=data[at:at+end-start]:raise ValueError('Callback code changed during staging')
    metadata={'origin':'menu_action_bridge' if actions else 'menu_observation_only','source_game':str(source),'source_sha256':HASH,'staged_sha256':hashlib.sha256(patched).hexdigest(),
              'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'game_copy':str(game),'events':str(root/'events.bin'),
              'campaign_observe':campaign_observe,'experimental_mini':experimental_mini,'preference_store':stored,'preferences':edits,'scope':('UNVALIDATED Mini Menu bridge; dedicated validation staging; original confirmation/drawing retained' if experimental_mini else 'Bounded Main/Quick/Single Player/Map/Spell/Quick results/Main Preferences engine-thread action bridge; original menu logic/drawing retained') if actions else 'Bounded callback/tick observation; original menu/drawing/actions retained',
              'native_draw':native_draw,'native_draw_sha256':hashlib.sha256(draw_dll.read_bytes()).hexdigest() if draw_dll else None,'native_draw_status':str(root/'native-draw-status.bin') if native_draw else None,
              'native_render':native_render,'render_dll_sha256':hashlib.sha256(render_dll.read_bytes()).hexdigest() if render_dll else None,
              'native_draw_cadence':native_cadence,
              'patch':'Additional DLL import; original callback bytes unchanged; in-memory guarded observation hooks'}
    metadata.update(glyph_metadata);metadata.update(resident_metadata);metadata.update(cadence_metadata);metadata.update(production_metadata);metadata.update(census_metadata)
    metadata.update(native_voices=native_voices,native_initial_silence=native_voices,audio_dll_sha256=hashlib.sha256(audio_dll.read_bytes()).hexdigest() if audio_dll else None,
                    native_wave_loading=native_voices,
                    native_audio_factory=native_voices,
                    native_extended_controls=native_voices,
                    native_output_recovery=native_voices,
                    audio_channel=str(root/'native-voices.bin') if native_voices else None)
    metadata.update(native_media=native_media,native_media_startup_probe=native_media,
                    media_channel=str(root/'native-media.bin') if native_media else None)
    if native_music:
        from native_music_launch import stage as stage_music
        music_source=(music_assets or REPO/'working/assets/native-cd-audio-20261012').resolve()
        if not music_source.is_relative_to(REPO/'working'):raise ValueError('Prepared music source must be under working/')
        metadata.update(stage_music(root,music_source))
    shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    if native_draw:
        immutable_inputs={q.relative_to(game).as_posix():hashlib.sha256(q.read_bytes()).hexdigest() for q in game.rglob('*') if q.is_file() and q.suffix.lower() in ('.exe','.dll','.spr','.bmp','.jpg','.jpeg','.pcx','.wav','.ini','.avi')}
        (root/'input-fingerprints.json').write_text(json.dumps(immutable_inputs,indent=2)+'\n')
    (root/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    if hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Source executable changed')
    return root
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--actions',action='store_true',help='Stage the opt-in bounded menu action bridge')
    parser.add_argument("--experimental-mini",action="store_true",help="UNVALIDATED: build Mini Menu hooks for a dedicated future validation run")
    parser.add_argument("--preferences-store",type=Path,help="Import only exposed Preferences settings into the disposable copy")
    parser.add_argument('--campaign-observe',action='store_true',help='Observe original Realm custom tick in a separate bounded diagnostic log')
    parser.add_argument('--native-render',action='store_true',help='Stage native command rendering alongside native menus')
    parser.add_argument('--resident-world',action='store_true',help='Bind resident GPU World completion to the public command stream')
    parser.add_argument('--native-draw',action='store_true',help='Use owned native graphics services with real DirectDraw disabled')
    parser.add_argument('--native-voices',action='store_true',help='Stage native DirectSound voices for the native menu/render session')
    parser.add_argument('--native-media',action='store_true',help='Route movies/file sounds through the session-owned native broker')
    parser.add_argument('--native-music',action='store_true',help='Stage prepared CD tracks for the owned native music broker')
    parser.add_argument('--music-assets',type=Path,help='Prepared CD catalog under working/')
    parser.add_argument('--skip-movies',action='store_true',help='Disable staged intro playback while retaining native file sounds')
    parser.add_argument('--source-game',type=Path,help='Pinned source installation under working/; default working/game-nocd')
    args=parser.parse_args()
    if args.experimental_mini and not args.actions:parser.error("--experimental-mini requires --actions")
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:print(f'Evidence directory: {prepare(args.actions,args.experimental_mini,args.preferences_store,args.campaign_observe,args.native_render,args.native_draw,args.resident_world,args.native_voices,args.source_game,args.native_media,args.native_music,args.music_assets,args.skip_movies)}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
