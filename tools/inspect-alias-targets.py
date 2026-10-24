#!/usr/bin/env python3
"""Read-only pinned callback-family, actor event and list cursor inspection."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from coverage_evidence import read_json, write_report
from original_x86 import Decoder, PE32
ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def inspect(image, decoder):
    inventory = read_json(ROOT/'research/runtime/coverage/binary-nocd.json', root=ROOT)
    functions = {f['entry']: f for f in inventory['functions']}
    destructors = set()
    def matches(raw, signature):
        at = raw.find(signature)
        while at >= 0:
            yield at
            at = raw.find(signature, at+1)
    for section in image.sections:
        if section['executable']:
            raw = image.read(section['address'], section['stored'])
            destructors.update(section['address']+at for at in matches(raw, bytes.fromhex('c701405c5c00c3')))
    tables = []
    signatures = {'adjusted': bytes.fromhex('8bc18b4c24048b5008518b480c03caff5004c20400'),
                  'direct': bytes.fromhex('8bc18b4c2404518b4808ff5004c20400')}
    for section in image.sections:
        if section['executable']:
            continue
        for table in range(section['address'], section['address']+section['stored']-7, 4):
            invoke, delete = image.word(table), image.word(table+4)
            if not image.text(invoke) or not image.text(delete):
                continue
            raw = image.read(delete, 8)
            if raw[3] != 0xe8:
                continue
            destruct = delete+8+struct.unpack('<i', raw[4:8])[0]
            if destruct not in destructors:
                continue
            kind = next((k for k, sig in signatures.items() if image.read(invoke, len(sig)) == sig), None)
            if kind is None:
                raise ValueError('Unreviewed callback representation at '+hex(invoke))
            tables.append(dict(table=table, invoke=invoke, deleting=delete, destruct=destruct, representation=kind))
    ranges = {0x51e890: [(0x51e890, 0x51ec59)]}
    for entry in [0x523be0, 0x524130, 0x524df0, 0x527310, 0x40e650, 0x4132e0]:
        if entry in functions:
            ranges[entry] = functions[entry]['ranges']
    for c in tables:
        ranges[c['invoke']] = [(c['invoke'], c['invoke']+len(signatures[c['representation']]))]
        for key in ['deleting', 'destruct']:
            ranges[c[key]] = functions[c[key]]['ranges']
    instructions = {}
    cursors = []
    for entry, spans in ranges.items():
        rows = []
        for start, end in spans:
            while start < end:
                row = decoder.decode(image.read(start, min(15, end-start)), start)
                rows.append(row)
                if '0x8fd]' in row['operand']:
                    cursors.append(dict(entry=entry, **row))
                start += row['size']
        instructions[hex(entry)] = rows
    return dict(success=True, static_only=True, original_sha256=PIN, tables=tables,
                instructions=instructions, cursor_references=cursors,
                event_jump_table=[image.word(0x51ec5c+i*4) for i in range(4)],
                event_case_map=list(image.read(0x51ec6c,10)),
                limitations=['Base-reset/deleting-wrapper candidate family only; not all engine callback types.',
                              'Initial exploratory linear decode entered switch data at51ec5c and failed at51ec75; exact body ends51ec59. Original manifest reverified.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Require a new immutable identity')
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True)
    decoder = Decoder()
    result = dict(success=False, static_only=True)
    try:
        image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256 != PIN:
            raise ValueError('Unsupported original image')
        result = inspect(image, decoder)
    finally:
        decoder.close()
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'], check=True)
        result['manifest_before_after'] = True
        result['sources'] = {str(Path(__file__).relative_to(ROOT)): hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
        write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], classes=len(result['tables']), cursor_references=len(result['cursor_references']))))

if __name__ == '__main__':
    main()
