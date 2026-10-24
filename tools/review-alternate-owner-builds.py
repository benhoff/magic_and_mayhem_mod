#!/usr/bin/env python3
"""Compare literal alternate-pointer uses in the supplied NoCD and retail images.

This closes only literal uses, not computed aliases, external writers or reachability.
The retail image must match the retained installer extraction manifest.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from coverage_evidence import evidence_digest, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
NOCD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RETAIL = '124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214'


def inspect(image, slots):
    decoder = Decoder()
    results = []
    try:
        for slot in slots:
            sites = []
            at = 0
            while True:
                at = image.raw.find(struct.pack('<I', slot), at)
                if at < 0:
                    break
                section = next((s for s in image.sections if
                    s['offset'] <= at <= s['offset'] + s['stored'] - 4), None)
                if section is None:
                    raise ValueError('Unassigned literal outside stored sections')
                address = section['address'] + at - section['offset']
                # These exact encodings establish a read of the global. They do
                # not let a speculative backward disassembly hide another use.
                start = address - (1 if image.read(address - 1, 1) == b'\xa1' else 2)
                row = decoder.decode(image.read(start, 15), start)
                raw = bytes.fromhex(row['bytes'])
                admitted = ((len(raw) == 5 and raw[0] == 0xa1) or
                    (len(raw) == 6 and raw[0] == 0x8b and raw[1] & 0xc7 == 0x05))
                if (not admitted or row['mnemonic'] != 'mov' or
                        row['operand'].split(', ', 1)[1] != f'dword ptr [{hex(slot)}]' or
                        not section['executable']):
                    raise ValueError('Literal use needs separate instruction review: ' + hex(address))
                sites.append(dict(file_offset=at, literal_address=address, instruction=row,
                                  classification='absolute_pointer_read'))
                at += 1
            results.append(dict(slot=slot, initial=image.read(slot, 4, True).hex(), sites=sites))
    finally:
        decoder.close()
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--retail', type=Path, required=True)
    parser.add_argument('--retail-manifest', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    names = ['tools/review-alternate-owner-builds.py', 'tools/original_x86.py',
             'tools/coverage_evidence.py']
    sources = {name: evidence_digest(ROOT/name, root=ROOT) for name in names}
    report = dict(schema=1, success=False, static_only=True, original_sha256=NOCD,
        sources=sources, computed_aliases_closed=False, all_writers_recovered=False,
        original_allocation_recovered=False, actual_unreachability_proved=False)
    before = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        manifest = args.retail_manifest.read_bytes()
        matches = [line.split('\t') for line in manifest.decode().splitlines()
                   if line.endswith('\tChaos.exe')]
        if matches != [[RETAIL, '1929244', 'Chaos.exe']]:
            raise ValueError('Retail extraction manifest differs')
        retail = PE32(args.retail.read_bytes())
        original = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if original.sha256 != NOCD or retail.sha256 != RETAIL:
            raise ValueError('Supplied executable changed')
        report['retail_input'] = dict(path=str(args.retail.resolve()), sha256=RETAIL,
            extraction_manifest_sha256=hashlib.sha256(manifest).hexdigest(),
            extraction_manifest=str(args.retail_manifest.resolve()),
            scope='Retained installer extraction; retail is not a registered live comparison build.')
        # Exact config writer/read witnesses identify the equivalent flag and
        # padding layout. Pointer identities additionally require dispatcher and
        # copy reader witnesses; offsets alone are not an ownership argument.
        report['retail_config_witnesses'] = {hex(a): retail.read(a, n).hex()
            for a, n in [(0x4d5b49, 10), (0x4d505a, 29), (0x4ed528, 14),
                         (0x5765e4, 16), (0x576638, 16)]}
        report['nocd'] = inspect(original, [0x6e1f68, 0x6e1f6c])
        report['retail'] = inspect(retail, [0x6bec10, 0x6bec14])
        if [len(x['sites']) for x in report['nocd']] != [5, 2]:
            raise ValueError('NoCD literal denominator changed')
        if [len(x['sites']) for x in report['retail']] != [3, 2]:
            raise ValueError('Retail literal denominator changed')
        report.update(success=True, direct_literal_publishers=0, direct_literal_releases=0,
            conclusion='Every complete DWORD literal occurrence of either reviewed pointer slot is an absolute read in both images. No direct literal publisher/release is present. This does not exclude computed, bulk, imported or external writes.',
            hypothesis='The alternate malloc route may be dormant or incomplete in these supplied builds; this is not proven global unreachability.')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if before:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                report['original_manifest_verified_before_after'] = True
            except subprocess.SubprocessError as error:
                report.update(success=False, preservation_error=str(error))
        report['sources_stable'] = all(evidence_digest(ROOT/n, root=ROOT) == h
                                       for n, h in sources.items())
        report['success'] &= report['sources_stable']
        sha = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=sha, error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
