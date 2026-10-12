#!/usr/bin/env python3
"""Read-only disassembly and anchors for the pinned ordinary health contract."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'maximum': (0x5075f0, 0x5076a9), 'signed': (0x5076f0, 0x507c20),
          'defended': (0x514860, 0x514982), 'damage': (0x514990, 0x514c56),
          'cleanup': (0x51f5e0, 0x51fba0), 'feedback': (0x416d80, 0x416e80),
          'damage_notice': (0x51fba0, 0x51fd60), 'absorb_notice': (0x51fd60, 0x51ff00)}


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
        data = {hex(0x5c5380): at(0x5c5380, 8).hex()}
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
