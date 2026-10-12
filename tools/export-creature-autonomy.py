#!/usr/bin/env python3
"""Read-only disassembly and anchors for the pinned autonomous decision producers and behavior handlers."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {
 'assist_admission': (0x51e090,0x51e310),
 'attack_response': (0x5186c0,0x518750),
 'immediate': (0x516220,0x5164f0),
 'target_admission': (0x524910,0x524ce0),
 'hazard': (0x5198b0,0x519ae0),
 'escape': (0x51d200,0x51d3a0),
 'orientation': (0x51d3a0,0x51d570),
 'healing_inventory': (0x51d660,0x51d7b0),
 'interaction': (0x51d7b0,0x51db30),
 'destination': (0x51db30,0x51def0),
 'substates': (0x51c600,0x51d200),
 'object_actions': (0x51def0,0x51e090),
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
        data = {hex(a): at(a, n).hex() for a,n in [(0x5e178c,312),(0x5e5450,324),(0x5e5708,324),(0x5e55c4,324),(0x51db0c,32)]}
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
