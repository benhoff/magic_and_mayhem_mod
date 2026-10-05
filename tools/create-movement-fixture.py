#!/usr/bin/env python3
"""Generate a synthetic frozen routing map; no original artifacts are consumed."""
import argparse
from pathlib import Path
import struct


def fixture(blocked=False, width=12, height=12, layers=3):
    rows = struct.pack(f'<{height}i', *[i * width for i in range(height)])
    levels = struct.pack(f'<{layers}i', *[i * width * height for i in range(layers)])
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


def terrain_fixture(profile='terrace'):
    """Controlled terrain inputs, not a projection of an installed MAP world."""
    raw = fixture(layers=5)
    header = list(struct.unpack_from('<14I', raw, 8))
    sizes = struct.unpack_from('<7I', raw, 64)
    blocks = []
    at = 92
    for size in sizes:
        blocks.append(bytearray(raw[at:at+size]));at += size
    cells, terrain, scalar = blocks[2], blocks[3], blocks[5]
    terrain.extend(bytes(2 * 0x164))
    for definition, elevation in [(2, 4), (3, 8)]:
        struct.pack_into('<i', terrain, definition * 0x164 + 0x94, elevation)
        terrain[definition * 0x164 + 0xb0] = 8
    for y in range(12):
        for x in range(12):
            if profile == 'terrace':
                definition = 2 if x == 2 else (3 if x in (3, 4) else 0)
                struct.pack_into('<H', cells, (x+12*(y+12))*12, definition)
            elif profile == 'slope' and 3 <= x <= 6:
                struct.pack_into('<H', cells, (x+12*(y+12))*12, 1)
    if profile == 'vertical':
        header[5] = 3  # controlled boundary band permits category-four links
        struct.pack_into('<i', scalar, 0x44, 2)
        struct.pack_into('<i', scalar, 0x3c, 1)
    elif profile not in ('terrace', 'slope'):
        raise ValueError('Unknown terrain fixture profile')
    return b'MNMWLD01'+struct.pack('<21I', *header, *map(len, blocks))+b''.join(blocks)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--blocked', action='store_true')
    parser.add_argument('--terrain-profile', choices=['terrace', 'slope', 'vertical'])
    args = parser.parse_args()
    if args.blocked and args.terrain_profile:
        parser.error('--blocked excludes --terrain-profile')
    with args.output.open('xb') as stream:
        stream.write(terrain_fixture(args.terrain_profile) if args.terrain_profile else fixture(args.blocked))
    print('Created synthetic frozen movement map:', args.output)
