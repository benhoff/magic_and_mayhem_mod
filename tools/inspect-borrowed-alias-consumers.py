#!/usr/bin/env python3
"""Inspect pinned member-callback thunks and stale source consumers, without patching."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_evidence import read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Require a new immutable report identity')
    result = dict(success=False, static_only=True, original_sha256=PIN,
                  sources={__file__.removeprefix(str(ROOT)+'/'):
                           hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    decoder = Decoder()
    try:
        image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256 != PIN:
            raise ValueError('Unsupported original')
        inventory = read_json(ROOT/'research/runtime/coverage/binary-nocd.json', root=ROOT)
        functions = {f['entry']: f for f in inventory['functions']}
        # This exact21-byte thunk has one indirect member target and ret4.
        # Searching its bytes discovers candidates, not whole-function coverage.
        signature = bytes.fromhex('8bc18b4c24048b5008518b480c03caff5004c20400')
        thunks = []
        for s in image.sections:
            if not s['executable']:
                continue
            raw = image.read(s['address'], s['stored'])
            at = raw.find(signature)
            while at >= 0:
                thunks.append(s['address']+at)
                at = raw.find(signature, at+1)
        tables = []
        for s in image.sections:
            if s['executable']:
                continue
            for at in range(s['address'], s['address']+s['stored']-7, 4):
                if image.word(at) in thunks and image.text(image.word(at+4)):
                    tables.append(dict(address=at, invoke=image.word(at), delete=image.word(at+4)))
        entries = {0x407c10, 0x407ed0, 0x52cc00}
        entries.update(t['delete'] for t in tables)
        # Every deleting wrapper's first direct call is its base-vtable reset.
        for table in tables:
            call=table['delete']+3
            if image.read(call,1)!=b'\xe8':
                raise ValueError('Unexpected callback deleting wrapper')
            import struct
            table['destruct']=call+5+struct.unpack('<i',image.read(call+1,4))[0]
            entries.add(table['destruct'])
        code = {}
        for entry in sorted(entries):
            if entry not in functions:
                raise ValueError('No inventoried boundary for '+hex(entry))
            rows = []
            for start, end in functions[entry]['ranges']:
                while start < end:
                    row = decoder.decode(image.read(start, min(15, end-start)), start)
                    rows.append(row)
                    start += row['size']
            code[hex(entry)] = rows
        for entry in thunks:
            rows, at = [], entry
            while at < entry+len(signature):
                row = decoder.decode(image.read(at, entry+len(signature)-at), at)
                rows.append(row)
                at += row['size']
            code[hex(entry)] = rows
        for table in tables:
            if image.read(table['destruct'],7)!=bytes.fromhex('c701405c5c00c3'):
                raise ValueError('Unexpected base callback destructor')
        result.update(success=True, tables=tables, member_thunks=thunks,
                      instructions=code, member_signature=signature.hex(),
                      anchors={hex(entry):image.read(entry,8).hex() for entry in entries|set(thunks)},
                      thunk_boundary_policy='Exact straight-line signature ending ret4; candidates require reviewed recovered-range exceptions before execution claims.')
    except Exception as exc:
        result['error'] = str(exc)
        raise
    finally:
        decoder.close()
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        result['manifest_before_after'] = True
        write_report(ROOT, a.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(report=str(a.report), tables=tables, thunks=thunks)))


if __name__ == '__main__':
    main()
