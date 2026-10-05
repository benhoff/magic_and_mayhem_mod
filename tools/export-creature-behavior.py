#!/usr/bin/env python3
"""Export pinned No-CD creature orders, AI, combat and spell evidence (read-only)."""

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
    "spell_effect_dispatch": (0x48b0f0, 0x48ea29),
    "projectile_impact_damage": (0x48ecf0, 0x48ee38),
    "projectile_impact_transition": (0x48ee40, 0x48f113),
    "secondary_update_dispatch": (0x4970e0, 0x499e24),
    "signed_health_change": (0x5076f0, 0x507c20),
    "inventory_action": (0x509ad0, 0x509b55),
    "behavior_dispatch": (0x50a930, 0x50a9e2),
    "behavior_transition": (0x50aa30, 0x50ab05),
    "behavior_reset": (0x50ab30, 0x50ac91),
    "idle_behavior": (0x50acb0, 0x50ad03),
    "creature_target_order": (0x50ad10, 0x50af02),
    "target_order_network_wrapper": (0x50af10, 0x50af9f),
    "target_attack_behavior": (0x50afa0, 0x50b0b1),
    "melee_range_gate": (0x50bd60, 0x50c0c5),
    "melee_damage_resolution": (0x50c0d0, 0x50c92f),
    "coordinate_attack_order": (0x50ca30, 0x50cc0e),
    "coordinate_attack_behavior": (0x50ccd0, 0x50cdb4),
    "death_transition": (0x50cdc0, 0x50cf29),
    "action_dispatch": (0x50da50, 0x50daf8),
    "action_transition": (0x50db60, 0x50e36b),
    "ranged_animation_action": (0x50e670, 0x50e6ea),
    "alternate_ranged_action": (0x50e6f0, 0x50e791),
    "melee_animation_action": (0x50e7a0, 0x50eb24),
    "shared_animation_action": (0x50f090, 0x50f329),
    "ready_action": (0x50f7c0, 0x50f84f),
    "move_order": (0x50fae0, 0x50fd4c),
    "movement_behavior": (0x510010, 0x510480),
    "motion_action": (0x5104b0, 0x510e3a),
    "motion_segment_setup": (0x510e80, 0x511c0c),
    "motion_route_consumption": (0x512460, 0x5127f9),
    "motion_coordinate_snap": (0x5070e0, 0x507189),
    "defended_damage_entry": (0x514860, 0x514982),
    "health_damage_and_lethal_transition": (0x514990, 0x514c56),
    "immediate_interaction_gate": (0x516220, 0x5164e5),
    "alternate_melee_action": (0x5170e0, 0x517427),
    "blood_lust_status_set": (0x518100, 0x518140),
    "periodic_behavior_decision": (0x518420, 0x5186b3),
    "ally_assistance_selector": (0x518750, 0x5189df),
    "home_radius_selector": (0x5189e0, 0x518c49),
    "combat_score_selector": (0x518c50, 0x518f77),
    "type15_corpse_selector": (0x518f80, 0x5190fb),
    "type0a_corpse_selector": (0x519100, 0x5192c9),
    "priority_threat_response": (0x519330, 0x51983e),
    "local_hazard_score": (0x5198b0, 0x519ad8),
    "budgeted_route_work": (0x519ae0, 0x51a5b6),
    "autonomous_substate_set": (0x51c3a0, 0x51c506),
    "autonomous_substate_dispatch": (0x51c510, 0x51c5d4),
    "escape_behavior": (0x51d200, 0x51d391),
    "vertical_interaction_order": (0x51e310, 0x51e46c),
    "area_explosion_and_release": (0x520100, 0x520539),
    "queued_order_dispatch": (0x520ad0, 0x520b24),
    "periodic_pre_decision": (0x522ad0, 0x522dfe),
    "nonnetwork_periodic_work": (0x523470, 0x523858),
    "queued_order_insert": (0x523be0, 0x5240ae),
    "queued_order_clear": (0x5243b0, 0x524460),
    "follow_reference_set": (0x524460, 0x5244ea),
    "periodic_follow_work": (0x5244f0, 0x5248e2),
    "target_candidate_validation": (0x524910, 0x524cd2),
    "creature_budget_scheduler": (0x52a780, 0x52a9ee),
    "creature_environment_pass": (0x52a9f0, 0x52ab17),
    "packed_cast_decode": (0x534000, 0x534090),
    "command_delivery_gate": (0x5383c0, 0x53869a),
    "packed_command_dispatch": (0x539080, 0x5395e9),
    "cast_admission": (0x57b710, 0x57c631),
}
ANCHORS = {
    0x5104b0: "83ec3853558be9", # Motion action entry.
    0x512460: "83ec4053558be9", # Route consumption entry.
    0x5070e0: "8b5424048b442408", # Discrete/fine coordinate snap.
    0x50e6a6: "8d8e1c060000",  # Cast receiver is embedded creature +0x61c.
    0x50e6ae: "e95dd00600",    # Ranged animation event enters cast admission.
    0x48b5d3: "e818c10700",    # Cure uses signed health change.
    0x50e6e1: "ff96f4050000",  # Behavior callback after ranged completion.
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
    output = Path(tempfile.mkdtemp(prefix="creature-behavior-", dir=parent))
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
