#!/usr/bin/env python3
"""Retain explicit default/software storage proof without changing legacy EP04 admission."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_storage_acceptance import progress
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--execution',nargs=3,action='append',required=True,metavar=('CONFIGURATION','ACQUISITION','COMPARISON'));p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith('research/runtime/storage-main-stall-results-20261101/') or '..' in Path(a.report).parts or (ROOT/a.report).exists():p.error('New owned immutable report required')
    with Reader(ROOT) as reader:
        groups=[(cfg,{k:dict(record=n,sha256=reader.digest(n)) for k,n in [('acquisition',acq),('comparison',cmp)]}) for cfg,acq,cmp in a.execution]
        result=progress(reader.json('docs/native-rendering-closure.json'),reader,groups)
        result['sources'].update({n:reader.digest(n) for n in ['tools/rendering_storage_acceptance.py','tools/join-rendering-storage-acceptance.py','docs/native-rendering-closure.json','tools/rendering_normal_acceptance.py','tools/rendering_acceptance.py','tools/coverage_claims.py']})
        h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,passed=result['passed_requirements'],pending=result['pending_requirements'],record=a.report,sha256=h)))


if __name__=='__main__':main()
