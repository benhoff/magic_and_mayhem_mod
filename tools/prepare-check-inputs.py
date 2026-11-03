#!/usr/bin/env python3
"""Provision shared campaign inputs once, with isolated pooled mutable loans."""
import argparse
import hashlib
import json
from pathlib import Path
from check_inputs import provision
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--from-checkout',type=Path,required=True)
    p.add_argument('--catalog-only',action='store_true',help='Preserve existing local input directories; campaign tools use the verified catalog explicitly')
    p.add_argument('--report',required=True)
    a=p.parse_args()
    if not a.report.startswith('research/runtime/') or '..' in Path(a.report).parts or (ROOT/a.report).exists():
        p.error('A new immutable research report is required')
    source=a.from_checkout.resolve()/'working'
    names=['tools/check_inputs.py','tools/prepare-check-inputs.py','tools/experiment_environment.py','tools/directplay_dx6_launch.py']
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    result=dict(schema=1,kind='shared_check_inputs',success=False,sources=sources)
    try:
        result['inputs']=provision(ROOT,dict(game=source/'game-nocd',assets=source/'assets/native-cd-audio-20261012',prefix=source/'fixtures/prefix-seed'),aliases=not a.catalog_only)
        result['catalog_only']=a.catalog_only
        result['success']=True
    except Exception as e:result['error']=str(e)
    result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
    result['success'] &= result['sources_stable']
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha)))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
