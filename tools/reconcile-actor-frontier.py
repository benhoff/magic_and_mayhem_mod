#!/usr/bin/env python3
"""Reconcile the twelve concrete providers without hiding their lower frontier."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from coverage_evidence import read_json, evidence_digest, write_report

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--comparison',type=Path,required=True)
    parser.add_argument('--inspection',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    comparison=read_json(args.comparison,root=ROOT)
    if not comparison['success'] or not comparison['sources_stable']:raise ValueError('Require successful current comparison')
    for p,sha in comparison['sources'].items():
        if evidence_digest(ROOT/p,root=ROOT)!=sha:raise ValueError('Comparison source drift '+p)
    oldpath=ROOT/'research/runtime/actor-session-source-results-20261022/reconciliation-reviewed.json'
    old=read_json(oldpath,root=ROOT)
    header='reconstruction/rendering/actor_frontier_providers.hpp'
    added={int(v,16) for v in re.findall(r'case (0x[0-9a-f]+):',(ROOT/header).read_text())}
    if len(added)!=12 or set(comparison['roots'])!=added:raise ValueError('Twelve-root matrix differs')
    routes={v['address']:v['model'] for v in old['concrete_routes']}
    if len(routes)!=171 or added&routes.keys():raise ValueError('Historical route set differs')
    routes.update({v:'FrontierProviders' for v in added})
    leads=[]
    for row in old['historical_semantic_leads']:
        value=dict(row)
        if value['address'] in added:
            value.pop('provider',None);value.update(disposition='concrete_route',model='FrontierProviders',domain='Declared owned-store and named lower-endpoint domain only')
        leads.append(value)
    if len(leads)!=43 or any(v['disposition'] in {'supplied_provider_remaining','unresolved_composition_remaining'} for v in leads):raise ValueError('Historical lead reconciliation incomplete')
    inspection=read_json(args.inspection/'report.json',root=ROOT)
    if not inspection['success'] or inspection['original_sha256']!=comparison['original_sha256']:raise ValueError('Inspection build differs')
    frontier=read_json(args.inspection/'call-frontier.json',root=ROOT)
    supplied={v['address']:v for v in comparison['supplied_providers']}
    # Keep every inspected site even if the exporter does not carry caller keys.
    result=dict(schema=1,success=True,static_review_only=True,
        scope='183 public routes; all43historical leads retained as24concrete routes and19embedded scoped helpers. Lower named suppliers, incomplete indirect targets and original-world cadence remain open.',
        sources={header:evidence_digest(ROOT/header,root=ROOT),'tools/reconcile-actor-frontier.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},
        original_sha256=comparison['original_sha256'],
        inputs={str(p):evidence_digest(p,root=ROOT) for p in [args.comparison,args.inspection/'report.json',args.inspection/'call-frontier.json',oldpath]},
        concrete_routes=[dict(address=a,model=m) for a,m in sorted(routes.items())],
        added_routes=sorted(added),historical_semantic_leads=leads,
        supplied_providers=comparison['supplied_providers'],
        inspected_call_sites=[dict(v,concrete_route=routes.get(v.get('target')),supplied_provider=supplied.get(v.get('target'))) for v in frontier],
        unresolved_indirect_sites=[v for v in frontier if 'target' not in v],
        historical_leads_closed_within_scope=True,recursive_provider_closure=False,
        whole_original_session=False,live_replacement=False,indirect_target_completeness=False)
    write_report(ROOT,args.output,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(routes=len(routes),historical_leads=len(leads),concrete_leads=sum(v['disposition']=='concrete_route' for v in leads),embedded_helpers=sum(v['disposition']=='embedded_scoped_helper' for v in leads),supplied_endpoints=len(supplied))))

if __name__=='__main__':main()
