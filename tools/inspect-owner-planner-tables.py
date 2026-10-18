#!/usr/bin/env python3
"""Export pinned planner vtables/constants without running or changing the game."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if a.output.exists():
        p.error('Refusing existing output')
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    try:
        b = (ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(b).hexdigest() != PIN:
            raise ValueError('Unsupported build')
        pe = struct.unpack_from('<I', b, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', b, pe+20)[0]
        def data(va, size):
            for i in range(struct.unpack_from('<H', b, pe+6)[0]):
                rva, n, offset = struct.unpack_from('<3I', b, sections+40*i+12)
                if 0x400000+rva <= va and va+size <= 0x400000+rva+n:
                    return b[offset+va-0x400000-rva:offset+va-0x400000-rva+size]
            raise ValueError('Unbacked address')
        tables = {hex(va): [hex(x) for x in struct.unpack('<7I', data(va,28))]
                  for va in [0x5c5358,0x5c53c0,*range(0x5c53f4,0x5c5a68,28)]}
        constants = {hex(va): {'bytes': data(va,8).hex(),
                     'double': struct.unpack('<d',data(va,8))[0], 'float': struct.unpack('<f',data(va,4))[0]}
                     for va in [*range(0x5c5378,0x5c53a8,8),0x5c53b4,0x5c53b8,0x5c5a68,0x5c5a88,0x5c5bb0,0x5c5f80,0x5c5f88]}
        result = dict(original_sha256=PIN, scope='Static vtable words and double constants only; no execution claim.',
                      tables=tables, constants=constants,
                      sources={str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
        # JSON permits finite values only; all selected constants are finite.
        a.output.parent.mkdir(parents=True,exist_ok=True)
        a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'],check=True)

if __name__ == '__main__':
    main()
