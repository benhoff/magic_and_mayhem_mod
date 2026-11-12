#!/usr/bin/env python3
"""Compare official1.100/NoCD bytes without inferring an absent allocator."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from coverage_evidence import evidence_digest, read_bytes, read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
NOCD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
UPDATED = '405001ab761c3d9bc3007cc9dfa2210ab66f429e37440eb9108ffcee658e8809'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    names = ['tools/review-original-update-owner.py', 'tools/unpack-original-update.py',
             'tools/apply-original-update.py', 'tests/original-update-host.c',
             'tools/review-alternate-owner-builds.py', 'tools/original_x86.py']
    sources = {n: evidence_digest(ROOT/n, root=ROOT) for n in names}
    report = dict(schema=1, success=False, static_only=True, sources=sources,
                  original_owner_recovered=False, all_writers_closed=False,
                  global_unreachability_proved=False, original_game_executed=False)
    before = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        receipt_path = 'research/runtime/alternate-original-results-20261025/apply03.json'
        receipt = read_json(ROOT/receipt_path, root=ROOT)
        if not receipt['success'] or not receipt['baseline_verified_before_after'] or not receipt['private_clients_stopped']:
            raise ValueError('Unadmitted official-update reconstruction')
        official = PE32(read_bytes(ROOT/receipt['updated']['record'], root=ROOT))
        nocd = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if official.sha256 != UPDATED or nocd.sha256 != NOCD or official.sections != nocd.sections:
            raise ValueError('Image/layout changed')
        differences = []
        for s in official.sections:
            x = official.read(s['address'], s['stored'])
            y = nocd.read(s['address'], s['stored'])
            indices = [i for i in range(len(x)) if x[i] != y[i]]
            groups = []
            for i in indices:
                if not groups or i != groups[-1][-1] + 1:
                    groups.append([i])
                else:
                    groups[-1].append(i)
            differences.extend(dict(section=s['name'], address=s['address']+g[0],
                                    official=x[g[0]:g[-1]+1].hex(), nocd=y[g[0]:g[-1]+1].hex()) for g in groups)
        expected = [(0x482335,6), (0x482365,2), (0x4823e1,6), (0x4825f8,2)]
        if [(r['address'],len(bytes.fromhex(r['official']))) for r in differences] != expected:
            raise ValueError('Unreviewed official/NoCD code difference')
        decoder = Decoder()
        try:
            for r in differences:
                if r['nocd'] != '90' * len(bytes.fromhex(r['official'])):
                    raise ValueError('Unexpected NoCD edit')
                r['official_instruction'] = decoder.decode(official.read(r['address'],15), r['address'])
                if r['official_instruction']['size'] != len(bytes.fromhex(r['official'])):
                    raise ValueError('Difference crosses instruction')
        finally:
            decoder.close()
        inventory_path = 'research/runtime/coverage/binary-nocd-alternate-canvas-20261020.json'
        inventory = read_json(ROOT/inventory_path, root=ROOT)
        entries = {0x4a88f0, 0x4e40d0, 0x58acf0, 0x58b3e0, 0x594508, 0x59455c}
        equal = []
        for f in inventory['functions']:
            if f['entry'] not in entries:
                continue
            chunks = [official.read(lo,hi-lo) for lo,hi in f['ranges']]
            if any(x != nocd.read(lo,hi-lo) for x,(lo,hi) in zip(chunks,f['ranges'])):
                raise ValueError('Original owner-related body differs')
            equal.append(dict(entry=f['entry'], ranges=f['ranges'], sha256=hashlib.sha256(b''.join(chunks)).hexdigest()))
        if {f['entry'] for f in equal} != entries:
            raise ValueError('Missing original body boundary')
        # Reuse the independently reviewed full-file literal classifier. This
        # covers raw occurrences at every file offset, not only known code rows.
        import importlib.util
        spec = importlib.util.spec_from_file_location('owner_builds', ROOT/'tools/review-alternate-owner-builds.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        literals = module.inspect(official, [0x6e1f68,0x6e1f6c])
        if [len(r['sites']) for r in literals] != [5,2]:
            raise ValueError('Official pointer literal count changed')
        slots = []
        for slot in (0x6e1f68,0x6e1f6c,0x6e1f70,0x6e1f74,0x6e1f78):
            slots.append(dict(address=slot, initial=official.read(slot,4,True).hex(),
                              loader_zero_fill=slot >= official.section(slot)['address']+official.section(slot)['stored']))
        imported = {r['address']:r['symbol'] for r in official.imports()}
        if imported[0x5c50ac] != 'GetDriveTypeA':
            raise ValueError('Drive-type import witness changed')
        report.update(success=True, official_sha256=UPDATED, nocd_sha256=NOCD,
                      reconstruction=dict(record=receipt_path, sha256=evidence_digest(ROOT/receipt_path, root=ROOT)),
                      inventory=dict(record=inventory_path, sha256=evidence_digest(ROOT/inventory_path, root=ROOT)),
                      stored_sections_layout_identical=True, stored_section_difference_bytes=16,
                      stored_section_differences=differences, complete_equal_bodies=equal,
                      pointer_literals=literals, global_initialization=slots,
                      direct_literal_publishers=0, direct_literal_releases=0,
                      conclusion='Official1.100 contains the same original alternate initialization,display rebuild,graphics shutdown,canvas retirement and two copies. All stored section bytes are identical to NoCD except four conditional branches replaced by16NOP bytes in drive/media checking. No original alternate-owner body is removed in those reviewed bodies; the original owner remains unrecovered.',
                      limitations=['Literal-only review is not a computed/bulk/external writer proof.',
                                   'No original game execution,global dormancy,allocation,initial pixels,base/source relationship or retirement ownership is established.'])
    except Exception as error:
        report.update(success=False,error=str(error))
    finally:
        if before:
            subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
            report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        sha=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','stored_section_difference_bytes')}|dict(sha256=sha)))
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
