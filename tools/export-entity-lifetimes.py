#!/usr/bin/env python3
"""Export pinned No-CD entity allocation, cleanup, release and reuse evidence (read-only)."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"
# Half-open instruction ranges; labels describe static findings, not live validation.
RANGES = {
    "creature_pool_prepare": (0x473100, 0x4731ff),
    "creature_constructor": (0x505db0, 0x505f70),
    "creature_destructor": (0x505f70, 0x50607c),
    "creature_slot_reset": (0x5060a0, 0x50621c),
    "creature_activate": (0x506290, 0x5065d7),
    "creature_baseline_reset": (0x5065e0, 0x506d25),
    "creature_release": (0x506d30, 0x5070a8),
    "creature_position_prepare": (0x5070e0, 0x507189),
    "creature_reference_invalidation": (0x508b00, 0x508c95),
    "creature_map_attach": (0x509230, 0x5093da),
    "creature_map_detach": (0x5093e0, 0x50962a),
    "creature_expiry_context": (0x50a198, 0x50a21f),
    "creature_death_transition": (0x50cdc0, 0x50cf29),
    "creature_cleanup": (0x51f5e0, 0x51fa5c),
    "creature_pool_reset": (0x52a5c0, 0x52a632),
    "creature_first_free": (0x52a640, 0x52a661),
    "creature_iterator": (0x52a670, 0x52a773),
    "creature_presentation_cleanup": (0x52baf0, 0x52bc30),
    "creature_array_delete_helper": (0x416530, 0x416585),
    "creature_effect_spawn": (0x48f690, 0x48f930),
    "creature_map_deployment": (0x4f0320, 0x4f0696),
    "creature_scenario_spawn_context": (0x5676d0, 0x567840),
    "creature_other_spawn": (0x592780, 0x592e83),
    "secondary_activate": (0x493f20, 0x4943b9),
    "secondary_release": (0x4943f0, 0x4945e3),
    "secondary_pool_reset": (0x49cc80, 0x49cda1),
    "secondary_general_admission": (0x49cdb0, 0x49ceb5),
    "secondary_reserved_admission": (0x49cec0, 0x49cee3),
    "third_pool_activate": (0x543970, 0x543c51),
    "third_pool_release": (0x543c60, 0x543d3b),
    "world_teardown_entity_context": (0x46ab30, 0x46ac3d),
    "world_pool_setup_context": (0x470a5e, 0x470c17),
}
ANCHORS = {
    0x473145: "68705f5000",       # Creature destructor supplied to array construction.
    0x506201: "8903",             # Slot reset installs its index.
    0x50639e: "896e04",           # Creature activation.
    0x5063ac: "a380f16d00",       # Creature high-water bound grows.
    0x506e03: "895d04",           # Creature deactivation.
    0x506d59: "e8a21d0000",       # Peer reference invalidation.
    0x49433a: "894d04",           # Secondary activation.
    0x4944a5: "897e04",           # Secondary deactivation.
    0x543a96: "a360126b00",       # Third pool high-water bound grows.
}


def export(executable):
    data = executable.read_bytes()
    if hashlib.sha256(data).hexdigest() != HASH:
        raise ValueError("Unsupported executable hash; refusing build-specific export")
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    opt = pe + 24
    base = struct.unpack_from("<I", data, opt + 28)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    size = struct.unpack_from("<H", data, pe + 20)[0]
    sections = []
    for index in range(count):
        rva, raw_size, raw = struct.unpack_from("<III", data, opt + size + index * 40 + 12)
        sections.append((base + rva, raw_size, raw))

    def read(va, length):
        for start, length_on_disk, offset in sections:
            if start <= va and va + length <= start + length_on_disk:
                return data[offset + va - start:offset + va - start + length]
        raise ValueError(f"Address is not file-backed: 0x{va:08x}")

    for va, expected in ANCHORS.items():
        if read(va, len(expected) // 2).hex() != expected:
            raise ValueError(f"Instruction anchor mismatch: 0x{va:08x}")
    disassembly = subprocess.run(["objdump", "-d", "-Mintel", str(executable)],
                                 check=True, capture_output=True, text=True).stdout
    instructions = []
    for line in disassembly.splitlines():
        match = re.match(r"\s*([0-9a-f]+):\s", line)
        if match:
            instructions.append((int(match[1], 16), line))
    parent = REPO / "working/decompiled"
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="entity-lifetimes-", dir=parent))
    artifacts = {}
    references = {name: [] for name in RANGES}
    names_by_start = {start: name for name, (start, _) in RANGES.items()}
    for index, (va, line) in enumerate(instructions):
        match = re.search(r"\b(call|jmp)\s+0x([0-9a-f]+)\b", line)
        if match and int(match[2], 16) in names_by_start:
            name = names_by_start[int(match[2], 16)]
            references[name].append({"site": f"0x{va:08x}", "kind": match[1],
                "context": [s for _, s in instructions[max(0, index - 5):index + 2]]})
    for name, (start, end) in RANGES.items():
        path = output / (name + ".asm")
        path.write_text("\n".join(line for va, line in instructions if start <= va < end) + "\n")
        artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    path = output / "direct-references.json"
    path.write_text(json.dumps(references, indent=2) + "\n")
    artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    if hashlib.sha256(executable.read_bytes()).hexdigest() != HASH:
        raise ValueError("Executable changed during export")
    manifest = {
        "schema": 1, "source": str(executable.resolve()), "source_sha256": HASH,
        "image_base": f"0x{base:08x}", "scope": "Static only; no live execution or replacement",
        "ranges": {name: [f"0x{a:08x}", f"0x{b:08x}"] for name, (a, b) in RANGES.items()},
        "instruction_anchors": {f"0x{va:08x}": value for va, value in ANCHORS.items()},
        "reference_limits": "Direct textual call/jmp references only; objdump may decode inline data. "
                            "Indirect calls require separate vtable/control-flow review.",
        "artifacts": artifacts,
        "objdump": subprocess.run(["objdump", "--version"], check=True,
                                   capture_output=True, text=True).stdout.splitlines()[0],
        "input_unchanged": True,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", nargs="?", type=Path,
                        default=REPO / "working/game-nocd/Chaos.exe")
    args = parser.parse_args()
    try:
        print(export(args.executable.resolve()))
        return 0
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Error: {error}\n")


if __name__ == "__main__":
    raise SystemExit(main())
