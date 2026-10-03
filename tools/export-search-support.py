#!/usr/bin/env python3
"""Export hash-checked search support assembly and file-backed movement tables."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parent.parent
HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"
RANGES = {
    "creature_scalar": (0x5205b0, 0x520620),
    "base_scalar": (0x505840, 0x505909),
    "adjust_scalar": (0x505920, 0x505a96),
    "scaled_metric": (0x4eaca0, 0x4eacb1),
    "context_constructor": (0x4bf070, 0x4bf188),
    "priority_insert": (0x430d40, 0x431149),
    "node_lookup": (0x440080, 0x4400ed),
    "node_insert": (0x431150, 0x431258),
    "metric": (0x4eac10, 0x4eac9f),
    "difference_x": (0x40e4e0, 0x40e50d),
    "difference_y": (0x40eb70, 0x40eb9d),
    "direction": (0x4eade0, 0x4eaded),
    "path_append": (0x440260, 0x4403ff),
    "path_end": (0x440180, 0x44018c),
    "path_begin": (0x54be50, 0x54be5c),
    "direct_search_caller": (0x513134, 0x5131e7),
    "local_route_caller": (0x51cc2a, 0x51ccae),
    "movement_argument": (0x4eb030, 0x4eb038),
    "truncate_float": (0x59bee0, 0x59bf07),
    "descriptor_member": (0x40e250, 0x40e285),
    "creature_acceptance": (0x514360, 0x51462c),
    "creature_occupancy": (0x4f5a40, 0x4f5aed),
    "any_terrain": (0x4f44e0, 0x4f45ae),
    "tall_terrain": (0x4f45b0, 0x4f46a9),
    "normalize_y_return": (0x4132e0, 0x413302),
    "boundary_test": (0x4f41a0, 0x4f431c),
    "cell_support": (0x4f3320, 0x4f3439),
    "record_query": (0x4f4330, 0x4f44db),
    "cell_validity_test": (0x4f3440, 0x4f354c),
    "validity_test": (0x4f46b0, 0x4f47c4),
    "occupancy_test": (0x4f3550, 0x4f35d0),
    "cell_test": (0x4f47d0, 0x4f48f6),
    "movement_test": (0x4f3990, 0x4f41a0),
}


def export(executable: Path) -> Path:
    data = executable.read_bytes()
    if hashlib.sha256(data).hexdigest() != HASH:
        raise ValueError("Unsupported executable hash; refusing build-specific export")
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    image_base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    sections = []
    for index in range(count):
        section = pe + 24 + optional_size + index * 40
        _, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, section + 8)
        sections.append((image_base + rva, raw_size, raw_offset))

    def read(va, length):
        for base, size, offset in sections:
            if base <= va and va + length <= base + size:
                return data[offset + va - base:offset + va - base + length]
        raise ValueError(f"Not file-backed: {va:x}")

    scalar_dispatch = list(struct.unpack("<5I", read(0x50590c, 20)))
    scalar_slope_factor = struct.unpack("<f", read(0x5c7260, 4))[0]
    boundary_dispatch = list(struct.unpack("<4I", read(0x4f431c, 16)))
    neighbors = list(struct.unpack("<78i", read(0x5e178c, 26 * 12)))
    directions = list(struct.unpack("<9i", read(0x5e41b4, 9 * 4)))
    local_offsets = list(struct.unpack("<81i", read(0x5e55c4, 27 * 12)))
    link_tables = {name: list(struct.unpack("<6i", read(va, 24)))
                   for name, va in {"x": 0x5c70c0, "y": 0x5c70d8, "z": 0x5c70f0,
                                    "masks": 0x5c7108, "costs": 0x5c7120}.items()}
    link_step_bytes = read(0x5c5388, 4)
    parent = REPO / "working/decompiled"
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="search-support-", dir=parent))
    artifacts = {}
    for name, (start, end) in RANGES.items():
        result = subprocess.run(["objdump", "-d", "-Mintel", f"--start-address={start}",
                                 f"--stop-address={end}", str(executable)],
                                check=True, capture_output=True, text=True)
        path = output / f"{name}.asm"
        path.write_text(result.stdout)
        artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    metadata = {"sha256": HASH, "input": str(executable.resolve()), "ranges": RANGES,
                "scalar_dispatch_va": "0x0050590c", "scalar_dispatch": scalar_dispatch,
                "scalar_slope_factor_va": "0x005c7260", "scalar_slope_factor": scalar_slope_factor,
                "boundary_dispatch_va": "0x004f431c", "boundary_dispatch": boundary_dispatch,
                "neighbor_offsets_va": "0x005e178c",
                "neighbor_offsets": [neighbors[index:index + 3] for index in range(0, 78, 3)],
                "direction_table_va": "0x005e41b4", "directions": directions,
                "local_caller_offsets_va": "0x005e55c4",
                "local_caller_offsets": [local_offsets[index:index + 3] for index in range(0, 81, 3)],
                "six_link_tables": link_tables,
                "six_link_float_step_va": "0x005c5388",
                "six_link_float_step_hex": link_step_bytes.hex(),
                "six_link_float_step": struct.unpack("<f", link_step_bytes)[0],
                "artifacts": artifacts,
                "input_unchanged": hashlib.sha256(executable.read_bytes()).hexdigest() == HASH}
    (output / "manifest.json").write_text(json.dumps(metadata, indent=2) + "\n")
    if not metadata["input_unchanged"]:
        raise ValueError("Input changed during read-only export")
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", nargs="?", type=Path, default=REPO / "working/game-nocd/Chaos.exe")
    args = parser.parse_args()
    print(f"Evidence directory: {export(args.executable)}")
