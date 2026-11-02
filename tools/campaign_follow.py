"""Read the bounded original +906 follow-reference observation, without writes."""
import struct
from campaign_gameplay import signed

HEADER = b'MNMFL001' + struct.pack('<II', 1, 64)


def read_rows(path, complete=False):
    data = path.read_bytes()
    if data[:16] != HEADER or complete and (len(data)-16) % 64:
        raise ValueError('Invalid follow observation header/length')
    end = 16+(len(data)-16)//64*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    if len(rows) > 4096 or any(r[0] != i+1 or not r[2] or r[4] != 14 or r[5] != 0 or not r[6]
                               for i, r in enumerate(rows)):
        raise ValueError('Invalid bounded follow observation sequence/source')
    return rows


def followed_wizard(rows, source, wizard, after=0):
    """Only a fresh live same-identity source/reference pair admits the driver."""
    candidates = [r for r in rows if r[3] == source['slot']]
    if not candidates:
        return None
    r = candidates[-1]
    if (r[0] <= after or r[4] != source['type'] or r[5] != source['owner'] or not r[6] or not source['active']
            or source['health'] <= 0 or signed(r[12]) <= 0 or not wizard['active'] or wizard['health'] <= 0
            or r[7] != wizard['slot'] or r[8] != wizard['type'] or r[9] != wizard['owner']
            or not r[10] or signed(r[11]) <= 0):
        return None
    return dict(sequence=r[0], tick=r[1], thread=r[2], source=r[3], reference=r[7],
                source_health=signed(r[12]), target_health=signed(r[11]),
                position=[signed(v) for v in r[13:16]])
