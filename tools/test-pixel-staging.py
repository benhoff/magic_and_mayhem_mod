#!/usr/bin/env python3
"""Stage real panel/image/JPEG/glyph/minimap imports and verify their launch guards."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from pixel_backend_launch import load, stage as pixels, authorize as pixels_authorize, enabled_from_environment as pixel_mode
from panel_backend_launch import stage as panels, authorize as panels_authorize
from jpeg_backend_launch import stage as jpeg, authorize as jpeg_authorize, enabled_from_environment as jpeg_mode
from minimap_backend_launch import stage as minimap, authorize as minimap_authorize, enabled_from_environment as minimap_mode
from borrowed_pixel_backend_launch import FEATURES, stage as borrowed, authorize as borrowed_authorize, enabled as borrowed_mode, selection
from glyph_backend_launch import stage as glyph, authorize as glyph_authorize

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--full-source', type=Path, help='Also prepare a real isolated source copy through the public menu launcher; no process launch')
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/pixel-staging'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=claims['sources'], claims=claims['claims'], original_sha256=HASH,
        scope='Actual pinned source guard, PE imports/build/copy/native JPEG preparation and all eight pixel/glyph authorization policies, four installed JPEG sizes and denied modes/ownership/hashes/changed sources. No process launch, installed in-memory hooks, renderer stream completion or live gameplay replacement.',
        experiment=str(out.relative_to(ROOT)), live_replacement=False)
    checks = 0

    def deny(action):
        nonlocal checks
        try:
            action()
        except (ValueError, FileNotFoundError):
            checks += 1
            return
        raise ValueError('Invalid staging/launch request was accepted')
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True, stdout=subprocess.DEVNULL)
        original = (ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(original).hexdigest()!=HASH:
            raise ValueError('Unsupported pinned source')
        game = out/'game'
        game.mkdir()
        installed = ROOT/'working/game-nocd'
        selected = {}
        from PIL import Image
        for path in sorted(installed.rglob('*')):
            if path.is_file() and path.suffix.lower() in ('.jpg','.jpeg'):
                with Image.open(path) as image:
                    if image.size in ((48,22),(264,152),(640,480),(800,600)):
                        selected.setdefault(image.size,path)
        if len(selected)!=4:
            raise ValueError('Missing installed JPEG size fixture')
        for source in selected.values():
            target = game/source.relative_to(installed)
            target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(source,target)
        patched = load('pixel_draw_redirect','tools/prepare-native-draw.py').redirect(original,enumeration=True)
        dll = load('pixel_draw_build','tools/build-native-draw.py').build(out/'native-draw-build')
        shutil.copyfile(dll,game/'MnmDraw.dll')
        patched, panel_metadata = panels(out,original,patched,True)
        patched, pixel_metadata = pixels(out,original,patched,True)
        patched, jpeg_metadata = jpeg(out,original,patched,True)
        patched, minimap_metadata = minimap(out,original,patched,True)
        borrowed_metadata = {}
        for name in FEATURES:
            patched, fields = borrowed(out,original,patched,name,True)
            borrowed_metadata.update(fields)
        patched, glyph_metadata = glyph(out,original,patched,'takeover',capture=False)
        metadata = dict(origin='menu_action_bridge',native_draw=True,native_render=True,resident_world=True,
                        **panel_metadata,**pixel_metadata,**jpeg_metadata,**minimap_metadata,**borrowed_metadata,**glyph_metadata)
        env = {}
        panels_authorize(out,metadata,env)
        pixels_authorize(out,metadata,env)
        jpeg_authorize(out,metadata,env)
        minimap_authorize(out,metadata,env)
        for name in FEATURES:borrowed_authorize(out,metadata,env,name)
        glyph_authorize(out,metadata,env)
        if any(env.get(name)!='1' for name in ('MNM_PANEL_TAKEOVER','MNM_PIXEL_TAKEOVER','MNM_JPEG_TAKEOVER','MNM_MINIMAP_TAKEOVER',*(v[0] for v in FEATURES.values()))) or env.get('MNM_GLYPH_CAPTURE')!='0':
            raise ValueError('Combined production activation failed')
        if any(name.encode() not in patched for name in ('MnmDraw.dll','MnmPanelWriters.dll','MnmPixelWriters.dll','MnmJpegPixels.dll','MnmScene.dll','MnmMinimapWriters.dll',*(v[1] for v in FEATURES.values()))):
            raise ValueError('Combined executable lacks an owned adapter import')
        for feature, authorize in [('panel',panels_authorize),('pixel',pixels_authorize),('jpeg',jpeg_authorize),('minimap',minimap_authorize)]:
            key={'panel':'panel_dll_sha256','pixel':'pixel_dll_sha256','jpeg':'jpeg_dll_sha256','minimap':'minimap_dll_sha256'}[feature]
            deny(lambda a=authorize:a(out,dict(metadata,native_draw=False),{}))
            deny(lambda a=authorize,k=key:a(out,{**metadata,k:'0'*64},{}))
        for name in FEATURES:
            deny(lambda n=name:borrowed_authorize(out,dict(metadata,native_draw=False),{},n))
            deny(lambda n=name:borrowed_authorize(out,{**metadata,n+'_dll_sha256':'0'*64},{},n))
            deny(lambda n=name:borrowed_mode({FEATURES[n][0]:'bad'},n))
        deny(lambda:selection({'MNM_OTHER_PIXEL_PRODUCERS':'yes'}))
        grouped = selection({'MNM_OTHER_PIXEL_PRODUCERS':'1'})
        if any(grouped.get(flag)!='1' for flag in ('MNM_PANEL_TAKEOVER','MNM_PIXEL_TAKEOVER','MNM_JPEG_TAKEOVER','MNM_MINIMAP_TAKEOVER',*(v[0] for v in FEATURES.values()))):
            raise ValueError('Grouped pixel producer selection failed')
        deny(lambda:minimap_mode({'MNM_MINIMAP_TAKEOVER':'true'}))
        deny(lambda:pixel_mode({'MNM_PIXEL_TAKEOVER':'bad'}))
        deny(lambda:pixel_mode({'MNM_PIXEL_TAKEOVER':'1','MNM_FADE_TAKEOVER':'1'}))
        deny(lambda:jpeg_mode({'MNM_JPEG_TAKEOVER':'true'}))
        deny(lambda:pixels(out,b'bad',patched,True))
        deny(lambda:jpeg(out,b'bad',patched,True))
        deny(lambda:jpeg_authorize(out,dict(metadata,jpeg_pixel_cache=None),{}))
        stale = {**metadata,'jpeg_pixel_cache':{**jpeg_metadata['jpeg_pixel_cache'],'cache_sha256':'0'*64}}
        deny(lambda:jpeg_authorize(out,stale,{}))
        # Mutate only the disposable source copy; authorize must catch it. Keep
        # the original installed source untouched and retain negative bytes.
        source_entry = next(iter(jpeg_metadata['jpeg_pixel_cache']['inputs'].values()))
        source_path = game/source_entry['path']
        with source_path.open('ab') as file:
            file.write(b'changed-staging-fixture')
        deny(lambda:jpeg_authorize(out,metadata,{}))
        env={'MNM_PANEL_TAKEOVER':'1','MNM_PIXEL_TAKEOVER':'1','MNM_JPEG_TAKEOVER':'1','MNM_MINIMAP_TAKEOVER':'1',**{v[0]:'1' for v in FEATURES.values()}}
        panels_authorize(out,{},env);pixels_authorize(out,{},env);jpeg_authorize(out,{},env);minimap_authorize(out,{},env)
        for name in FEATURES:borrowed_authorize(out,{},env,name)
        if any(name in env for name in ('MNM_PANEL_TAKEOVER','MNM_PIXEL_TAKEOVER','MNM_JPEG_TAKEOVER','MNM_MINIMAP_TAKEOVER',*(v[0] for v in FEATURES.values()))):
            raise ValueError('Inherited pixel modes survived without staged imports')
        if args.full_source:
            source = args.full_source.resolve()
            if not source.is_relative_to(ROOT/'working') or not source.is_dir():
                raise ValueError('Full source must be owned under working/')
            cloned = out/'source-game'
            subprocess.run(['cp','-a','--reflink=auto',source,cloned],check=True)
            if cloned.is_symlink() or not (cloned/'Chaos.exe').resolve().is_relative_to(cloned.resolve()):
                raise ValueError('Source copy executable escapes owned staging')
            (cloned/'Chaos.exe').write_bytes(original)
            inherited = dict(os.environ)
            try:
                for flag in list(os.environ):
                    if flag.startswith('MNM_'):
                        del os.environ[flag]
                os.environ.update(MNM_OTHER_PIXEL_PRODUCERS='1',MNM_GLYPH_BACKEND='takeover',
                                  MNM_VERIFY_ASSETS='0',MNM_VERIFY_ORIGINALS='0',MNM_CACHE_ADAPTERS='1')
                prepare = load('pixel_public_prepare','tools/prepare-menu-observer.py')
                staged_root = prepare.prepare(actions=True,native_render=True,native_draw=True,
                    resident_world=True,source_game=cloned,skip_movies=True)
                full = json.loads((staged_root/'manifest.json').read_text())
                activation = {}
                panels_authorize(staged_root,full,activation);pixels_authorize(staged_root,full,activation)
                jpeg_authorize(staged_root,full,activation);minimap_authorize(staged_root,full,activation)
                for name in FEATURES:borrowed_authorize(staged_root,full,activation,name)
                glyph_authorize(staged_root,full,activation)
                if full.get('glyph_capture') is not False or full.get('resident_world') is not True or len(activation)<8:
                    raise ValueError('Public full source pixel staging/authorization failed')
                report['public_source_staging'] = dict(root=str(staged_root.relative_to(ROOT)),
                    manifest_sha256=sha(staged_root/'manifest.json'),
                    staged_sha256=full['staged_sha256'], source_sha256=full['source_sha256'],
                    native_jpeg_images=full['jpeg_pixel_cache']['decoded_images'],
                    glyph_capture=full['glyph_capture'],resident_world=full['resident_world'])
            finally:
                os.environ.clear();os.environ.update(inherited)
        report.update(source_staging_passed=True, combined_imports=9, native_jpeg_images=4, refusal_checks=checks,
            staged_sha256=hashlib.sha256(patched).hexdigest(), cache_sha256=jpeg_metadata['jpeg_pixel_cache']['cache_sha256'],
            activation={k:v for k,v in metadata.items() if k.startswith('native_')}, success=True)
        (out/'staged.exe').write_bytes(patched)
        (out/'staging-metadata.json').write_text(json.dumps(metadata,indent=2)+'\n')
    except Exception as error:
        report['error'] = repr(error)
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True, stdout=subprocess.DEVNULL)
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in claims['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
