#!/usr/bin/env python3
"""Publish only reviewed rendering/particle discovery bodies, retaining old unknowns."""
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
EXPECTED={5529024: ([[5529024, 5529174]], '17d144cc63f201eb0ea59a42fc70b13e0d7c42d5a2314bf696d707215ca7bb8c'), 5529888: ([[5529888, 5530153]], '9549515971b20cb17570a765bf13f89978882a37756101f318becc2016f0822c'), 5530160: ([[5530160, 5530211]], '769246446698447db50bbae2ef3a8d37d99ff068907ee5158545fbe84b9d798d'), 5530448: ([[5530448, 5530943]], '72c5462059c670c0490c8907c9fc36d98fcbf24c314da1e0a9f7de475d1124c3'), 5530944: ([[5530944, 5531010]], '776a735e612f21df78240a07761f5ae3a6cf6931a49bf73c9d09aa301733b413'), 5531024: ([[5531024, 5531173]], 'afb5df84a9bac623688d835eb867cde3ffe2d7e45593b9d91f48bb23d60f32ce'), 5531808: ([[5531808, 5532697]], 'd48782ae912a14b5af72793ef3effbd6edf9e20bf9800f88f9d8a94d345f4abd'), 5534384: ([[5534384, 5534394]], 'cc716743f43049cce5a113c1b4e0624269ba3ee2ff8eb0adba58358c87889b04'), 5534576: ([[5534576, 5534586]], '9fb1b26acdad7baa4595b3283d3b02fd932cfd672e850e83e6e4e2dc4dcc0854'), 5539904: ([[5539904, 5539921]], 'b7c728564c4abb20749343b2c2fbd480fa249254b56d994a942d20b3b9dc66b4'), 5540800: ([[5540800, 5542320]], 'e0ea6d8b10a7248272da2f53c5e81a88f41b58a797379aad56defe922d7a46d6'), 5542576: ([[5542576, 5542773]], '7e95e36a43a94a2e37262ca4906d9525d16d2648ab871bc34d76ea8fd8339a3c'), 5542784: ([[5542784, 5543559]], '9f4444b506160cca7c88ee6bff2008fa95c11b548c7c1e603ea4216777395e0a'), 5543728: ([[5543728, 5543957]], 'c37628191cf937c1b6bdef1a2fe90fd6e5540eb185c2909fe8e37e256cdf4a2a'), 5543968: ([[5543968, 5544191]], '595145bd999e21d44ec4f02ea58dc2484ed185e08592f26c8713568aaae191fd'), 5544864: ([[5544864, 5545093]], '90738588b64a862d8bf46aa076df98a24765ddf3d6ad664e03780934850e1c83'), 5545104: ([[5545104, 5545327]], '595145bd999e21d44ec4f02ea58dc2484ed185e08592f26c8713568aaae191fd'), 5546304: ([[5546304, 5547168]], 'b5851b4419a32cafd226c1ee7707656a075e90e4e880e419ae62924a041ad5e1'), 5547168: ([[5547168, 5547502]], '40698894cafdcf7f9ec807bb4a7f83840edfd3ebd67fed1f98d0b1058c1bdb07'), 5548512: ([[5548512, 5548518]], 'e36623525b7c35b277173e56616b2d865cf6515aa2be1d12d6814ef725cecffb'), 5548528: ([[5548528, 5548534]], 'e36623525b7c35b277173e56616b2d865cf6515aa2be1d12d6814ef725cecffb'), 5548544: ([[5548544, 5548550]], '4a5d3fbc43f9c76ffccf7f1b906233d5d14bbe12798031e3d163d089ac406cef'), 5548560: ([[5548560, 5548592]], '631a20081477b0fa1d40ebea9fc7211019d425e78fe9af284bf724e0d23dad93'), 5548640: ([[5548640, 5548644]], '3b2eded1bf6b0f7390191ad1371a956a736f9d18c2a7de0db7e90886555b69e1'), 5548656: ([[5548656, 5548660]], '7e6bda0a031f94dbe8477fe3ce3ba2b7e3d761a050d55625bc7d92212670e60c'), 5548672: ([[5548672, 5548704]], '078149b81469fa39db292ce80863ba2941e79d4e6d1ec414e6dc682d7d8b39c5'), 5548704: ([[5548704, 5548725]], 'bf1bb4ca41555493f5c05e73a62392aa6cc7b44764049a719bea521bb286ce43'), 5548736: ([[5548736, 5548740]], 'c8cc0b90c656bd6a43ec1c5684c1a494142d2fce4803153d9b1a0944cad7e281'), 5548752: ([[5548752, 5548756]], '3b2eded1bf6b0f7390191ad1371a956a736f9d18c2a7de0db7e90886555b69e1'), 5548768: ([[5548768, 5548789]], 'f51865a4156a5ee3c477c2d3f955044e61c75c8e5f7103c8841351b27697c304'), 5548800: ([[5548800, 5548825]], '591be547dbf6026e039177920f5da4619548d2f879c08535cc3f2fca502a1767'), 5548832: ([[5548832, 5548836]], '07ad3a50b1c271e07fb57fb47d2f0e77f3342d41bfa410990cb1320a45035fb2'), 5548848: ([[5548848, 5548852]], 'e27121b8816e0aef2ed29676899d26e79f66af7e5ee5aed902be7208122bb635'), 5548896: ([[5548896, 5548917]], '0960a7a7d8b63cff6de4fe133e96acca4d5198c535a33618f37e685377077967'), 5548928: ([[5548928, 5549047]], '18bde9c9ff004f80433ba3fc9f05d7cb9ac3fd7ad1ecfbabf2fce7ee62d0be6b'), 5549056: ([[5549056, 5549066]], '7d0b43f517c217eac0078b4eb0a88980848412c54ec71833cb01589b7b593441'), 5549072: ([[5549072, 5549103]], 'c172100b49381c52f777ac61f4edc9eefbec99493bada787cb30dc160ccd42e3'), 5549104: ([[5549104, 5549110]], 'f0d994779fd80a5f7b93900d3fd895c8f58f622faa06107f7500f1da92444bef'), 5549120: ([[5549120, 5549126]], 'f0d994779fd80a5f7b93900d3fd895c8f58f622faa06107f7500f1da92444bef'), 5549136: ([[5549136, 5549181]], '1cfce8e3cc651ead566e6e07688ba775a587a768fcba8f20682d46d2b5bfaf19'), 5549184: ([[5549184, 5549190]], '6918347da097c39b18e2530fa730ba6657ede7739471f8f305f642e775667a96'), 5549200: ([[5549200, 5549245]], '1cfce8e3cc651ead566e6e07688ba775a587a768fcba8f20682d46d2b5bfaf19'), 5549248: ([[5549248, 5549254]], '05c190004e38e967cfd490d9918fbd98364d400769db9030c4720d1cc56fbedf'), 5549264: ([[5549264, 5549273]], '15ce82c6841a022c0ba3c850e078f75577770be8da683b19b5eb8986e8393841'), 5549280: ([[5549280, 5549289]], '15ce82c6841a022c0ba3c850e078f75577770be8da683b19b5eb8986e8393841'), 5549296: ([[5549296, 5549317]], '28bb5a03f2aa288c66b8e5a12b6650260cd9504f9007bfaac974c9982d3c1e12'), 5549328: ([[5549328, 5549342]], 'b9acfeb26d2a6d138d8fa58f10e85ee880a11280dcc946c2f7ae659c19213c17'), 5549344: ([[5549344, 5549358]], '2715a74a0f3d9521bda9838c448bcc7bd4ffe4468a2e23bb1b37210d5f048e17'), 5549360: ([[5549360, 5549392]], 'bbdbe79e7d9b2afe9e1f88ab7a933aa0bc2a7c5f450eed8e7b2177cf81752c80'), 5759408: ([[5759408, 5760308]], '9bd0f959b3fb7991983d3315340e99f52f4e13bc77405d89f5ba726b8016055a'), 5773104: ([[5773104, 5774038]], '4c74e1192a5c3a0a471e436ca575f1f5f3be3e35cd51a11905e2fc83b5b37553'), 5774048: ([[5774048, 5775040]], 'a2a7e7fc2d9ac667abeaf7dd09f1f381870ed1847b312e2c6b2033d8ae9e08e7')}
def sha(path):return hashlib.sha256(evidence_bytes(path)).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--candidate',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new retained output')
    out.mkdir(parents=True);candidate_path=args.candidate.resolve()/'inventory.json';candidate=json.loads(evidence_text(candidate_path));export=json.loads(evidence_text(args.candidate/'report.json'))
    baseline=ROOT/'research/runtime/coverage/binary-nocd-wizard-ai-20261017.json';old=json.loads(evidence_text(baseline))
    if not export['success'] or sha(candidate_path)!=export['artifacts']['inventory.json'] or old['source_sha256']!=PIN or candidate['source_sha256']!=PIN:raise ValueError('Inventory provenance mismatch')
    sources={n:sha(ROOT/n) for n in ['tools/review-all-original-inventory.py','tools/original_x86.py','tools/inventory-binary.py',baseline.relative_to(ROOT).as_posix()]}
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
                reason='Entry-aligned original bytes and disassembly/decompilation agree with the selected particle vtable/default-constructor/update/draw/accessor or explicit alternate-buffer reader root. New body discovery only; semantic and indirect-flow closure remain independently accounted.'))
            added.append(item)
        inventory=copy.deepcopy(old);inventory['functions'].extend(added);inventory['functions'].sort(key=lambda f:f['entry'])
        module=SourceFileLoader('focused_inventory',str(ROOT/'tools/inventory-binary.py')).load_module();inventory['unassigned_executable_ranges']=module.uncovered(inventory['sections'],inventory['functions'])
        inventory['provenance']=dict(baseline=baseline.relative_to(ROOT).as_posix(),baseline_sha256=sha(baseline),candidate_sha256=sha(candidate_path),sources=sources,
            scope='52reviewed original particle/alternate-reader entry bodies. Preserve all prior functions, imports, flows, classifications and unresolved indirects. Reject unrelated55a650 range broadening; no semantic or all-writer closure. New-function flow attribution remains pending in the retained raw export.')
        target=out/'inventory.json';target.write_text(json.dumps(inventory,separators=(',',':'))+'\n')
        result.update(success=True,original_sha256=PIN,added_functions=reviews,unaccepted_function_changes=export['changed_functions'],
            functions=len(inventory['functions']),unassigned_ranges=len(inventory['unassigned_executable_ranges']),inventory_sha256=sha(target),
            historical_flows_preserved=True,reason='All52added ranges are original-byte/hash checked and disjoint from every historical function. Adopt only those bodies; retain unrelated changed55a650 ranges and all old unresolved flow ownership unchanged.')
    except (OSError,ValueError,subprocess.SubprocessError) as error:result['error']=str(error)
    finally:
        if before:
            try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);result['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:result.update(success=False,preservation_error=str(error))
        result['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());result['success'] &= result['sources_stable'];(out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(out/'report.json');return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
