"""Fresh native World channel envelopes, shared by isolated launch tools."""
import secrets
import struct

HEADER = 128
INPUT = 32 * 1024 * 1024
ORACLE = 8 * 1024 * 1024
BUILD = 0x40209ca7


def create(path, verify=False, history_frames=0):
    if not 0 <= history_frames <= 16:
        raise ValueError('History target must be 1..16, or zero for v1')
    slots = history_frames or 2
    size = HEADER + slots * ((32 if history_frames else 16) + INPUT + ORACLE)
    header = bytearray(HEADER)
    header[:8] = b'MNMWCH02' if history_frames else b'MNMWCH01'
    struct.pack_into('<15I', header, 8, 2 if history_frames else 1, size,
                     secrets.randbelow(0xffffffff) + 1, 0, 0, 0, 0, 0, 0,
                     (2 if history_frames else 0) | int(verify), 0,
                     slots, INPUT, ORACLE, BUILD)
    if history_frames:
        struct.pack_into('<I', header, 68, history_frames)
    with path.open('xb') as channel:
        channel.write(header)
        channel.truncate(size)


def validate_fresh(path):
    with path.open('rb') as channel:
        header = channel.read(HEADER)
    if len(header) != HEADER:
        raise ValueError('Truncated World channel header')
    words = struct.unpack('<32I', header)
    history = words[2] == 2
    slots = words[13]
    size = HEADER + slots * ((32 if history else 16) + INPUT + ORACLE)
    if (header[:8] != (b'MNMWCH02' if history else b'MNMWCH01') or
            words[2] != (2 if history else 1) or
            not words[4] or words[3] != size or path.stat().st_size != size or
            (not 1 <= slots <= 16 if history else slots != 2) or
            words[11] not in ((2, 3) if history else (0, 1)) or
            words[14:17] != (INPUT, ORACLE, BUILD) or
            any(words[5:11]) or words[12] or
            (history and words[17] != slots) or
            any(words[18 if history else 17:])):
        raise ValueError('Stale or invalid World channel identity')
    # Fresh slots must be unowned; their payload pages remain sparse.
    with path.open('rb') as channel:
        for index in range(slots):
            channel.seek(HEADER + index * ((32 if history else 16) + INPUT + ORACLE))
            if channel.read(4) != bytes(4):
                raise ValueError('Stale World channel ownership')
    return history
