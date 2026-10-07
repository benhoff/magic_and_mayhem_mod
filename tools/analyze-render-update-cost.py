#!/usr/bin/env python3
"""Measure UPDATE pixel redundancy by replaying a bounded owned command archive."""
import argparse
import array
import collections
import hashlib
import json
from pathlib import Path
import struct
import sys
ROOT = Path(__file__).resolve().parents[1]
LIMIT = 64 * 1024 * 1024


def pixels(raw, bits):
    if bits == 24:
        return [int.from_bytes(raw[i:i+3], 'little') for i in range(0, len(raw), 3)]
    out = array.array({8: 'B', 16: 'H', 32: 'I'}[bits], raw)
    if sys.byteorder != 'little':
        out.byteswap()
    return out


def analyze(path):
    assert 16 <= path.stat().st_size <= LIMIT, 'Archive byte budget'
    raw = path.read_bytes()
    assert raw[:8] in [b'MNMCMD01', b'MNMCMD02']
    assert struct.unpack_from('<II', raw, 8) == (int(chr(raw[7])), 16)
    at, sequence, surfaces = 16, 0, {}
    counts, sizes, updates = collections.Counter(), collections.Counter(), []
    retained = 0
    while at < len(raw):
        assert sequence < 4096 and at + 12 <= len(raw)
        op, seq, n = struct.unpack_from('<3I', raw, at)
        assert seq == sequence + 1 and at + 12 + n <= len(raw)
        sequence = seq
        payload = raw[at+12:at+12+n]
        at += 12 + n
        counts[op] += 1
        sizes[op] += 12 + n
        if op == 1:
            sid, w, h, bits, *_ = struct.unpack_from('<7I', payload)
            assert sid not in surfaces and len(surfaces) < 32
            assert 0 < w <= 2048 and 0 < h <= 2048 and bits in [8, 16, 24, 32]
            size = w*h*(bits//8)
            assert len(payload) == 28+size and retained+size <= LIMIT
            surfaces[sid] = (w, h, bits, pixels(payload[28:], bits))
            retained += size
        elif op == 2:
            sid, x, y, w, h = struct.unpack_from('<5I', payload)
            sw, sh, bits, native = surfaces[sid]
            assert w and h and x+w <= sw and y+h <= sh
            assert len(payload) == 20+w*h*(bits//8)
            incoming = pixels(payload[20:], bits)
            changed = 0
            for row in range(h):
                start = (y+row)*sw+x
                old, new = native[start:start+w], incoming[row*w:(row+1)*w]
                changed += sum(a != b for a, b in zip(old, new))
                native[start:start+w] = new
            updates.append(dict(sequence=seq, width=w, height=h,
                                pixel_bytes=w*h*(bits//8), changed_pixel_storage_bytes=changed*(bits//8)))
        elif op == 3:
            source, target, sx, sy, right, bottom, dx, dy, keyed, key = struct.unpack('<10I', payload)
            sw, sh, sb, src = surfaces[source]
            dw, dh, db, dst = surfaces[target]
            w, h = right-sx, bottom-sy
            assert sb == db and w > 0 and h > 0 and right <= sw and bottom <= sh and dx+w <= dw and dy+h <= dh
            # Snapshot source rows before updating a possibly aliased destination.
            rows = [src[(sy+y)*sw+sx:(sy+y)*sw+right] for y in range(h)]
            for y, row in enumerate(rows):
                start = (dy+y)*dw+dx
                if keyed:
                    for x, value in enumerate(row):
                        if value != key:
                            dst[start+x] = value
                else:
                    dst[start:start+w] = row
        elif op == 11:
            a, b = struct.unpack('<2I', payload)
            assert a != b and surfaces[a][:3] == surfaces[b][:3]
            surfaces[a], surfaces[b] = surfaces[b], surfaces[a]
        elif op == 7:
            sid, = struct.unpack('<I', payload)
            w, h, bits, _ = surfaces.pop(sid)
            retained -= w*h*(bits//8)
        elif op in [4, 5, 6, 8, 9, 12, 13, 14, 15]:
            pass  # Palette/control records do not change native surface bytes.
        else:
            raise ValueError('Unsupported archive operation '+str(op))
    return dict(archive=str(path), archive_sha256=hashlib.sha256(raw).hexdigest(),
                archive_bytes=len(raw), records=sequence, opcode_counts=dict(counts),
                opcode_wire_bytes=dict(sizes), update_pixel_bytes=sum(u['pixel_bytes'] for u in updates),
                changed_pixel_storage_bytes=sum(u['changed_pixel_storage_bytes'] for u in updates), updates=updates)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    assert not args.output.exists(), 'Preserve prior measurement reports'
    report = dict(schema=1, success=True, sources={str(Path(__file__).relative_to(ROOT)):
                  hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},
                  scope='Bounded owned wire replay and UPDATE redundancy measurement. Changed pixels count all storage bytes including unused bits. Archive may end in diagnostic GAP; this is not independent original pixel or driver validation.',
                  **analyze(args.archive))
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'updates'}, indent=2))


if __name__ == '__main__':
    main()
