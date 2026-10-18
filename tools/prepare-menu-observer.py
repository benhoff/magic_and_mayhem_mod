#!/usr/bin/env python3
"""Stage a hash-checked disposable installation with the opt-in menu observer."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sys
import subprocess
import tempfile
sys.path.insert(0, str(Path(__file__).resolve().parent))
from launch_build_cache import build_adapter
from launch_asset_checks import full_asset_verification
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def prepare(actions=False,experimental_mini=False,preferences_store=None,campaign_observe=False,native_render=False,native_draw=False,resident_world=False,native_voices=False,source_game=None,native_media=False,native_music=False,music_assets=None,skip_movies=False,grimoire_observe=False,original_gameplay=False):
    from rts_controls_launch import enabled_from_environment as rts_enabled,stage as stage_rts
    rts_controls=rts_enabled(os.environ)
    if rts_controls and original_gameplay:
        raise ValueError('Controls enhancements cannot be combined with passive original-gameplay evidence')
    from borrowed_pixel_backend_launch import FEATURES,selection,enabled as borrowed_enabled,stage as stage_borrowed
    pixel_environment=selection(os.environ)
    from pixel_forwarding_guard import selection as forwarding_guard_selection
    no_pixel_forwarding=forwarding_guard_selection(pixel_environment,native_draw=native_draw,native_render=native_render,actions=actions)
    borrowed={name:borrowed_enabled(pixel_environment,name) for name in FEATURES}
    if any(borrowed.values()) and not (native_draw and native_render and actions):
        raise ValueError('Native pixel producers require native menu/drawing staging')
    from panel_backend_launch import enabled_from_environment as panels_enabled,stage as stage_panels
    panel_takeover=panels_enabled(pixel_environment)
    from pixel_backend_launch import enabled_from_environment as pixels_enabled,stage as stage_pixels
    pixel_takeover=pixels_enabled(pixel_environment)
    from jpeg_backend_launch import enabled_from_environment as jpeg_enabled,stage as stage_jpeg
    jpeg_takeover=jpeg_enabled(pixel_environment)
    from minimap_backend_launch import enabled_from_environment as minimap_enabled,stage as stage_minimap
    minimap_takeover=minimap_enabled(pixel_environment)
    if minimap_takeover and not (native_draw and native_render and actions):
        raise ValueError('Native minimap pixels require native menu/drawing staging')
    if jpeg_takeover and not (native_draw and native_render and actions):
        raise ValueError('Native JPEG pixels require native menu/drawing staging')
    if pixel_takeover and not (native_draw and native_render and actions):
        raise ValueError('Native pixel writers require native menu/drawing staging')
    if panel_takeover and not (native_draw and native_render and actions):
        raise ValueError('Native panels require native menu/drawing staging')
    if type(original_gameplay) is not bool:raise ValueError('Invalid original gameplay observation policy')
    if original_gameplay and (not campaign_observe or actions or native_render or native_draw or resident_world or native_voices):
        raise ValueError('Original gameplay observation requires campaign observation without native rendering or actions')
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
    dll=build_adapter('tools/build-menu-observer.py',lambda: module('menu_build','tools/build-menu-observer.py').build(experimental_mini=experimental_mini),options={'experimental_mini':experimental_mini})
    parent=REPO/'working/experiments/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if game.is_symlink() or game.resolve()==source:
        raise ValueError('Disposable staging aliases its source')
    for name in ['Chaos.exe','MnmRender.dll','MnmMenu.dll','MnmScene.dll','MnmWord.dll','MnmAudio.dll','MnmDraw.dll','MnmPanelWriters.dll','MnmPixelWriters.dll','MnmJpegPixels.dll','MnmMinimapWriters.dll','MnmColourRect.dll','MnmRasterWriters.dll','MnmDcCopy.dll','MnmEngineCadence.dll','MnmSceneProduction.dll','MnmApiCensus.dll','CFG/prefs.cfg','CFG/Encrypted/prefs.cfg']:
        if not (game/name).resolve().is_relative_to(game.resolve()):
            raise ValueError('Disposable write target escapes game copy: '+name)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=HASH:raise ValueError('Copy hash mismatch')
    patched=data;draw_dll=None
    if native_draw:
        draw_dll=build_adapter('tools/build-native-draw.py',lambda: module('menu_native_draw_build','tools/build-native-draw.py').build(root/'native-draw-build'))
        patched=module('menu_native_draw_import','tools/prepare-native-draw.py').redirect(patched,enumeration=True)
        shutil.copy2(draw_dll,game/'MnmDraw.dll')
        shutil.copy2(draw_dll.parent/'manifest.json',root/'native-draw-build.json')
    patched=module('menu_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
    render_dll=None
    if native_render:
        if not actions:raise ValueError('Native rendering requires the menu action bridge')
        render_dll=build_adapter('tools/build-render-bridge.py',lambda: module('menu_render_build','tools/build-render-bridge.py').build())
        patched=module('menu_render_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmRender.dll','RenderAnchor',b'.mnmgl')
        shutil.copy2(render_dll,game/render_dll.name)
        shutil.copy2(render_dll.parent/'manifest.json',root/'render-bridge-build.json')
        (root/'lock-capture').mkdir()
    audio_dll=None
    if native_voices:
        audio_dll=build_adapter('tools/build-audio-bridge.py',lambda: module('menu_audio_build','tools/build-audio-bridge.py').build())
        patched=module('menu_audio_import','tools/prepare-native-audio.py').redirect(data,patched)
        shutil.copy2(audio_dll,game/'MnmAudio.dll')
        shutil.copy2(audio_dll.parent/'manifest.json',root/'audio-bridge-build.json')
    from glyph_backend_launch import stage as stage_glyph,mode_from_environment
    glyph_mode=mode_from_environment()
    patched,glyph_metadata=stage_glyph(root,data,patched,None if resident_world else glyph_mode,
        capture=not (native_draw and native_render and glyph_mode=='takeover'))
    patched,panel_metadata=stage_panels(root,data,patched,panel_takeover)
    patched,pixel_metadata=stage_pixels(root,data,patched,pixel_takeover)
    patched,jpeg_metadata=stage_jpeg(root,data,patched,jpeg_takeover)
    patched,minimap_metadata=stage_minimap(root,data,patched,minimap_takeover)
    borrowed_metadata={}
    for name,active in borrowed.items():
        patched,fields=stage_borrowed(root,data,patched,name,active)
        borrowed_metadata.update(fields)
    resident_metadata={}
    if resident_world:
        scene=build_adapter('tools/build-scene-observer.py',lambda: module('public_world_scene_build','tools/build-scene-observer.py').build())
        patched=module('public_world_scene_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmScene.dll','SceneAnchor',b'.mnworld')
        shutil.copy2(scene,game/'MnmScene.dll');shutil.copy2(scene.parent/'manifest.json',root/'scene-bridge-build.json')
        (root/'resident-world').mkdir()
        resident_metadata=dict(resident_world=True,owned_world_snapshot=os.environ.get('MNM_WORLD_SNAPSHOT')=='1',resident_scene_sha256=hashlib.sha256(scene.read_bytes()).hexdigest())
        patched,glyph_metadata=stage_glyph(root,data,patched,mode_from_environment(),scene_imported=True,capture=False)
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
        if production not in ('0','1','2','3','4','5') or not resident_world or cadence is not None:raise ValueError('Scene production requires isolated resident diagnostic staging')
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
    patched,rts_metadata=stage_rts(root,data,patched,rts_controls,os.environ.get('MNM_RTS_CONTROLS_CONFIG',REPO/'patches/rts-controls.json'))
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
              'original_gameplay':original_gameplay,'grimoire_observe':grimoire_observe,'campaign_observe':campaign_observe,'experimental_mini':experimental_mini,'preference_store':stored,'preferences':edits,'scope':('UNVALIDATED Mini Menu bridge; dedicated validation staging; original confirmation/drawing retained' if experimental_mini else 'Bounded Main/Quick/Single Player/Map/Spell/Quick results/Main Preferences engine-thread action bridge; original menu logic/drawing retained') if actions else 'Bounded callback/tick observation; original menu/drawing/actions retained',
              'native_pixel_no_forward':no_pixel_forwarding,'native_draw':native_draw,'native_draw_sha256':hashlib.sha256(draw_dll.read_bytes()).hexdigest() if draw_dll else None,'native_draw_status':str(root/'native-draw-status.bin') if native_draw else None,
              'native_render':native_render,'render_dll_sha256':hashlib.sha256(render_dll.read_bytes()).hexdigest() if render_dll else None,
              'native_draw_cadence':native_cadence,
              'patch':'Additional DLL import; original callback bytes unchanged; in-memory guarded observation hooks'}
    from native_only_contract import selected, POLICY
    metadata.update(native_only_policy=POLICY if selected(os.environ) else None, skip_movies=skip_movies)
    metadata.update(glyph_metadata);metadata.update(resident_metadata);metadata.update(cadence_metadata);metadata.update(production_metadata);metadata.update(census_metadata)
    metadata.update(panel_metadata)
    metadata.update(pixel_metadata)
    metadata.update(jpeg_metadata)
    metadata.update(minimap_metadata)
    metadata.update(borrowed_metadata)
    metadata.update(rts_metadata)
    metadata.update(native_voices=native_voices,native_initial_silence=native_voices,native_audio_profiles=native_voices,native_audio_file_sizes=native_voices,audio_dll_sha256=hashlib.sha256(audio_dll.read_bytes()).hexdigest() if audio_dll else None,
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
    from directplay_dx6_launch import stage as stage_directplay
    metadata['directplay_dx6']=stage_directplay(root)
    from menu_save_store import stage as stage_saves
    metadata['save_store']=stage_saves(game,REPO/'working/user-saves/native-menu') if actions else None
    shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    metadata['full_asset_verification']=full_asset_verification()
    if native_draw and metadata['full_asset_verification']:
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
    parser.add_argument('--original-gameplay',action='store_true',help='Explicit passive creature/spell diagnostics with original display and controls; requires --campaign-observe, excludes actions/native rendering')
    parser.add_argument('--grimoire-observe',action='store_true',help='Observe original Grimoire custom tick; no native book dispatch or ownership')
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
    if os.environ.get('MNM_VERIFY_ORIGINALS','1')!='0':subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:print(f'Evidence directory: {prepare(args.actions,args.experimental_mini,args.preferences_store,args.campaign_observe,args.native_render,args.native_draw,args.resident_world,args.native_voices,args.source_game,args.native_media,args.native_music,args.music_assets,args.skip_movies,grimoire_observe=args.grimoire_observe,original_gameplay=args.original_gameplay)}',flush=True)
    finally:
        if os.environ.get('MNM_VERIFY_ORIGINALS','1')!='0':subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
