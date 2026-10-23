"""Strict terminal accounting for the supplied-runtime capability probe."""
import struct


def decode(data):
    if not data or len(data) % 24:
        raise ValueError('Incomplete legacy admission rows')
    rows = list(struct.iter_unpack('<6I', data))
    terminal = rows[-1]
    if terminal[0] != 99 or terminal[1:4] != (0, 0, 0) or terminal[5] or terminal[4] not in range(1, 6):
        raise ValueError('Unknown legacy admission terminal')
    if any(r[0] == 99 for r in rows[:-1]):
        raise ValueError('Repeated legacy admission terminal')
    stage = terminal[4]
    api = [r for r in rows[:-1] if 90 <= r[0] <= 94]
    if [r[0] for r in api] != list(range(90, 91 if stage == 1 else 92 if stage == 2 else 93 if stage == 3 else 95)):
        raise ValueError('Missing/reordered native API outcome')
    for r in api:
        if r[1:3] != (0, 0) or (r[0] != 90 and r[4] not in (0, 1)) or r[5] != (8 if r[0] == 93 else 0):
            raise ValueError('Malformed native API result')
    if bool(api[0][4]) != (stage > 1):
        raise ValueError('Loader result disagrees with terminal')
    if stage > 1 and bool(api[1][4]) != (stage > 2):
        raise ValueError('Export result disagrees with terminal')
    if stage > 2 and (not api[2][3] and bool(api[2][4])) != (stage > 3):
        raise ValueError('Create result disagrees with terminal')
    if stage > 3 and (not api[4][3] and bool(api[4][4])) != (stage == 5):
        raise ValueError('Surface result disagrees with terminal')
    labels = {1: 'loader_refused', 2: 'export_unavailable', 3: 'create_refused',
              4: 'surface_unavailable', 5: 'offscreen_surface_available'}
    return dict(rows=rows, admission_stage=stage, outcome=labels[stage],
                legacy_surface_available=stage == 5,
                loader_error=api[0][3] if stage == 1 else None)
