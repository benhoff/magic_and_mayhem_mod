#!/usr/bin/env python3
"""Generate a synthetic frozen routing map; no original artifacts are consumed."""
import argparse
from pathlib import Path
import struct


def fixture(blocked=False, width=12, height=12):
    layers = 3
    rows = struct.pack(f'<{height}i', *[i * width for i in range(height)])
    levels = struct.pack('<3i', *[i * width * height for i in range(layers)])
    cells = bytearray(width * height * layers * 12)
    for index in range(width * height * layers):
        struct.pack_into('<H', cells, index * 12 + 4, 0xffff)
        if index < width * height:
            struct.pack_into('<H', cells, index * 12, 1)
    if blocked:
        index = 5 + width * (1 + height)
        struct.pack_into('<H', cells, index * 12 + 4, 17)
        # Base validity rejects bit 0x4000 even when the search's selected
        # destination special-case bypasses the occupancy check.
        struct.pack_into('<H', cells, index * 12 + 10, 0x4000)
    terrain = bytearray(2 * 0x164)
    struct.pack_into('<i', terrain, 0x164 + 0x94, 16)
    terrain[0x164 + 0xb0] = 8
    creature = bytearray(0xd07)
    struct.pack_into('<3i', creature, 8, 1, 1, 1)
    scalar = bytearray(0x198)
    struct.pack_into('<3i', scalar, 8, 1, 1, 500)
    for index in range(48):
        struct.pack_into('<I', scalar, 0xd8 + index * 4, 40 if (index // 12) % 2 else 60)
    generator = bytearray(0x5c9)
    struct.pack_into('<i', generator, 0x589, 720)
    blocks = [rows, levels, cells, terrain, creature, scalar, generator]
    header = [0x1200000, width, height, layers, width * height, 1, 300, 0,
              5, 1, 1, 720, struct.unpack('<I', struct.pack('<f', 0.969))[0], 0x1000000]
    return b'MNMWLD01' + struct.pack('<21I', *header, *map(len, blocks)) + b''.join(blocks)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--blocked', action='store_true')
    args = parser.parse_args()
    with args.output.open('xb') as stream:
        stream.write(fixture(args.blocked))
    print('Created synthetic 12x12x3 frozen map:', args.output)
