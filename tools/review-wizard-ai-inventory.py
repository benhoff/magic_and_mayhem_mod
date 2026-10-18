#!/usr/bin/env python3
"""Publish reviewed wizard spell-task entry bodies while retaining prior inventory unknowns."""
import argparse
import copy
import hashlib
from importlib.machinery import SourceFileLoader
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
EXPECTED = {4482800: ([[4482800, 4483096]], 'e698fa09276e8f62820e46408a51b9435aeefd0bb93155893002811807b27b74'), 4501104: ([[4501104, 4501630]], 'd93c0c1a1576dc005b1ba6fa12ce2ed7c0d2525ff5378cbdbda47f89122a44ca'), 4506976: ([[4506976, 4507288]], '3f7efa13d0fda701150537319654ab169915aec329902565e086654e29ff8a3b'), 4520400: ([[4520400, 4520688]], '5c4cc10a7f0259750067ee1cea0c01651595e0e89994099634386015673009a6'), 4522176: ([[4522176, 4522468]], '8434e68b50f6aaf410f6bd4738101326acf7e80b8fd1d3d75d0a3e0a116b68a4'), 4523904: ([[4523904, 4524196]], 'f498d1be6106867a9d73d74a0fae1e018bcb864790adb14ff5768331f8ccf152'), 4525792: ([[4525792, 4526084]], '46f3e4d31fcce89765cfbfafa4fcbb0b22153215773ede1b00cb4bbd4ae0b4f7'), 4569952: ([[4569952, 4570245]], 'b47ee21e0d55fb301ed9d311a2fb4270862d39ebdc52bfab5740ba1fd6a8c836'), 4575920: ([[4575920, 4576213]], '62221901db6e64672f09436038d7e6655b00ec640f60b2e384cbb7267976737d')}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    raw=a.candidate/'inventory.json';report=json.loads((a.candidate/'report.json').read_text())
    if not report['success'] or sha(raw)!=report['inventory_sha256']:raise ValueError('Candidate provenance mismatch')
    candidate=json.loads(raw.read_text());baseline=ROOT/'research/runtime/coverage/binary-nocd-owner-planner-20261016.json';old=json.loads(baseline.read_text())
    if candidate['source_sha256']!=PIN or old['source_sha256']!=PIN:raise ValueError('Unsupported original build')
    added=[]
    for entry,(ranges,digest) in EXPECTED.items():
        item=next(f for f in candidate['functions'] if f['entry']==entry)
        if item['ranges']!=ranges or item['sha256']!=digest:raise ValueError('Reviewed function body differs')
        if any(f['entry']==entry for f in old['functions']):raise ValueError('Already published')
        added.append(item)
    inventory=copy.deepcopy(old)
    inventory['functions'].extend(added)
    inventory['functions'].sort(key=lambda f:f['entry'])
    module=SourceFileLoader('focused_inventory',str(ROOT/'tools/inventory-binary.py')).load_module()
    inventory['unassigned_executable_ranges']=module.uncovered(inventory['sections'],inventory['functions'])
    inventory['provenance']={'baseline':str(baseline.relative_to(ROOT)),'baseline_sha256':sha(baseline),'candidate':str(raw.resolve()),'candidate_sha256':sha(raw),
        'sources':{'tools/review-wizard-ai-inventory.py':sha(Path(__file__)),'tools/inventory-binary.py':sha(ROOT/'tools/inventory-binary.py')},
        'scope':'Focused discovery merge: nine pinned wizard task entry bodies, including Cure and Magic Sphere, reviewed against read-only disassembly and vtable/factory references. Preserve prior functions, flows, imports and classifications. New-function flow attribution remains pending in the raw export; unrelated names and55a650 broadening are not adopted.'}
    target=a.output/'inventory.json';target.write_text(json.dumps(inventory,separators=(',',':'))+'\n')
    review={'success':True,'inventory_sha256':sha(target),'baseline_sha256':sha(baseline),'candidate_sha256':sha(raw),'added':added,
        'unaccepted_function_changes':report['changed'],'unaccepted_removals':report['removed'],
        'reason':'Reviewed entrypoint bodies, return boundaries and virtual-table references agree with retained wizard exports. Five function changes only rename known entries;55a650 changes body boundaries and remains unresolved. Preserve all old flow/dispatch unknowns; retain raw discovery for new-flow attribution. New bodies establish discovery boundaries, not whole-function semantic recovery.',
        'scope':'Discovery accounting only; no semantic validation or past gate pass follows from this merge.'}
    (a.output/'review.json').write_text(json.dumps(review,indent=2)+'\n')
    print(json.dumps({'functions':len(inventory['functions']),'unassigned_ranges':len(inventory['unassigned_executable_ranges']),'unaccepted_function_changes':len(report['changed'])}))
if __name__=='__main__':main()
