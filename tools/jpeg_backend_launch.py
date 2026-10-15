"""Stage the optional native static-JPEG decoder call and source-only cache."""
import hashlib
import importlib.util
from pathlib import Path
import shutil
from launch_build_cache import build_adapter
from jpeg_pixel_cache import stage as cache_stage, authorize as cache_authorize

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def enabled_from_environment(environment):
    value = environment.get('MNM_JPEG_TAKEOVER')
    if value not in (None, '0', '1'):
        raise ValueError('MNM_JPEG_TAKEOVER must be 0 or 1')
    return value == '1'


def stage(root, original, patched, enabled):
    if type(enabled) is not bool:
        raise ValueError('Invalid native JPEG policy')
    if not enabled:
        return patched, {}
    if hashlib.sha256(original).hexdigest()!=HASH:
        raise ValueError('Unsupported native JPEG source executable')
    exporter = load('jpeg_offsets', 'tools/export-menu-support.py')
    expected = bytes.fromhex('ff1508535c00')
    at = exporter.image_offset(original, 0x58d34b, len(expected))
    if original[at:at+len(expected)]!=expected:
        raise ValueError('Unsupported original JPEG decoder call')
    target = root/'game/MnmJpegPixels.dll'
    if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
        raise ValueError('JPEG DLL target escapes disposable game')
    dll = build_adapter('tools/build-jpeg-pixels.py', lambda: load('jpeg_build', 'tools/build-jpeg-pixels.py').build())
    cache = cache_stage(root)
    updated = load('jpeg_import', 'tools/prepare-shadow-experiment.py').add_import(
        patched, 'MnmJpegPixels.dll', 'JpegPixelsAnchor', b'.mnjpeg')
    shutil.copy2(dll, target)
    shutil.copy2(dll.parent/'manifest.json', root/'jpeg-bridge-build.json')
    return updated, dict(native_jpeg_takeover=True, jpeg_pixel_cache=cache,
        jpeg_dll_sha256=hashlib.sha256(target.read_bytes()).hexdigest())


def authorize(root, metadata, environment):
    for name in ('MNM_JPEG_TAKEOVER', 'MNM_JPEG_PIXELS', 'MNM_JPEG_ROOT'):
        environment.pop(name, None)
    enabled = metadata.get('native_jpeg_takeover', False)
    if type(enabled) is not bool:
        raise ValueError('Invalid staged native JPEG policy')
    if not enabled:
        return
    if (metadata.get('native_draw') is not True or metadata.get('native_render') is not True
            or metadata.get('origin')!='menu_action_bridge' or not metadata.get('jpeg_pixel_cache')):
        raise ValueError('Native JPEG pixels require owned native drawing and source cache')
    game, target = root/'game', root/'game/MnmJpegPixels.dll'
    if (game.is_symlink() or not game.resolve().is_relative_to(root.resolve()) or target.is_symlink()
            or not target.resolve().is_relative_to(game.resolve())
            or hashlib.sha256(target.read_bytes()).hexdigest()!=metadata.get('jpeg_dll_sha256')):
        raise ValueError('Native JPEG DLL hash/path mismatch')
    cache_authorize(root, metadata, environment)
    environment['MNM_JPEG_TAKEOVER'] = '1'
