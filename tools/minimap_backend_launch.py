"""Stage and authorize independently guarded native minimap pixel passes."""
import hashlib
import importlib.util
from pathlib import Path
import shutil
from launch_build_cache import build_adapter

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SITES = ((0x553a6f, '8b462985c0'), (0x553799, '668b9451cb000000'),
         (0x553986, '8bb9eb000000'), (0x55280b, '398e07620000'),
         (0x552bdc, '398e07620000'), (0x552fac, '398e07620000'),
         (0x55337c, '398e07620000'))


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def enabled_from_environment(environment):
    value = environment.get('MNM_MINIMAP_TAKEOVER')
    if value not in (None, '0', '1'):
        raise ValueError('MNM_MINIMAP_TAKEOVER must be 0 or 1')
    return value == '1'


def stage(root, original, patched, enabled):
    if type(enabled) is not bool:
        raise ValueError('Invalid native minimap policy')
    if not enabled:
        return patched, {}
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported minimap source executable')
    exporter = load('minimap_offsets', 'tools/export-menu-support.py')
    for address, expected in SITES:
        code = bytes.fromhex(expected)
        at = exporter.image_offset(original, address, len(code))
        if original[at:at+len(code)] != code:
            raise ValueError('Unsupported minimap splice')
    target = root/'game/MnmMinimapWriters.dll'
    if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
        raise ValueError('Minimap DLL target escapes disposable game')
    dll = build_adapter('tools/build-minimap-writers.py',
        lambda: load('minimap_build', 'tools/build-minimap-writers.py').build())
    updated = load('minimap_import', 'tools/prepare-shadow-experiment.py').add_import(
        patched, 'MnmMinimapWriters.dll', 'MinimapWritersAnchor', b'.mnmap')
    shutil.copy2(dll, target)
    shutil.copy2(dll.parent/'manifest.json', root/'minimap-bridge-build.json')
    return updated, dict(native_minimap_takeover=True,
        minimap_dll_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
        minimap_takeover_scope='Admitted terrain/cell/creature/camera pixel passes; original preparation/lifecycle retained')


def authorize(root, metadata, environment):
    environment.pop('MNM_MINIMAP_TAKEOVER', None)
    enabled = metadata.get('native_minimap_takeover', False)
    if type(enabled) is not bool:
        raise ValueError('Invalid staged native minimap policy')
    if not enabled:
        return
    if (metadata.get('native_draw') is not True or metadata.get('native_render') is not True
            or metadata.get('origin') != 'menu_action_bridge'):
        raise ValueError('Native minimap writers require native menu/drawing staging')
    game = root/'game'
    target = game/'MnmMinimapWriters.dll'
    if (game.is_symlink() or not game.resolve().is_relative_to(root.resolve()) or target.is_symlink()
            or not target.resolve().is_relative_to(game.resolve())
            or hashlib.sha256(target.read_bytes()).hexdigest() != metadata.get('minimap_dll_sha256')):
        raise ValueError('Native minimap DLL hash/path mismatch')
    environment['MNM_MINIMAP_TAKEOVER'] = '1'
