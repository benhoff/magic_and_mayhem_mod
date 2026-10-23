"""Decode bounded, post-retirement accepted mode3 CPU-store observations."""
from pathlib import Path
import struct

HEADER_BYTES = 40
ROW_BYTES = 40
CAPACITY = 64
LIMIT = 10_000_000

def decode(raw):
    if not HEADER_BYTES <= len(raw) <= HEADER_BYTES + CAPACITY * ROW_BYTES:
        raise ValueError('CPU store profile size')
    h = struct.unpack_from('<10I', raw)
    if raw[:8] != b'MNMSTP01' or h[2:6] != (1, HEADER_BYTES, ROW_BYTES, CAPACITY) or h[9]:
        raise ValueError('CPU store profile header')
    used, flags, total = h[6:9]
    if used > CAPACITY or flags & ~15 or total > LIMIT or len(raw) != HEADER_BYTES + used * ROW_BYTES:
        raise ValueError('CPU store profile extent/flags')
    rows, sites = [], set()
    for offset in range(HEADER_BYTES, len(raw), ROW_BYTES):
        pc, count, byte_count, width, length, *tail = struct.unpack_from('<10I', raw, offset)
        code = struct.pack('<4I', *tail[:4])
        if (not pc or pc in sites or not count or count > total or width not in (1, 2, 4)
                or byte_count != count * width or not 1 <= length <= 15
                or any(code[length:]) or tail[4]):
            raise ValueError('CPU store profile site')
        sites.add(pc)
        rows.append(dict(address=pc, stores=count, bytes=byte_count, width=width,
                         instruction=code[:length].hex()))
    if sum(r['stores'] for r in rows) != total:
        raise ValueError('CPU store profile count mismatch')
    return dict(version=1, complete=flags == 0, flags=flags, stores=total,
                bytes=sum(r['bytes'] for r in rows), sites=sorted(rows, key=lambda r: (-r['stores'], r['address'])),
                scope='Successful protected mode3 exception-handler scalar stores only; direct native store-service writes are separate; no reads, unsupported instructions, timer changes or original pixel equivalence. Nonzero flags retain an incomplete observation.')

def decode_file(path):
    with Path(path).open('rb') as stream:
        raw = stream.read(HEADER_BYTES + CAPACITY * ROW_BYTES + 1)
    return decode(raw)
