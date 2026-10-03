#!/usr/bin/env python3
"""Frozen route replay inputs, captured through the existing WineDbg controller."""
import hashlib
import json
from pathlib import Path
import struct
import time

MAGIC = b"MNMWLD01"
HEADER = struct.Struct("<8s14I7I")
LIMIT = 64 * 1024 * 1024
EXECUTABLE_HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"


def read_bytes(debugger, address, length, deadline=None):
    if not address or address + length > 0x100000000:
        raise ValueError("Invalid x86 snapshot memory span")
    def check_deadline():
        if deadline is not None and time.monotonic() >= deadline:
            raise TimeoutError("Capture deadline reached while reading world inputs")
    check_deadline()
    data = bytearray()
    # Bounded commands keep WineDbg's output parser and PTY manageable.
    words, tail = divmod(length, 4)
    for offset in range(0, words, 256):
        check_deadline()
        count = min(256, words - offset)
        data.extend(struct.pack(f"<{count}I", *debugger.memory(address + offset * 4, count)))
    if tail:
        check_deadline()
        data.extend(debugger.memory(address + words * 4, tail, 1))
    return bytes(data)


def capture_world(debugger, image_base, search, output, deadline=None, allow_continuation=False):
    live = lambda va: image_base + va - 0x400000
    read = lambda va, size: read_bytes(debugger, va, size, deadline)
    word = lambda va: struct.unpack("<I", read(va, 4))[0]
    signed = lambda n: n - 0x100000000 if n & 0x80000000 else n
    flag = search.get("flag_before", 1)
    if not isinstance(flag, int) or not 0 <= flag <= 255:
        raise ValueError("Invalid search initialization byte")
    if not flag and not allow_continuation:
        raise ValueError("World replay requires a fresh search; continuation containers are not captured")
    obj = search["object"]
    x, y = signed(word(live(0x6c5494))), signed(word(live(0x6c5498)))
    z = signed(word(live(0x5e1780)))
    if not (1 <= x <= 1024 and 1 <= y <= 1024 and 1 <= z <= 32):
        raise ValueError("Snapshot dimensions exceed bounded engine tables")
    plane = signed(word(live(0x6c54a0)))
    rows = read(live(0x6cb942), y * 4)
    layers = read(live(0x6cb8c2), z * 4)
    row_values = struct.unpack(f"<{y}i", rows)
    layer_values = struct.unpack(f"<{z}i", layers)
    if min(row_values) < 0 or min(layer_values) < 0 or plane <= 0:
        raise ValueError("Negative map offsets or invalid plane stride")
    count = max(row_values) + max(layer_values) + x
    if count * 12 > LIMIT // 2:
        raise ValueError("Snapshot cell span exceeds bounded capture limit")
    cell_base = word(live(0x6c54dc))
    cells = read(cell_base, count * 12)
    max_terrain = max(struct.unpack_from("<H", cells, i)[0] for i in range(0, len(cells), 12))
    terrain_base = word(live(0x65660c))
    terrain = read(terrain_base, (max_terrain + 1) * 0x164) if terrain_base else bytes(0x164)
    if not terrain_base and max_terrain:
        raise ValueError("Nonzero terrain index with null table")
    object_bytes = read(obj, 0xd07)
    type_index = struct.unpack_from("<i", object_bytes, 0xa8)[0]
    if not 0 <= type_index <= 1024:
        raise ValueError("Type index exceeds bounded capture limit")
    scalar_pointer = struct.unpack_from("<I", object_bytes, 0xac)[0]
    scalar_type = read(scalar_pointer, 0x198)
    generator_type = read(live(0x6a5f80) + type_index * 0x5c9, 0x5c9)
    blocks = [rows, layers, cells, terrain, object_bytes, scalar_type, generator_type]
    values = [obj, x, y, z, plane, word(live(0x6c5c80)), search["budget_before"],
              search["unknown_argument"], *search["coordinates"], word(live(0x6c007d)),
              word(live(0x5e15f0)), cell_base]
    header = HEADER.pack(MAGIC, *[v & 0xffffffff for v in values], *map(len, blocks))
    payload = header + b"".join(blocks)
    if len(payload) > LIMIT:
        raise ValueError("Snapshot exceeds size limit")
    output = Path(output)
    output.write_bytes(payload)
    manifest = {"format": "MNMWLD01", "executable_sha256": EXECUTABLE_HASH, "origin": "live_winedbg", "image_base": image_base,
                "object": obj, "scalar_type_pointer": scalar_pointer,
                "generator_type_pointer": live(0x6a5f80) + type_index * 0x5c9,
                "sha256": hashlib.sha256(payload).hexdigest(), "block_sizes": list(map(len, blocks)),
                "dimensions": [x, y, z], "frozen_inputs": True,
                "context": search.get("context"), "flag_before": flag,
                "scope": "Creature search inputs; continuation requires a preceding captured fresh call; no heap containers captured"}
    output.with_suffix(".json").write_text(json.dumps(manifest, indent=2) + "\n")
    return output.name


def verify_snapshot(path):
    path = Path(path)
    data = path.read_bytes()
    manifest = json.loads(path.with_suffix(".json").read_text())
    if len(data) > LIMIT or hashlib.sha256(data).hexdigest() != manifest["sha256"]:
        raise ValueError("Snapshot hash/size mismatch")
    if len(data) < HEADER.size:
        raise ValueError("Truncated snapshot header")
    fields = HEADER.unpack_from(data)
    if fields[0] != MAGIC or HEADER.size + sum(fields[15:]) != len(data):
        raise ValueError("Snapshot format/block lengths mismatch")
    return manifest
