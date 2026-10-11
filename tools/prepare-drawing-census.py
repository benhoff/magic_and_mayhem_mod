#!/usr/bin/env python3
"""Stage a fresh read-only drawing census in a disposable installation."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    return module


def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(source, movies=False, menu_only=False):
    source = source.resolve(); raw = (source / 'Chaos.exe').read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256: raise ValueError('Unsupported source executable')
    census = load('drawing_build', 'tools/build-drawing-census.py').build(source / 'Chaos.exe')
    menu = load('drawing_menu', 'tools/build-menu-observer.py').build()
    parent = ROOT / 'working/experiments/drawing-census'; parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); game = root / 'game'
    load('drawing_isolation', 'tools/prepare-scene-observer.py').copy_installation(source, game)
    if sha(game / 'Chaos.exe') != BUILD_SHA256: raise ValueError('Disposable source copy hash mismatch')
    # The pinned executable uses Save\\ for .sav files and __temp.vas.
    # Source-media installations can omit this empty writable output directory.
    saves = game / 'Save'
    if not saves.resolve().is_relative_to(game.resolve()): raise ValueError('Save output escapes disposable copy')
    created_directories = [] if saves.exists() else ['Save']
    saves.mkdir(exist_ok=True)
    for path in [game / 'MnmDrawingCensus.dll', game / 'MnmMenu.dll']:
        if not path.resolve().is_relative_to(game.resolve()): raise ValueError('DLL destination escapes disposable copy')
    patch = load('drawing_import', 'tools/prepare-shadow-experiment.py').add_import
    staged = raw if menu_only else patch(raw, census.name, 'DrawingCensusAnchor', b'.mndraw')
    staged = patch(staged, menu.name, 'MenuAnchor', b'.mnmenu')
    pe = struct.unpack_from('<I', raw, 60)[0]; count = struct.unpack_from('<H', raw, pe + 6)[0]
    table = pe + 24 + struct.unpack_from('<H', raw, pe + 20)[0]
    for i in range(count):
        size, offset = struct.unpack_from('<II', raw, table + i * 40 + 16)
        if staged[offset:offset + size] != raw[offset:offset + size]: raise ValueError('Original section bytes changed')
    (game / 'Chaos.exe').write_bytes(staged); shutil.copy2(census, game / census.name); shutil.copy2(menu, game / menu.name)
    settings = load('drawing_settings', 'tools/run-opengl-game.py')
    edits = settings.disable_cd_music(game)
    edits += settings.patch_staged_preferences(game, 'VIDEO', {'PlayFMV': 'TRUE', 'PlayFMVOut': 'TRUE'}) if movies else settings.skip_movies(game)
    capture = root / 'capture'; capture.mkdir()
    control = bytearray(64); control[:16] = b'MNMDCTL1' + struct.pack('<II', 1, 64)
    struct.pack_into('<III', control, 16, 2, 1 if movies else 2, 1); (capture / 'census-control.bin').write_bytes(control)
    channel = bytearray(106496); channel[:16] = b'MNMMCM12' + struct.pack('<II', 12, len(channel))
    struct.pack_into('<III', channel, 16, 2, 1, 1); (root / 'channel.bin').write_bytes(channel)
    shutil.copy2(census.parent / 'manifest.json', root / 'census-build.json')
    shutil.copy2(menu.parent / 'manifest.json', root / 'menu-build.json')
    manifest = {'schema': 1, 'origin': 'drawing_census_observation', 'original_sha256': BUILD_SHA256,
                'binaries': {name: sha(game / name) for name in ('Chaos.exe', 'MnmDrawingCensus.dll', 'MnmMenu.dll')},
                'movies_enabled': movies, 'preferences': edits, 'source_game': str(source),
                'created_directories': created_directories,
                'call_census_enabled': not menu_only,
                'scope': 'Original-active signature-pinned drawing call metadata; no native rendering or gameplay changes.'}
    (root / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    if sha(source / 'Chaos.exe') != BUILD_SHA256: raise ValueError('Original source executable changed')
    return root


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--source-game', type=Path, required=True)
    parser.add_argument('--movies', action='store_true')
    parser.add_argument('--menu-only', action='store_true', help='Diagnostic control: original drawing entries are unpatched')
    args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: print(prepare(args.source_game, args.movies, args.menu_only), flush=True)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
