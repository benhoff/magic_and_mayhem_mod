#!/usr/bin/env python3
"""Retain pinned queue dispatch targets and disassembly; no admission inference."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def inspect(executable):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != HASH:
        raise ValueError('Unsupported original executable')
    pe = struct.unpack_from('<I', raw, 0x3c)[0]
    sections, optional = struct.unpack_from('<H', raw, pe+6)[0], struct.unpack_from('<H', raw, pe+20)[0]
    base = struct.unpack_from('<I', raw, pe+24+28)[0]
    if raw[:2] != b'MZ' or raw[pe:pe+4] != b'PE\0\0' or base != 0x400000:
        raise ValueError('Unsupported PE identity')
    def at(address, length):
        for index in range(sections):
            row = pe+24+optional+index*40
            _, va, size, offset = struct.unpack_from('<4I', raw, row+8)
            delta = address-base-va
            if 0 <= delta and delta+length <= size:
                return raw[offset+delta:offset+delta+length]
        raise ValueError('Address lacks original file-backed bytes')
    if at(0x5002a0, 9) != bytes.fromhex('83 ec 18 53 b8 00 6c ca 88') or at(0x5003fc, 7) != bytes.fromhex('ff 24 85 84 11 50 00'):
        raise ValueError('Queue entry/table dispatch signature mismatch')
    tables = []
    for dispatch, table in [(0x5003fc,0x501184),(0x50093d,0x501210),(0x500d69,0x50129c)]:
        data = at(table, 35*4)
        tables.append({'dispatch': f'0x{dispatch:08x}', 'table': f'0x{table:08x}',
                       'sha256': hashlib.sha256(data).hexdigest(),
                       'entries': [{'kind':kind, 'target':f'0x{target:08x}'}
                                   for kind,target in enumerate(struct.unpack('<35I',data))]})
    assembly = subprocess.check_output(['objdump','-d','-Mintel','--start-address=0x5002a0',
                                        '--stop-address=0x501150',str(executable)], text=True)
    return {'schema':1, 'source_executable_sha256':HASH, 'tables':tables, 'disassembly':assembly,
            'scope':'Pinned static targets for three 0..34 dispatch tables. Kind33 has a pre-table terrain branch; unsigned defaults/hidden kinds and actual mode selection must be reviewed separately. Targets are not complete raster semantics or native admission.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, default=ROOT/'working/game-nocd/Chaos.exe')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True)
    try:
        report = inspect(args.executable)
        report['sources'] = {'tools/inspect-world-dispatch.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
        with args.output.open('x') as output:
            json.dump(report, output, indent=2)
            output.write('\n')
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True)
    print(args.output)


if __name__ == '__main__':
    main()
