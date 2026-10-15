"""Stage and authorize independently guarded native image/effect pixel loops."""
import hashlib
import importlib.util
from pathlib import Path
import shutil
from launch_build_cache import build_adapter

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SITES = ((0x54b31d, 'c744241400000000'), (0x545ce9, '2bc389442414'),
         (0x54b53d, 'c744241c00000000'), (0x54b618, 'c744240c00000000'),
         (0x58d7e7, 'c744241400000000'), (0x58d430, '8b450c33c9'),
         (0x58edb0, '538b471033db'), (0x486aca, 'ff1534505c00'))


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def enabled_from_environment(environment):
    value = environment.get('MNM_PIXEL_TAKEOVER')
    if value not in (None, '0', '1'):
        raise ValueError('MNM_PIXEL_TAKEOVER must be 0 or 1')
    if value == '1' and environment.get('MNM_FADE_TAKEOVER') not in (None, '0'):
        raise ValueError('Pixel takeover already owns the fade splice')
    return value == '1'


def stage(root, original, patched, enabled):
    if type(enabled) is not bool:
        raise ValueError('Invalid native pixel policy')
    if not enabled:
        return patched, {}
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported pixel source executable')
    exporter = load('pixel_offsets', 'tools/export-menu-support.py')
    for address, expected in SITES:
        code = bytes.fromhex(expected)
        at = exporter.image_offset(original, address, len(code))
        if original[at:at+len(code)] != code:
            raise ValueError('Unsupported pixel splice')
    target = root/'game/MnmPixelWriters.dll'
    if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
        raise ValueError('Pixel DLL target escapes disposable game')
    dll = build_adapter('tools/build-pixel-writers.py',
        lambda: load('pixel_build', 'tools/build-pixel-writers.py').build())
    updated = load('pixel_import', 'tools/prepare-shadow-experiment.py').add_import(
        patched, 'MnmPixelWriters.dll', 'PixelWritersAnchor', b'.mnpixel')
    shutil.copy2(dll, target)
    shutil.copy2(dll.parent/'manifest.json', root/'pixel-bridge-build.json')
    return updated, dict(native_pixel_takeover=True,
        pixel_dll_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
        pixel_takeover_scope='Admitted PCX/RGB-add/brighten/copy/fade and owned DIB pixels; original preparation/lifecycle retained')


def authorize(root, metadata, environment):
    environment.pop('MNM_PIXEL_TAKEOVER', None)
    enabled = metadata.get('native_pixel_takeover', False)
    if type(enabled) is not bool:
        raise ValueError('Invalid staged native pixel policy')
    if not enabled:
        return
    if (metadata.get('native_draw') is not True or metadata.get('native_render') is not True
            or metadata.get('origin') != 'menu_action_bridge'):
        raise ValueError('Native pixel writers require native menu/drawing staging')
    if environment.get('MNM_FADE_TAKEOVER') not in (None, '0'):
        raise ValueError('Pixel takeover already owns the fade splice')
    game = root/'game'
    target = game/'MnmPixelWriters.dll'
    if (game.is_symlink() or not game.resolve().is_relative_to(root.resolve()) or target.is_symlink()
            or not target.resolve().is_relative_to(game.resolve())
            or hashlib.sha256(target.read_bytes()).hexdigest() != metadata.get('pixel_dll_sha256')):
        raise ValueError('Native pixel DLL hash/path mismatch')
    environment['MNM_PIXEL_TAKEOVER'] = '1'
