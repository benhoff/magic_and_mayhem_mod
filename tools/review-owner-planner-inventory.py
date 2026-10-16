#!/usr/bin/env python3
"""Publish reviewed owner-task entry bodies while retaining prior inventory unknowns."""
import argparse
import copy
import hashlib
from importlib.machinery import SourceFileLoader
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
EXPECTED = {4281920: ([[4281920, 4281957]], '7f5b6b3cdf5243524eca377071d09148504a2d2767e59a5c46aac0dbac1407ff'), 4281968: ([[4281968, 4282014]], '5653c05baed56649e66128f710ebaf26735bce24adda52bebaa9ebf404d9659f'), 4299008: ([[4299008, 4299013]], 'd94cc1c82e8ea202d242e9bd83dcad4b047b4cc733a571a749d0f2613aae60ee'), 4350384: ([[4350384, 4350783]], '4e7bbdf109700db6ca72eca5e5143af8efc365642cae4f3388764a256e4f24be'), 4398048: ([[4398048, 4398290]], '9ceab7965f9dd4ef3e962f3cadec9e0f2265b98c5f3c74b1ef1bda1fadf2a69a'), 4398304: ([[4398304, 4401356]], 'a21dc07a6d32823edca09c4da31dfbdab18ccc9f293cc46edcdf836631b3a41a'), 4403136: ([[4403136, 4403378]], '2b8af578b8ee1cff46c9136b7c176d9c66b10dab8349a50fdd117dd3f38c6f69'), 4403392: ([[4403392, 4406428]], 'ef3f5704404fe76d877920f7be77f78c8448e41cefe3817f13bfd7c071afc70f'), 4422096: ([[4422096, 4426184]], '3f238cf327d28159330c076769f13de1cccdd4abd77c157195e6e275d08607b2'), 4426240: ([[4426240, 4426612]], '7ce91be91b65afe5caf6029cea3d171eecd1e815569cf0df1d76408c6bc21c5c'), 4431488: ([[4431488, 4431865]], 'a48c799514ac4c2ec3101d8173a17f897fcf41a26a0a553f4a28428a3bc62d9a'), 4431872: ([[4431872, 4436576]], '2936cf0fee067e4fcf680f3c39d04df185eca7add9cafd937b17a5d42192a474'), 4436624: ([[4436624, 4436957]], '876329ecbea1a47cec9e8ac0f2b48221c170a40b756f49d5fd2c7c6d813b5990'), 4437184: ([[4437184, 4440963]], '1736bf5bc8bdc4671702c46b9ee7f3ca8a380156023b47254133e105f96204cc'), 4441024: ([[4441024, 4441590]], '5ad77eb9bd3a8f47c6e5e451c2e97c3cd30e79f20d3f206a7ae503c98eb8e7af'), 4441600: ([[4441600, 4441601]], 'ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e')}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    raw=a.candidate/'inventory.json';report=json.loads((a.candidate/'report.json').read_text())
    if not report['success'] or sha(raw)!=report['inventory_sha256']:raise ValueError('Candidate provenance mismatch')
    candidate=json.loads(raw.read_text());baseline=ROOT/'research/runtime/coverage/binary-nocd-regional-graph-20261015.json';old=json.loads(baseline.read_text())
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
        'sources':{'tools/review-owner-planner-inventory.py':sha(Path(__file__)),'tools/inventory-binary.py':sha(ROOT/'tools/inventory-binary.py')},
        'scope':'Focused discovery merge: sixteen pinned entry bodies, including fallback and task6/7/10/12/13 providers, reviewed against read-only disassembly and vtable/factory references. Preserve prior functions, flows, imports and classifications. New-function flow attribution remains pending in the raw export; unrelated names and55a650 broadening are not adopted.'}
    target=a.output/'inventory.json';target.write_text(json.dumps(inventory,separators=(',',':'))+'\n')
    review={'success':True,'inventory_sha256':sha(target),'baseline_sha256':sha(baseline),'candidate_sha256':sha(raw),'added':added,
        'unaccepted_function_changes':report['changed'],'unaccepted_removals':report['removed'],
        'reason':'Reviewed entrypoint bodies, return boundaries and virtual-table references agree with retained planner exports. Five function changes only rename known entries;55a650 changes body boundaries and remains unresolved. Preserve all old flow/dispatch unknowns; retain raw discovery for new-flow attribution. New bodies establish discovery boundaries, not whole-function semantic recovery.',
        'scope':'Discovery accounting only; no semantic validation or past gate pass follows from this merge.'}
    (a.output/'review.json').write_text(json.dumps(review,indent=2)+'\n')
    print(json.dumps({'functions':len(inventory['functions']),'unassigned_ranges':len(inventory['unassigned_executable_ranges']),'unaccepted_function_changes':len(report['changed'])}))
if __name__=='__main__':main()
