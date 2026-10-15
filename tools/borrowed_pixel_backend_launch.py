"""Stage bounded CPU raster/rectangle writers independently of World GPU ownership."""
import hashlib
import shutil
from pixel_backend_launch import HASH, load
from launch_build_cache import build_adapter

FEATURES = {
    'colour_rect': ('MNM_COLOUR_RECT_TAKEOVER', 'MnmColourRect.dll', 'ColourRectAnchor',
                    'tools/build-colour-rect.py', b'.mnrect', ((0x547e53, '8b471285c0'),)),
    'raster': ('MNM_RASTER_TAKEOVER', 'MnmRasterWriters.dll', 'RasterWritersAnchor',
               'tools/build-raster-writers.py', b'.mnrast', (
                   (0x5948a9, '8b7d0c033d681e5f00'), (0x594ada, '8b7d0c033d681e5f00'),
                   (0x595311, '8b7d0c033d681e5f00'), (0x5954e0, '8b7d0c033d681e5f00'),
                   (0x595c3f, '8b7d0c033d681e5f00'), (0x595e65, '8b7d0c033d681e5f00'),
                   (0x596136, '8b7d0c033d681e5f00'), (0x596301, '8b7d0c033d681e5f00'),
                   (0x595731, '8b0dc82d6a00'), (0x596d78, '8b7d0c033d681e5f00'),
                   (0x596efd, '8b7d0c033d681e5f00'), (0x597146, '8b7d0c033d681e5f00'),
                   (0x597310, '8b7d0c033d681e5f00'), (0x596550, '8b7d0c033d681e5f00'),
                   (0x596964, '8b7d0c033d681e5f00'), (0x57e210, '8b7004894c2418'),
                   (0x57ed0e, '8b7810894c2410'), (0x57f16c, '894424148b470c'),
                   (0x57f671, '8b4b1089442410'), (0x580782, '8b790c8b542434'),
                   (0x57e57c, '8b790c8b7104'), (0x589cd3, '8d1c163bdd'),
                   (0x57de69, '8b45102bc1'))),
}


def selection(environment):
    """Optional group selects scoped pixel writers; it is not a whole renderer flag."""
    value = environment.get('MNM_OTHER_PIXEL_PRODUCERS')
    if value not in (None, '0', '1'):
        raise ValueError('MNM_OTHER_PIXEL_PRODUCERS must be 0 or 1')
    result = dict(environment)
    reject_retired_dc_hook(result)
    if value == '1':
        for flag in ('MNM_PANEL_TAKEOVER', 'MNM_PIXEL_TAKEOVER', 'MNM_JPEG_TAKEOVER',
                     'MNM_MINIMAP_TAKEOVER', *(v[0] for v in FEATURES.values())):
            result.setdefault(flag, '1')
    return result


def reject_retired_dc_hook(environment, metadata=None):
    if environment.get('MNM_DC_COPY_TAKEOVER') not in (None, '0') or (metadata or {}).get('native_dc_copy_takeover') not in (None, False):
        raise ValueError('Retired DC-copy hook targeted ClientToScreen; prepare a new installation without it')
    environment.pop('MNM_DC_COPY_TAKEOVER', None)


def enabled(environment, feature):
    flag = FEATURES[feature][0]
    value = environment.get(flag)
    if value not in (None, '0', '1'):
        raise ValueError(flag+' must be 0 or 1')
    return value == '1'


def stage(root, original, patched, feature, active):
    if type(active) is not bool:
        raise ValueError('Invalid native pixel producer policy')
    if not active:
        return patched, {}
    flag, name, anchor, builder, section, sites = FEATURES[feature]
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported pixel producer source executable')
    exporter = load(feature+'_offsets', 'tools/export-menu-support.py')
    for address, expected in sites:
        code = bytes.fromhex(expected)
        at = exporter.image_offset(original, address, len(code))
        if original[at:at+len(code)] != code:
            raise ValueError('Unsupported '+feature+' pixel splice')
    target = root/'game'/name
    if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
        raise ValueError('Pixel producer target escapes disposable game')
    dll = build_adapter(builder, lambda: load(feature+'_build', builder).build())
    updated = load(feature+'_import', 'tools/prepare-shadow-experiment.py').add_import(patched, name, anchor, section)
    shutil.copy2(dll, target)
    shutil.copy2(dll.parent/'manifest.json', root/(feature+'-bridge-build.json'))
    return updated, {'native_'+feature+'_takeover': True,
                     feature+'_dll_sha256': hashlib.sha256(target.read_bytes()).hexdigest()}


def authorize(root, metadata, environment, feature):
    flag, name, *_ = FEATURES[feature]
    environment.pop(flag, None)
    active = metadata.get('native_'+feature+'_takeover', False)
    if type(active) is not bool:
        raise ValueError('Invalid staged native pixel producer policy')
    if not active:
        return
    if (metadata.get('native_draw') is not True or metadata.get('native_render') is not True
            or metadata.get('origin') != 'menu_action_bridge'):
        raise ValueError('Native pixel producers require owned menu/drawing staging')
    game = root/'game'
    target = game/name
    if (game.is_symlink() or not game.resolve().is_relative_to(root.resolve()) or target.is_symlink()
            or not target.resolve().is_relative_to(game.resolve())
            or hashlib.sha256(target.read_bytes()).hexdigest() != metadata.get(feature+'_dll_sha256')):
        raise ValueError('Native pixel producer DLL hash/path mismatch')
    environment[flag] = '1'
