#!/usr/bin/env python3
"""Read-only disassembly and anchors for the pinned creature AI, reach and stun contracts."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {
 'reach': (0x50bd60,0x50c0d0), 'response': (0x50c930,0x50ca30),
 'stun_admit': (0x521160,0x521420), 'stun_response': (0x521420,0x5215c0),
 'stun_begin': (0x50d060,0x50d580), 'ready': (0x50f7c0,0x50f850),
 'ai': (0x518420,0x518750), 'assist': (0x518750,0x5189e0),
 'home': (0x5189e0,0x518c50), 'score': (0x518c50,0x518f80),
 'corpse21': (0x518f80,0x519100), 'corpse10': (0x519100,0x519330),
 'priority': (0x519330,0x5198b0), 'hazard': (0x5198b0,0x519ae0),
 'substate': (0x51c3a0,0x51c600), 'escape': (0x51d200,0x51d760),
 'attack': (0x50afa0,0x50bd60), 'eligibility': (0x524910,0x524ac0),
 'maintenance': (0x509e50,0x50a930), 'stun_tick': (0x50d850,0x50d930), 'stun_action': (0x50f590,0x50f7c0),
 'distance': (0x4f4b30,0x4f4ba0), 'target_admission': (0x524910,0x524ad0),
 'escape_selector': (0x51d660,0x51d7b0), 'spell_selector': (0x51d7b0,0x51dfc0),
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
        # Immutable data constant used by ordinary lethal feedback.
        data = {hex(a): at(a, n).hex() for a,n in [(0x5e51e4,200),(0x5e1758,40)]}
        assert source.read_bytes() == raw
        report = dict(schema=1, scope='Static read-only exports, not execution equivalence.',
                      source_sha256=HASH, anchors=anchors, data=data, artifacts=artifacts,
                      input_unchanged=True)
    finally:
        subprocess.run(verify, check=True)
    report['original_manifest_verified_before_after'] = True
    (args.output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
