#!/usr/bin/env python3
"""Reconcile the emitter constructor and manager ownership and preserve the remaining actor frontier."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from coverage_evidence import read_json,evidence_digest,write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['comparison','inspection','output']:p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();comparison=read_json(a.comparison,root=ROOT);inspection=read_json(a.inspection,root=ROOT)
 if not comparison['success'] or not comparison['sources_stable']:raise ValueError('Require successful stable comparison')
 for n,h in comparison['sources'].items():
  if evidence_digest(ROOT/n,root=ROOT)!=h:raise ValueError('Comparison source drift '+n)
 if not inspection['success'] or inspection['original_sha256']!=comparison['original_sha256']:raise ValueError('Inspection mismatch')
 previous=ROOT/'research/runtime/actor-particle-results-20261022/reconciliation.json';old=read_json(previous,root=ROOT)
 header='reconstruction/rendering/actor_emission_provider.hpp'
 added={int(v,16) for v in re.findall(r'case (0x[0-9a-f]+):',(ROOT/header).read_text())}
 if added!={0x545910,0x546ce0,0x545650,0x5456c0,0x546120,0x546350,0x546270,0x546290} or set(comparison['roots'])!=added or comparison['caller_roots']!=[0x493f20]:raise ValueError('Support roots differ')
 routes={v['address']:v['model'] for v in old['concrete_routes']}
 if len(routes)!=219 or added&routes.keys():raise ValueError('Historical routes differ')
 routes.update({v:'EmissionComposition' for v in added});supplied={v['address']:v for v in comparison['supplied_providers']}
 frontier=[]
 for v in inspection['frontier']:
  row=dict(v)
  if v['operand'].startswith('0x'):
   target=int(v['operand'],16);row.update(target=target,concrete_route=routes.get(target),supplied_provider=supplied.get(target))
  frontier.append(row)
 result=dict(schema=1,success=True,static_review_only=True,scope='227 public routes including emitter/child updates, real manager ticks and actual493f20 construction. Retain43 historical leads,112 named supplier specifications and one separately identified owned nested-update callback control; original owner/planner and other provider recovery remains partial.',sources={header:evidence_digest(ROOT/header,root=ROOT),'tools/reconcile-actor-emission.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},original_sha256=comparison['original_sha256'],inputs={str(q):evidence_digest(q,root=ROOT) for q in [a.comparison,a.inspection,previous]},concrete_routes=[dict(address=v,model=m) for v,m in sorted(routes.items())],added_routes=sorted(added),previously_supplied_now_concrete=sorted({v['address'] for v in old['supplied_providers']}&added),historical_semantic_leads=old['historical_semantic_leads'],supplied_providers=comparison['supplied_providers'],inspected_call_sites=frontier,unresolved_indirect_sites=old['unresolved_indirect_sites']+[v for v in frontier if 'target' not in v],recursive_provider_closure=False,whole_original_session=False,live_replacement=False,indirect_target_completeness=False)
 write_report(ROOT,a.output,(json.dumps(result,indent=2)+'\n').encode())
 print(json.dumps(dict(routes=len(routes),added=len(added),supplied_endpoints=len(supplied))))
if __name__=='__main__':main()
