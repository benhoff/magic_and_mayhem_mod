#!/usr/bin/env python3
"""Read-only disassembly and anchors for the pinned creature scheduling, orders, transitions and aiming."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {
 'order_containers': (0x52cb50,0x52cd70),
 'order_nodes': (0x52d0c0,0x52d220),
 'follow_execution': (0x513850,0x513990),
 'reference_repair': (0x508b00,0x508db0),
 'maintenance': (0x509e50,0x50a930),
 'behavior_helpers': (0x50b0c0,0x50b2c0),
 'follow_and_routes': (0x522e00,0x523be0),
 'movement_behavior': (0x510010,0x510480),
 'special_behaviors': (0x50d580,0x50da50),
 'special_behavior_entries': (0x51c230,0x51c3a0),
 'special_behavior_tail': (0x521750,0x521790),
 'attack_animation': (0x50e400,0x50e670),
 'action_callbacks': (0x50e670,0x50fae0),
 'alternate_melee_action': (0x5170e0,0x517450),
 'special_action_callbacks': (0x521790,0x522340),
 'motion_action': (0x510480,0x510e80),
 'ranged_style': (0x505cf0,0x505d60),
 'scheduler': (0x52a780,0x52a9f0),
 'route': (0x519ae0,0x51a7f0),
 'periodic_terrain': (0x522ad0,0x522e00),
 'valuation': (0x5244f0,0x524910),
 'follow': (0x523470,0x523870),
 'orders': (0x523be0,0x5244f0),
 'queue_delivery': (0x5120c0,0x512800),
 'behavior_transition': (0x50a930,0x50ad10),
 'action_transition': (0x50da50,0x50e620),
 'aim': (0x50b2c0,0x50bd60),
 'order_ingress': (0x50ad10,0x50b2c0),
 'coordinate_order': (0x50ca30,0x50ce30),
 'move_order': (0x50fae0,0x50fd50),
}




def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    if args.output.exists():
        p.error('Refusing existing evidence output')
    verify = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(verify, check=True)
    try:
        source = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        raw = source.read_bytes()
        if hashlib.sha256(raw).hexdigest() != HASH:
            raise ValueError('Unsupported immutable executable')
        pe, = struct.unpack_from('<I', raw, 60)
        opt = pe + 24
        sections = opt + struct.unpack_from('<H', raw, pe + 20)[0]

        def at(va, size):
            for i in range(struct.unpack_from('<H', raw, pe + 6)[0]):
                rva, n, offset = struct.unpack_from('<3I', raw, sections + i * 40 + 12)
                if 0x400000 + rva <= va and va + size <= 0x400000 + rva + n:
                    return raw[offset + va - 0x400000 - rva:offset + va - 0x400000 - rva + size]
            raise ValueError('Address not file backed')
        args.output.mkdir(parents=True)
        anchors, artifacts = {}, {}
        for name, (start, end) in RANGES.items():
            result = subprocess.run(['objdump', '-D', '-Mintel', f'--start-address={start}',
                                     f'--stop-address={end}', str(source)], capture_output=True, check=True)
            path = args.output / (name + '.asm')
            path.write_bytes(result.stdout)
            artifacts[path.name] = hashlib.sha256(result.stdout).hexdigest()
            anchors[hex(start)] = at(start, 16).hex()
        # Read-only caller discovery for the remaining reference and AI fields.
        # Retain only bounded contexts; textual matches are not semantic proof.
        all_text = subprocess.run(['objdump', '-d', '-Mintel', str(source)], capture_output=True, text=True, check=True).stdout.splitlines()
        field_references = {}
        for field in ['0x906', '0xbdb', '0x8e5', '0x6f37f8']:
            field_references[field] = [all_text[max(0, i-6):i+7] for i, line in enumerate(all_text) if field in line]
        contexts = json.dumps(field_references, indent=2).encode() + b'\n'
        (args.output / 'field-references.json').write_bytes(contexts)
        artifacts['field-references.json'] = hashlib.sha256(contexts).hexdigest()

        data = {hex(a): at(a, n).hex() for a,n in [(0x50d818,35),(0x50e36c,100),(0x5e51e4,100),(0x5e5248,100),(0x5e52ac,100),(0x5e1910,8),(0x50a9e4,64),(0x50daf8,100),(0x51a5b8,72),(0x5248e4,30),(0x5c7270,48),(0x5c5378,40),(0x5c6070,8)]}
        assert source.read_bytes() == raw
        methods = {0x509e50, 0x50a8d0, 0x52a780, 0x519ae0, 0x524460, 0x513850,
                   0x508b00, 0x5244f0, 0x522ad0, 0x523470, 0x523860, 0x523be0,
                   0x524130, 0x5243b0, 0x520ad0, 0x50aa30, 0x50ab30, 0x50ad10,
                   0x50ca30, 0x50fae0, 0x50a930, 0x50da50, 0x50db60, 0x50b0c0,
                   0x50b2c0, 0x505cf0, 0x50e400, 0x510e80, 0x512460, 0x5128a0,
                   0x51a600, 0x50b650, 0x50ed80, 0x50ec80}
        # The tables point to resolver arms, not directly to method bodies.
        # Each non-null arm starts with MOV EAX, imm32; retain both anchors.
        for table, count in ((0x50a9e4, 16), (0x50daf8, 25)):
            for arm in struct.unpack('<' + str(count) + 'I', at(table, count * 4)):
                methods.add(arm)
                instruction = at(arm, 5)
                if instruction[0] == 0xb8:
                    methods.add(struct.unpack_from('<I', instruction, 1)[0])
        methods.discard(0)
        method_anchors = {hex(a): at(a, 16).hex() for a in sorted(methods)}
        report = dict(schema=1, scope='Static read-only exports and exact method anchors, not execution equivalence. Tables identify dispatch targets without asserting complete semantics.',
                      source_sha256=HASH, anchors=anchors, data=data, artifacts=artifacts,
                      method_anchors=method_anchors, input_unchanged=True,
                      exporter_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest())
    finally:
        subprocess.run(verify, check=True)
    report['original_manifest_verified_before_after'] = True
    (args.output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
