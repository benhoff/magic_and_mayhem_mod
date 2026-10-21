#!/usr/bin/env python3
"""Review only the newly found 59455c x87 copy body; preserve historical discovery."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from original_x86 import PE32, Decoder
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
DIGEST='5fa7b04519567f0cbefd9ec6550daaf01fa00e667914cb288334da3d56cc5585'
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output')
    out.mkdir(parents=True)
    baseline=ROOT/'research/runtime/coverage/binary-nocd-all-rendering-complete-entries-20261020.json'
    paths=['tools/review-alternate-canvas-inventory.py','tools/original_x86.py','tools/inventory-binary.py',baseline.relative_to(ROOT).as_posix()]
    sources={n:sha(ROOT/n) for n in paths}
    report=dict(schema=1,success=False,static_only=True,sources=sources,original_sha256=PIN,semantic_closure_proved=False)
    before=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        old=json.loads(baseline.read_text());export=json.loads((a.candidate/'report.json').read_text())
        candidate_path=a.candidate/'inventory.json';candidate=json.loads(candidate_path.read_text())
        if image.sha256!=PIN or not export['success'] or sha(candidate_path)!=export['artifacts']['inventory.json']:
            raise ValueError('Unadmitted export/original')
        if old['source_sha256']!=PIN or candidate['source_sha256']!=PIN:raise ValueError('Inventory build differs')
        new=next(f for f in candidate['functions'] if f['entry']==0x59455c)
        if new['ranges']!=[[0x59455c,0x5945ec]] or new['sha256']!=DIGEST or hashlib.sha256(image.read(0x59455c,144)).hexdigest()!=DIGEST:
            raise ValueError('Reviewed body differs')
        if any(max(lo,0x59455c)<min(hi,0x5945ec) for f in old['functions'] for lo,hi in f['ranges']):
            raise ValueError('Historical function overlap')
        decoder=Decoder();at=0x59455c;rows=[]
        try:
            while at<0x5945ec:
                row=decoder.decode(image.read(at,min(15,0x5945ec-at)),at);rows.append(row);at+=row['size']
        finally:decoder.close()
        if at!=0x5945ec or rows[-1]['mnemonic']!='ret':raise ValueError('Boundary differs')
        inventory=copy.deepcopy(old);inventory['functions'].append(new);inventory['functions'].sort(key=lambda f:f['entry'])
        spec=importlib.util.spec_from_file_location('alternate_inventory',ROOT/'tools/inventory-binary.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        inventory['unassigned_executable_ranges']=module.uncovered(inventory['sections'],inventory['functions'])
        inventory['provenance']=dict(baseline=baseline.relative_to(ROOT).as_posix(),baseline_sha256=sha(baseline),
            candidate_sha256=sha(candidate_path),scope='Add only disjoint hash/byte/range/decompiler reviewed 59455c. Preserve prior functions, imports, flows and unresolved indirects. No original caller/reachability, allocation or writer closure.')
        (out/'inventory.json').write_text(json.dumps(inventory,separators=(',',':'))+'\n')
        report.update(success=True,added_function=new,instructions=rows,functions=len(inventory['functions']),
            historical_flows_preserved=True,inventory_sha256=sha(out/'inventory.json'),
            candidate_sha256=sha(candidate_path),decompiled_sha256=sha(a.candidate/'c/0059455c.c'),
            unaccepted_function_changes=export['changed_functions'],
            direct_flow_review='Two local backedges 5945c8->594579 and 5945e3->594573; return at5945eb. No call or indirect edge in this helper; incoming caller unresolved.')
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if before:
            try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json')
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
