#!/usr/bin/env python3
"""Replay a bounded native-pixel DirectDraw blit capture without running the game."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import tempfile

REPO = Path(__file__).resolve().parent.parent
MAX_CAPTURE = 128 + 256*256*4 + 2*2048*2048*4 + 2048
OPERATIONS = {1: 'Blt', 2: 'BltFast', 3: 'Flip', 4: 'Lock', 5: 'Unlock', 6: 'CreateSurface'}


def decode(data):
    if len(data) < 128 or len(data) > MAX_CAPTURE or data[:8] != b'MNMBLT01':
        raise ValueError('Invalid or truncated MNMBLT01 capture')
    h = struct.unpack_from('<32I', data)
    if h[2] != 1 or h[3] not in (1, 2) or h[31] or h[9] & 0x80000000:
        raise ValueError('Unsupported version, operation, reserved field, or failed HRESULT')
    fast = h[3] == 2
    if h[5] & ~(0x31 if fast else 0x09008000) or h[6] != bool(h[5] & (1 if fast else 0x8000)):
        raise ValueError('Unsupported flags or inconsistent key mode')
    sw, sh, dw, dh, bits = h[18:23]
    if not (0 < sw <= 256 and 0 < sh <= 256 and 0 < dw <= 2048 and 0 < dh <= 2048):
        raise ValueError('Dimensions exceed the capture bounds')
    if bits not in (8, 16, 24, 32):
        raise ValueError('Unsupported native pixel format')
    if h[6] and not 0 <= h[7] == h[8] < (1 << bits):
        raise ValueError('Unsupported source color key; only a single exact key is replayed')
    masks = h[23:26]
    if bits != 8:
        for mask in masks:
            normalized = mask // (mask & -mask) if mask else 0
            if not mask or mask >= (1 << bits) or normalized > 255 or normalized & (normalized + 1):
                raise ValueError('Unsupported RGB mask')
        if masks[0] & masks[1] or masks[0] & masks[2] or masks[1] & masks[2]:
            raise ValueError('Overlapping RGB masks')
    sr = struct.unpack_from('<4i', data, 40)
    dr = struct.unpack_from('<4i', data, 56)
    for rect, w, height in ((sr, sw, sh), (dr, dw, dh)):
        l, t, r, b = rect
        if not (0 <= l < r <= w and 0 <= t < b <= height):
            raise ValueError('Out-of-bounds rectangle')
    if sr[2]-sr[0] != dr[2]-dr[0] or sr[3]-sr[1] != dr[3]-dr[1]:
        raise ValueError('Stretched blits are not supported')
    stride = bits // 8
    src_bytes, dst_bytes, palette_bytes = h[26:29]
    if (src_bytes, dst_bytes, palette_bytes) != (sw*sh*stride, dw*dh*stride, 1024 if bits == 8 else 0):
        raise ValueError('Invalid payload lengths')
    if len(data) != 128 + src_bytes + 2*dst_bytes + 2*palette_bytes:
        raise ValueError('Truncated capture or trailing bytes')
    start = 128
    payloads = []
    for size in (src_bytes, dst_bytes, dst_bytes, palette_bytes, palette_bytes):
        payloads.append(data[start:start+size]); start += size
    return {'operation': OPERATIONS[h[3]], 'caller_return_va': f'0x{h[4]:08x}',
            'flags': h[5], 'key': (h[7], h[8]) if h[6] else None,
            'hresult': h[9], 'src_rect': sr, 'dst_rect': dr,
            'src_size': (sw, sh), 'dst_size': (dw, dh), 'bits': bits, 'masks': masks,
            'source_token': h[29], 'destination_token': h[30],
            **dict(zip(('source', 'before', 'after', 'source_palette', 'destination_palette'), payloads))}


def replay(capture):
    """Reference copy in native pixel space with an optional single exact key."""
    result = bytearray(capture['before'])
    pixel_size = capture['bits'] // 8
    sw, _ = capture['src_size']; dw, _ = capture['dst_size']
    sl, st, sr, sb = capture['src_rect']; dl, dt, _, _ = capture['dst_rect']
    source, key = capture['source'], capture['key']
    for y in range(sb-st):
        for x in range(sr-sl):
            offset = ((st+y)*sw+sl+x)*pixel_size
            pixel = source[offset:offset+pixel_size]
            if key is not None and int.from_bytes(pixel, 'little') == key[0]:
                continue
            dest = ((dt+y)*dw+dl+x)*pixel_size
            result[dest:dest+pixel_size] = pixel
    return bytes(result)


def compare(capture, actual):
    expected, size = capture['after'], capture['bits']//8
    if len(actual) != len(expected):
        raise ValueError('Replay changed surface dimensions')
    count, first = 0, None
    for offset in range(0, len(expected), size):
        if actual[offset:offset+size] != expected[offset:offset+size]:
            count += 1
            if first is None:
                pixel = offset//size
                first = {'x': pixel % capture['dst_size'][0], 'y': pixel // capture['dst_size'][0],
                         'captured': expected[offset:offset+size].hex(), 'replayed': actual[offset:offset+size].hex()}
    return {'matching': count == 0, 'mismatching_pixels': count, 'first_mismatch': first}


def ppm(capture, pixels, source=False):
    width, height = capture['src_size' if source else 'dst_size']
    palette = capture['source_palette' if source else 'destination_palette']
    size = capture['bits']//8
    channels = [(mask & -mask, mask // (mask & -mask)) for mask in capture['masks']] if size != 1 else []
    out = bytearray(f'P6\n{width} {height}\n255\n'.encode())
    for offset in range(0, len(pixels), size):
        value = int.from_bytes(pixels[offset:offset+size], 'little')
        if size == 1:
            out.extend(palette[value*4:value*4+3])
        else:
            out.extend(((value//low)&maximum)*255//maximum for low, maximum in channels)
    return out


def summarize_events(data):
    if len(data) < 16 or data[:8] != b'MNMDRW01' or struct.unpack_from('<2I', data, 8) != (1, 64):
        raise ValueError('Invalid event stream header')
    if (len(data)-16) % 64 or len(data) > 16 + 2048*64:
        raise ValueError('Truncated or oversized event stream')
    counts, callers = Counter(), Counter()
    for index, offset in enumerate(range(16, len(data), 64), 1):
        row = struct.unpack_from('<16I', data, offset)
        if row[0] != index or row[1] not in OPERATIONS:
            raise ValueError('Invalid event sequence or operation')
        name = OPERATIONS[row[1]]
        counts[name] += 1; callers[(name, f'0x{row[2]:08x}')] += 1
    return {'events': sum(counts.values()), 'limit_reached': sum(counts.values()) == 2048,
            'counts': dict(counts),
            'callers': [{'operation': op, 'return_va': va, 'count': count} for (op, va), count in callers.most_common()],
            'scope': 'First 2048 observed calls; concurrent/reentrant events may be skipped; observer locks excluded'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture', type=Path, help='blit-0001.bin or its draw-capture directory')
    args = parser.parse_args()
    path = args.capture / 'blit-0001.bin' if args.capture.is_dir() else args.capture
    parent = REPO/'working/experiments/render-replay'; parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    report = {'capture': str(path.resolve()), 'scope': 'Native opaque/source-key copy reference, not a complete renderer'}
    status = 2
    try:
        events = path.parent/'events.bin'
        if events.exists():
            if events.stat().st_size > 16 + 2048*64:
                raise ValueError('Oversized event stream')
            report['inventory'] = summarize_events(events.read_bytes())
        if path.stat().st_size > MAX_CAPTURE:
            raise ValueError('Oversized capture')
        data = path.read_bytes(); capture = decode(data); actual = replay(capture)
        report.update({key: value for key, value in capture.items() if not isinstance(value, bytes)})
        report['capture_sha256'] = hashlib.sha256(data).hexdigest()
        report['comparison'] = compare(capture, actual)
        for name, pixels, source in [('source', capture['source'], True), ('before', capture['before'], False),
                                     ('captured', capture['after'], False), ('replayed', actual, False)]:
            (output/(name+'.ppm')).write_bytes(ppm(capture, pixels, source))
        status = 0 if report['comparison']['matching'] else 1
    except (ValueError, OSError) as error:
        report['inconclusive'] = str(error)
    (output/'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Report: {output / "report.json"}')
    print('Inconclusive' if status == 2 else 'Pixels match' if status == 0 else 'Pixel mismatch')
    return status


if __name__ == '__main__':
    raise SystemExit(main())
