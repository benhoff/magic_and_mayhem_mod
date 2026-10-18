#!/usr/bin/env python3
"""Inventory pinned actor/task field accesses; static leads, not behavior proof."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    if a.output.exists():
        p.error('Refusing existing output')
    manifest = [ROOT / 'tools/original-manifest.sh', 'verify']
    subprocess.run(manifest, check=True)
    try:
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        raw = exe.read_bytes()
        assert hashlib.sha256(raw).hexdigest() == PIN
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', raw, pe + 20)[0]
        def constant(va):
            for i in range(struct.unpack_from('<H', raw, pe + 6)[0]):
                rva, size, off = struct.unpack_from('<3I', raw, sections + 40*i + 12)
                if 0x400000+rva <= va and va+8 <= 0x400000+rva+size:
                    b = raw[off+va-0x400000-rva:off+va-0x400000-rva+8]
                    return dict(bytes=b.hex(), value=struct.unpack('<d', b)[0])
            raise ValueError('unbacked constant')
        inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd-owner-planner-20261016.json').read_text())
        asm = subprocess.check_output(['objdump', '-d', '-M', 'intel', str(exe)], text=True)
        offsets = ['922', '926', '936', 'de7', '2f', '33']
        hits = {x: [] for x in offsets}
        for line in asm.splitlines():
            match = re.match(r'\s*([0-9a-f]+):', line)
            if not match:
                continue
            for offset in offsets:
                if re.search(r'\+0x' + offset + r'\]', line):
                    va = int(match[1], 16)
                    owners = [hex(f['entry']) for f in inventory['functions']
                              if any(lo <= va < hi for lo, hi in f['ranges'])]
                    hits[offset].append(dict(address=hex(va), functions=owners, instruction=line.strip()))
        result = dict(success=True, scope='Static displacement search; aliases, indexed accesses and semantic field identity require separate review.',
                      original_sha256=PIN, hits=hits, distress_threshold=constant(0x5c5a88),
                      sources={'tools/inspect-owner-task-inputs.py': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
        a.output.parent.mkdir(parents=True, exist_ok=True)
        with a.output.open('x') as f:
            json.dump(result, f, indent=2)
            f.write('\n')
        assert hashlib.sha256(exe.read_bytes()).hexdigest() == PIN
    finally:
        subprocess.run(manifest, check=True)


if __name__ == '__main__':
    main()
