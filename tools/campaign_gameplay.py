"""Read-only bounded gameplay observation protocol; no process memory writes."""
import struct

HEADER = b'MNMGP001' + struct.pack('<II', 1, 64)


def signed(value):
    return value if value < 0x80000000 else value - 0x100000000


def read_rows(path, complete=False):
    data = path.read_bytes()
    if data[:16] != HEADER:
        raise ValueError('Invalid gameplay observation header')
    if complete and (len(data)-16) % 64:
        raise ValueError('Truncated gameplay observation')
    end = 16 + (len(data)-16)//64*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    if len(rows) >= 65536 or [r[0] for r in rows] != list(range(1, len(rows)+1)) or any(r[1] not in (1, 2) for r in rows):
        raise ValueError('Invalid or exhausted gameplay observation sequence')
    return rows


def creatures(rows):
    latest = {}
    for r in rows:
        if r[1] == 1:
            latest[r[4]] = dict(sequence=r[0], slot=r[4], type=r[5], owner=r[6], active=r[7],
                health=signed(r[8]), x=signed(r[9]), y=signed(r[10]), z=signed(r[11]),
                behavior=r[12], action=r[13], target=r[14], mana=signed(r[15]))
    return latest


def damage_rows(rows, owner, source_slots, after):
    return [r for r in rows if r[1] == 2 and r[0] > after and r[4] in source_slots
            and r[5] == 14 and r[6] == owner and r[10] not in (owner, 0xffffffff)
            and r[13] and signed(r[11]) > 0 and signed(r[12]) < signed(r[11])]
