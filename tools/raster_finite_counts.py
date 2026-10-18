"""Decode normal-exit CPU displacement observations without inferring traffic."""
from pathlib import Path
import struct


def decode(path, native_by_route):
    data = Path(path).read_bytes()
    if len(data) != 208 or data[:8] != b'MNMFDSP1':
        raise ValueError('Invalid finite displacement extent/magic')
    words = struct.unpack('<50I', data[8:])
    if words[:4] != (1, 208, 23, 0):
        raise ValueError('Unsupported or overflowed finite displacement counters')
    rows = []
    for i in range(23):
        calls, pixels = words[4+i*2:6+i*2]
        native = native_by_route['splice'+str(i)]
        if bool(calls) != bool(pixels) or pixels < calls or calls > native:
            raise ValueError('Finite counters exceed independently recorded native traffic')
        rows.append(dict(route='raster/splice'+str(i), calls=calls, retained_pixels=pixels))
    return dict(routes=rows, calls=sum(r['calls'] for r in rows),
                retained_pixels=sum(r['retained_pixels'] for r in rows),
                original_outside_allocation_equivalence=False)
