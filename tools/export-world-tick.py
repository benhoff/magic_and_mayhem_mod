#!/usr/bin/env python3
"""Export pinned No-CD world-update, dispatch, pacing and timer evidence (read-only)."""

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
    "gameplay_constructor_tail": (0x46a0e0, 0x46a14f),
    "gameplay_enter": (0x46a460, 0x46a8e0),
    "world_update": (0x46afc0, 0x46bcab),
    "creature_update": (0x509e50, 0x50a8d0),
    "creature_budget_scheduler": (0x52a780, 0x52a9ee),
    "creature_secondary_pass": (0x52a9f0, 0x52ab17),
    "creature_scheduled_work": (0x519ae0, 0x51a5c0),
    "cached_search_budget": (0x513134, 0x5131e7),
    "screen_message_loop": (0x557370, 0x557451),
    "screen_push": (0x557040, 0x557130),
    "screen_pop": (0x557130, 0x557194),
    "outer_pacing": (0x4e3e40, 0x4e40c3),
    "world_command_gate": (0x5383c0, 0x5386a0),
    "timer_wrappers": (0x534aa0, 0x534b7d),
    "cursor_timer_callback": (0x4eaaf0, 0x4eaaf8),
    "cursor_timer_worker": (0x4a4910, 0x4a4de4),
    "timer_start_context": (0x5576f0, 0x557742),
    "one_second_callback": (0x469a60, 0x469aa2),
    "wrapped_world_update": (0x4757c0, 0x4757e5),
    "alternate_world_update_context": (0x576879, 0x57689f),
}
ANCHORS = {
    0x557421: "ff5010",           # Active screen vtable +0x10.
    0x46a10b: "c706d85d5c00",     # Gameplay vtable installation.
    0x46b496: "a330486c00",       # World-counter store.
    0x46b735: "e816e70900",       # Per-creature update call.
    0x52a78f: "c7873002000035000000",  # Scheduler budget = 53.
    0x534af3: "ff15e0525c00",     # No-CD timeSetEvent slot.
    0x4eaaf0: "e81b9efbff",       # 20 ms callback calls cursor worker.
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
    vtable = list(struct.unpack("<6I", read(0x5c5dd8, 24)))
    if vtable[4] != 0x46afc0:
        raise ValueError("Gameplay update vtable slot mismatch")
    disassembly = subprocess.run(["objdump", "-d", "-Mintel", str(executable)],
                                 check=True, capture_output=True, text=True).stdout
    instructions = []
    for line in disassembly.splitlines():
        match = re.match(r"\s*([0-9a-f]+):\s", line)
        if match:
            instructions.append((int(match[1], 16), line))
    parent = REPO / "working/decompiled"
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="world-tick-", dir=parent))
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
        "gameplay_vtable_va": "0x005c5dd8",
        "gameplay_vtable": [f"0x{value:08x}" for value in vtable],
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
