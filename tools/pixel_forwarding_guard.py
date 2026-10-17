"""Select/authorize the six-family no-forward guard; no whole-renderer claim."""
from borrowed_pixel_backend_launch import FEATURES

FLAG = 'MNM_NATIVE_PIXEL_NO_FORWARD'
FAMILIES = ('MNM_PANEL_TAKEOVER', 'MNM_PIXEL_TAKEOVER', 'MNM_JPEG_TAKEOVER',
            'MNM_MINIMAP_TAKEOVER', *(v[0] for v in FEATURES.values()))


def selection(environment, *, native_draw, native_render, actions):
    value = environment.get(FLAG)
    if value not in (None, '0', '1'):
        raise ValueError(FLAG+' must be 0 or 1')
    active = value == '1'
    if active and (not all((native_draw,native_render,actions)) or
                   any(environment.get(flag) != '1' for flag in FAMILIES)):
        raise ValueError('No-forward guard requires owned menu/drawing and all six pixel families')
    return active


def authorize(root, metadata, environment):
    environment.pop(FLAG, None)
    active = metadata.get('native_pixel_no_forward', False)
    if type(active) is not bool:
        raise ValueError('Invalid staged no-forward guard')
    if not active:
        return
    if metadata.get('origin') != 'menu_action_bridge' or not selection(
            dict(environment, **{FLAG:'1'}), native_draw=metadata.get('native_draw') is True,
            native_render=metadata.get('native_render') is True, actions=True):
        raise ValueError('No-forward guard staging mismatch')
    if environment.get('MNM_NATIVE_PIXEL_COUNTERS') != '1':
        raise ValueError('No-forward guard requires fresh route counters')
    refusal = root/'game/native-pixel-forward-refusal.bin'
    if refusal.exists() or refusal.is_symlink():
        raise ValueError('Stale or redirected pixel-forward refusal')
    environment[FLAG] = '1'


def decode_refusal(raw):
    import struct
    if len(raw) != 128:
        raise ValueError('Pixel guard refusal size')
    r = struct.unpack('<32I', raw)
    if raw[:8] != b'MNMPGRD1' or r[2:4] != (1,128) or not 1 <= r[4] <= 7 or r[8] != int(r[4] != 1) or any(r[21:]):
        raise ValueError('Pixel guard refusal header/reason/reserved')
    expected = [(1<<n)-1 for n in (3,8,1,7,1,23)]
    if list(r[15:21]) != expected or any(a & ~b for a,b in zip(r[9:15],expected)):
        raise ValueError('Pixel guard installation masks')
    return dict(schema=1,reason=r[4],family=r[5],route=r[6],event=r[7],enabled=bool(r[8]),
                installed_masks=list(r[9:15]),expected_masks=expected,
                complete_rendering_replacement=False)
