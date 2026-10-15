"""Public map-to-recorder cache points; no bytes or ownership inference."""
import struct
from drawing_storage_point import point_header
from drawing_api_session import sha


def decode(data, resource_observation):
    if len(data) < 160 or (len(data)-64) % 96 or len(data) > 64+131072*96:
        raise ValueError('Map storage envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMWSP01' or h[2:6] != (1, 96, 0x40209ca7, 131072) or any(h[6:]):
        raise ValueError('Map storage header')
    maps = {c['call']: c for c in resource_observation['calls']
            if c['method'] == 2 and c['result'] < 0x80000000}
    rows = list(struct.iter_unpack('<24I', data[64:]))
    points = []; seen = set()
    for ordinal, r in enumerate(rows[:-1], 1):
        c = maps.get(r[2]); m = r[8:24]
        if r[0] != ordinal or r[1] != 2 or c is None or r[2] in seen or r[7] not in (0, 1):
            raise ValueError('Map storage identity')
        if (r[3], r[4], r[5]) != (c['object'], c['map_interval'], c['thread']):
            raise ValueError('Map storage interval/thread mismatch')
        seen.add(r[2])
        if not r[7]:
            if any(m):
                raise ValueError('Failed export has metadata')
        else:
            flags = m[2]
            point_header(m,r[5])
            if flags & 16:
                if flags & 4 or not flags & 2 or m[5] or not m[6] or not 0 < m[7] <= 2048 or not 0 < m[8] <= 2048 or not m[7] <= m[9] <= 4096:
                    raise ValueError('Invalid live storage association')
                if c['row_pitch'] != m[9]*2 or c['slice_pitch'] < c['row_pitch']*m[8]:
                    raise ValueError('Storage descriptor extent mismatch')
            elif any(m[6:12]):
                raise ValueError('Unknown storage has canvas metadata')
        points.append(dict(map_call=r[2], resource=r[3], map_interval=r[4], thread=r[5],
                           tick=r[6], export_status=r[7], metadata=list(m),
                           canvas=m[6], texture=c['texture'], texture_generation=c['texture_generation'],
                           matched=bool(r[7] and m[2] & 16), source_ownership_proved=False))
    t = rows[-1]
    if t[0] != len(rows) or t[1] != 4 or t[2] not in (1, 2) or t[3] != len(points) or t[4] or not t[5] or any(t[7:]) or seen != set(maps):
        raise ValueError('Map storage closure/coverage')
    return dict(schema=1, success=True, journal_sha256=sha(data), points=points,
                matched=sum(p['matched'] for p in points), unknown=sum(not p['matched'] for p in points),
                source_ownership_proved=False, source_upload_identity_proved=False,
                private_layout_read=False, complete_presented_journey=False)
