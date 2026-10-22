#!/usr/bin/env python3
"""Publish the reviewed original particle RNG static constructor, retaining all previous discoveries."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import copy
import hashlib
from importlib.machinery import SourceFileLoader
import json
from pathlib import Path
import subprocess
from original_x86 import Decoder,PE32
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
EXPECTED={4975616: ([[4975616, 4975643]], '1a9c5320419ab434fd612216b5717ab8fca1ecf6106f512ad31b8a7d2fa92267')}
def sha(path):return hashlib.sha256(evidence_bytes(path)).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--candidate',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new retained output')
    out.mkdir(parents=True);candidate_path=args.candidate.resolve()/'inventory.json';candidate=json.loads(evidence_text(candidate_path));export=json.loads(evidence_text(args.candidate/'report.json'))
    baseline=ROOT/'research/runtime/coverage/binary-nocd-all-rendering-20261020.json';old=json.loads(evidence_text(baseline))
    if not export['success'] or sha(candidate_path)!=export['artifacts']['inventory.json'] or old['source_sha256']!=PIN or candidate['source_sha256']!=PIN:raise ValueError('Inventory provenance mismatch')
    sources={n:sha(ROOT/n) for n in ['tools/review-original-rng-inventory.py','tools/original_x86.py','tools/inventory-binary.py',baseline.relative_to(ROOT).as_posix()]}
    result=dict(schema=1,success=False,static_only=True,sources=sources,baseline=baseline.relative_to(ROOT).as_posix(),baseline_sha256=sha(baseline),candidate_sha256=sha(candidate_path),all_flow_or_semantic_closure=False)
    (out/'prospective.json').write_text(json.dumps(result,indent=2)+'\n');before=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        image=PE32(evidence_bytes(ROOT/'original/Arcane_Nocd/Chaos.exe'));decoder=Decoder()
        if image.sha256!=PIN:raise ValueError('Original changed')
        added=[];reviews=[];seen={f['entry'] for f in old['functions']}
        for entry,(ranges,digest) in EXPECTED.items():
            item=next(f for f in candidate['functions'] if f['entry']==entry)
            if entry in seen or item['ranges']!=ranges or item['sha256']!=digest:raise ValueError('Reviewed body differs')
            body_hash=hashlib.sha256();instructions=[]
            for lo,hi in ranges:
                for function in old['functions']:
                    if any(max(lo,a)<min(hi,b) for a,b in function['ranges']):raise ValueError('Overlapping historical function')
                body_hash.update(image.read(lo,hi-lo));at=lo
                while at<hi:
                    row=decoder.decode(image.read(at,min(15,hi-at)),at);instructions.append(row);at+=row['size']
                if at!=hi:raise ValueError('Misaligned reviewed range')
            if body_hash.hexdigest()!=digest:raise ValueError('Original function bytes differ')
            c=args.candidate/'c'/f'{entry:08x}.c'
            if not c.is_file():raise ValueError('Missing reviewed decompilation')
            reviews.append(dict(entry=entry,ranges=ranges,sha256=digest,decompiled_sha256=sha(c),instructions=len(instructions),last_instruction=instructions[-1],
                reason='Entry-aligned original bytes and disassembly/decompilation agree with the selected particle RNG CRT static-constructor root4bec00. New body discovery only; semantic and indirect-flow closure remain independently accounted.'))
            added.append(item)
        inventory=copy.deepcopy(old);inventory['functions'].extend(added);inventory['functions'].sort(key=lambda f:f['entry'])
        module=SourceFileLoader('focused_inventory',str(ROOT/'tools/inventory-binary.py')).load_module();inventory['unassigned_executable_ranges']=module.uncovered(inventory['sections'],inventory['functions'])
        inventory['provenance']=dict(baseline=baseline.relative_to(ROOT).as_posix(),baseline_sha256=sha(baseline),candidate_sha256=sha(candidate_path),sources=sources,
            scope='Reviewed original particle RNG static-constructor4bec00 body. Preserve all prior functions, imports, flows, classifications and unresolved indirects. Reject unrelated55a650 range broadening; no semantic or all-writer closure. New-function flow attribution remains pending in the retained raw export.')
        target=out/'inventory.json';target.write_text(json.dumps(inventory,separators=(',',':'))+'\n')
        result.update(success=True,original_sha256=PIN,added_functions=reviews,unaccepted_function_changes=export['changed_functions'],
            functions=len(inventory['functions']),unassigned_ranges=len(inventory['unassigned_executable_ranges']),inventory_sha256=sha(target),
            historical_flows_preserved=True,reason='The added4bec00range is original-byte/hash checked and disjoint from every historical function. Adopt only those bodies; retain unrelated changed55a650 ranges and all old unresolved flow ownership unchanged.')
    except (OSError,ValueError,subprocess.SubprocessError) as error:result['error']=str(error)
    finally:
        if before:
            try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);result['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:result.update(success=False,preservation_error=str(error))
        result['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());result['success'] &= result['sources_stable'];(out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(out/'report.json');return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
