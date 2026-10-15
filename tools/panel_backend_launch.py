"""Stage and admit the partial post-Lock panel producer takeover."""
import hashlib
import importlib.util
from pathlib import Path
import shutil

from launch_build_cache import build_adapter

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def enabled_from_environment(environment):
    value = environment.get('MNM_PANEL_TAKEOVER')
    if value not in (None, '0', '1'):
        raise ValueError('MNM_PANEL_TAKEOVER must be 0 or 1')
    return value == '1'


def stage(root, original, patched, enabled):
    if type(enabled) is not bool:
        raise ValueError('Invalid native panel policy')
    if not enabled:
        return patched, {}
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported panel source executable')
    exporter = load('panel_offsets', 'tools/export-menu-support.py')
    for address, expected in ((0x58de5a, '8b54245c894424288bea'),
                              (0x58d92c, '8b7c244c89442430'),
                              (0x58e32c, '8b4c24388bd8895c2430')):
        code = bytes.fromhex(expected)
        at = exporter.image_offset(original, address, len(code))
        if original[at:at+len(code)] != code:
            raise ValueError('Unsupported panel splice')
    target = root/'game/MnmPanelWriters.dll'
    if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
        raise ValueError('Panel DLL target escapes disposable game')
    dll = build_adapter('tools/build-panel-writers.py',
                        lambda: load('panel_build', 'tools/build-panel-writers.py').build())
    updated = load('panel_import', 'tools/prepare-shadow-experiment.py').add_import(
        patched, 'MnmPanelWriters.dll', 'PanelWritersAnchor', b'.mnpanel')
    shutil.copy2(dll, target)
    shutil.copy2(dll.parent/'manifest.json', root/'panel-bridge-build.json')
    return updated, dict(native_panel_takeover=True,
                         panel_dll_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
                         panel_takeover_scope='Admitted bevel/highlight/whitewash pixel loops; original Lock/Unlock and other producers retained')


def authorize(root, metadata, environment):
    # Inherited mode cannot enable a hook without this session's staged import.
    environment.pop('MNM_PANEL_TAKEOVER', None)
    enabled = metadata.get('native_panel_takeover', False)
    if type(enabled) is not bool:
        raise ValueError('Invalid staged native panel policy')
    if not enabled:
        return
    if (metadata.get('native_draw') is not True or metadata.get('native_render') is not True
            or metadata.get('origin') != 'menu_action_bridge'):
        raise ValueError('Native panels require native menu/drawing staging')
    game = root/'game'
    target = game/'MnmPanelWriters.dll'
    if (game.is_symlink() or not game.resolve().is_relative_to(root.resolve()) or target.is_symlink()
            or not target.resolve().is_relative_to(game.resolve())
            or hashlib.sha256(target.read_bytes()).hexdigest() != metadata.get('panel_dll_sha256')):
        raise ValueError('Native panel DLL hash/path mismatch')
    environment['MNM_PANEL_TAKEOVER'] = '1'
