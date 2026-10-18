#!/usr/bin/env python3
"""Publish only the verified navigation comparator from a retained discovery re-export."""
import argparse
import copy
import hashlib
from importlib.machinery import SourceFileLoader
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    raw=a.candidate/'inventory.json';report=json.loads((a.candidate/'report.json').read_text())
    if not report['success'] or sha(raw)!=report['inventory_sha256']:raise ValueError('Candidate provenance mismatch')
    candidate=json.loads(raw.read_text());baseline=ROOT/'research/runtime/coverage/binary-nocd.json';old=json.loads(baseline.read_text())
    if candidate['source_sha256']!=PIN or old['source_sha256']!=PIN:raise ValueError('Unsupported original build')
    comparator=next(f for f in candidate['functions'] if f['entry']==0x53d6f0)
    if comparator['ranges']!=[[0x53d6f0,0x53d707]] or comparator['sha256']!='a964deca08cd1425a76c14075c5635fe95c29da889f3efc069341e8f26ebdd1c':raise ValueError('Comparator body differs')
    if any(f['entry']==comparator['entry'] for f in old['functions']):raise ValueError('Comparator already published')
    inventory=copy.deepcopy(old);inventory['functions'].append(comparator);inventory['functions'].sort(key=lambda f:f['entry'])
    module=SourceFileLoader('focused_inventory',str(ROOT/'tools/inventory-binary.py')).load_module()
    inventory['unassigned_executable_ranges']=module.uncovered(inventory['sections'],inventory['functions'])
    inventory['provenance']={'baseline':str(baseline.relative_to(ROOT)),'baseline_sha256':sha(baseline),'candidate':str(raw.resolve()),'candidate_sha256':sha(raw),
        'sources':{'tools/review-regional-graph-inventory.py':sha(Path(__file__)),'tools/inventory-binary.py':sha(ROOT/'tools/inventory-binary.py')},
        'scope':'Focused reviewed discovery merge: add only verified23-byte comparator53d6f0..53d706. Retain all prior functions/flows/imports and unknown classifications. The raw re-export and unrelated55a650 broadening remain separate pending discovery evidence.'}
    target=a.output/'inventory.json';target.write_text(json.dumps(inventory,separators=(',',':'))+'\n')
    review={'success':True,'inventory_sha256':sha(target),'baseline_sha256':sha(baseline),'candidate_sha256':sha(raw),'added':[comparator],
        'unaccepted_function_changes':report['changed'],'unaccepted_removals':report['removed'],
        'reason':'Comparator body and incoming qsort data reference match the pinned static export; whole original qsort execution is independently compared with duplicate keys and full capacity. The unrelated55a650 contiguous-body broadening is not attributed to regional construction. Retain it and its flow delta in the raw candidate; no prior unknown range/flow is removed except the confirmed23-byte comparator range.',
        'scope':'Discovery accounting only; no semantic validation or past gate pass follows from this merge.'}
    (a.output/'review.json').write_text(json.dumps(review,indent=2)+'\n')
    print(json.dumps({'functions':len(inventory['functions']),'unassigned_ranges':len(inventory['unassigned_executable_ranges']),'unaccepted_function_changes':len(report['changed'])}))
if __name__=='__main__':main()
