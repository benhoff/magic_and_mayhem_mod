#!/usr/bin/env python3
"""Reconcile font providers while retaining named lower boundaries."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from coverage_evidence import read_json, evidence_digest, write_report
ROOT = Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--comparison',type=Path,required=True)
    p.add_argument('--inspection',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    args=p.parse_args()
    comparison=read_json(args.comparison,root=ROOT)
    if not comparison['success'] or not comparison['sources_stable']:raise ValueError('Require successful current comparison')
    for name,sha in comparison['sources'].items():
        if evidence_digest(ROOT/name,root=ROOT)!=sha:raise ValueError('Comparison drift '+name)
    oldpath=ROOT/'research/runtime/actor-profile-palette-results-20261022/reconciliation.json'
    old=read_json(oldpath,root=ROOT)
    header='reconstruction/rendering/actor_font_providers.hpp'
    added={int(v,16) for v in re.findall(r'case (0x[0-9a-f]+):',(ROOT/header).read_text())}
    if len(added)!=17 or set(comparison['roots'])!=added:raise ValueError('Seventeen-root matrix differs')
    routes={v['address']:v['model'] for v in old['concrete_routes']}
    if len(routes)!=198 or added&routes.keys():raise ValueError('Historical routes differ')
    routes.update({v:'FontProviders' for v in added})
    inspection=read_json(args.inspection/'inspection-report.json',root=ROOT)
    if not inspection['success'] or inspection['original_sha256']!=comparison['original_sha256']:raise ValueError('Inspection build differs')
    frontier=read_json(args.inspection/'inspection-call-frontier.json',root=ROOT)
    supplied={v['address']:v for v in comparison['supplied_providers']}
    retired=sorted({v['address'] for v in old['supplied_providers']}&added)
    result=dict(schema=1,success=True,static_review_only=True,
        scope='215 public routes including17 font roots. Retain43 historical leads as24 concrete routes and19 embedded scoped helpers. Lower suppliers, incomplete indirect targets and original planner cadence remain open.',
        sources={header:evidence_digest(ROOT/header,root=ROOT),'tools/reconcile-actor-font.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},
        original_sha256=comparison['original_sha256'],
        inputs={str(q):evidence_digest(q,root=ROOT) for q in [args.comparison,args.inspection/'inspection-report.json',args.inspection/'inspection-call-frontier.json',oldpath]},
        concrete_routes=[dict(address=a,model=m) for a,m in sorted(routes.items())],
        added_routes=sorted(added),previously_supplied_now_concrete=retired,
        historical_semantic_leads=old['historical_semantic_leads'],supplied_providers=comparison['supplied_providers'],
        inspected_call_sites=[dict(v,concrete_route=routes.get(v.get('target')),supplied_provider=supplied.get(v.get('target'))) for v in frontier],
        unresolved_indirect_sites=[v for v in frontier if 'target' not in v],
        recursive_provider_closure=False,whole_original_session=False,live_replacement=False,indirect_target_completeness=False)
    write_report(ROOT,args.output,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(routes=len(routes),added_routes=len(added),previous_suppliers_replaced=len(retired),supplied_endpoints=len(supplied))))
if __name__=='__main__':main()
