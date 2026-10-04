#!/usr/bin/env python3
"""Export pinned No-CD save/load and campaign progression evidence (read-only)."""

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
# Bounded half-open ranges; some include adjacent context, not exact function bodies.
RANGES = {
    'save_load': (0x4ea280, 0x4ea55d),
    'world_tail_read': (0x4ea560, 0x4ea595),
    'save_write': (0x4ea5a0, 0x4ea984),
    'world_tail_write': (0x4ea990, 0x4ea9c8),
    'load_menu_dispatch': (0x4aa5d0, 0x4aa6b8),
    'save_menu_dispatch': (0x4abd00, 0x4abefe),
    'world_save_load_ingress': (0x46afc0, 0x46b10a),
    'realm_config_read': (0x54e230, 0x54e685),
    'realm_read': (0x54e720, 0x54e747),
    'realm_write': (0x54e750, 0x54e777),
    'realm_move_admission': (0x54eec0, 0x54ef57),
    'realm_region_resolution': (0x54e7e0, 0x54ea07),
    'result_text': (0x4745e0, 0x4747b8),
    'wizard_read': (0x5942d0, 0x59430b),
    'wizard_write': (0x594290, 0x5942ca),
    'script_read': (0x56d270, 0x56d30f),
    'script_write': (0x56d200, 0x56d26f),
    'world_write': (0x4713a0, 0x471af0),
    'world_read': (0x471af0, 0x4724b3),
    'file_wrapper_raw_read': (0x4a1690, 0x4a16ea),
    'file_wrapper_packed_write': (0x4a17c0, 0x4a1820),
    'file_wrapper_read': (0x4a1820, 0x4a18a7),
    'file_wrapper_write': (0x4a1790, 0x4a17c0),
    'realm_update': (0x5517a0, 0x5521f4),
    'realm_battle_return': (0x550e10, 0x5510c1),
    'realm_battle_launch': (0x5510d0, 0x5512be),
    'realm_view_cleanup': (0x550370, 0x550599),
    'main_campaign_start': (0x4a75c0, 0x4a7844),
    'file_transform_decode': (0x4a18b0, 0x4a1a16),
    'file_transform_encode': (0x4a1a20, 0x4a1bf1),
    'file_outer_transform': (0x4a1c00, 0x4a1cd4),
    'file_rle_encode': (0x4a1ce0, 0x4a1e76),
    'file_lz_encode': (0x4a28d0, 0x4a29d6),
    'cipher_seed': (0x54e170, 0x54e200),
    'cipher_next': (0x54e200, 0x54e230),
    'wizard_entry_write': (0x5906d0, 0x590800),
    'wizard_entry_read': (0x590800, 0x590930),
    'wizard_list_read': (0x5904c0, 0x590610),
    'wizard_record_write': (0x592e90, 0x593558),
    'wizard_record_read': (0x593560, 0x593a7e),
    'realm_type_selection': (0x593ad0, 0x593b02),
    'result_button': (0x4747a0, 0x4747b5),
    'controller_persistent_read': (0x577610, 0x5776c2),
    'controller_persistent_write': (0x577580, 0x577606),
    'region_occupant_count': (0x54e690, 0x54e6c2),
}
ANCHORS = {0x4ea3db: "837c241414"}



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
    output = Path(tempfile.mkdtemp(prefix="persistence-progression-", dir=parent))
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
        selected = [line for va, line in instructions if start <= va < end]
        if not selected:
            raise ValueError(f"Empty disassembly range: {name}")
        path.write_text("\n".join(selected) + "\n")
        artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    path = output / "direct-references.json"
    path.write_text(json.dumps(references, indent=2) + "\n")
    artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    # Installed plaintext campaign inputs are read-only evidence, separate from the EXE.
    configs = {}
    for realm in ("Celtic", "Greek", "Medieval"):
        path = executable.parent / "Realms" / realm / "RealmView.cfg"
        if not path.is_file():
            continue
        import configparser
        raw = path.read_bytes()
        cfg = configparser.ConfigParser(interpolation=None)
        cfg.read_string(raw.decode("latin-1"))
        configs[realm] = {"source": str(path.resolve()), "sha256": hashlib.sha256(raw).hexdigest(),
                         "general": dict(cfg["GENERAL"]), "sections": cfg.sections()}
    path = output / "realm-configs.json"
    path.write_text(json.dumps(configs, indent=2) + "\n")
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
