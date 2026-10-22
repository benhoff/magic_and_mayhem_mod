#!/usr/bin/env python3
"""Retain explicit actor service suppliers and unresolved downstream boundaries.

This is a static reconciliation, not new execution or transitive closure proof.
Function association alone never supplies a semantic completion claim.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]
SUPPLIERS={
 'RS.actor-downstream-value-services':'reconstruction/rendering/actor_services.hpp',
 'RS.actor-downstream-controller-services':'reconstruction/rendering/actor_service_controllers.hpp',
 'RS.actor-downstream-world-services':'reconstruction/rendering/actor_service_world.hpp',
 'RS.actor-downstream-ui-services':'reconstruction/rendering/actor_service_ui.hpp',
}

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--instructions',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();inputs=a.instructions.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        p.error('Require new retained output under working/tests')
    if not inputs.is_relative_to(ROOT/'working/tests'):
        p.error('Require retained instruction inspection under working/tests')
    inspection=json.loads((inputs/'report.json').read_text())
    if not inspection['success'] or not inspection['static_only']:
        p.error('Require successful static inspection')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    by={b['id']:b for b in register['behaviors']}
    rows=json.loads((inputs/'call-frontier.json').read_text())
    selected={};sources={'tools/reconcile-actor-downstream.py':sha(ROOT/'tools/reconcile-actor-downstream.py')}
    for bid,path in SUPPLIERS.items():
        b=by[bid];assert path in b['implementation'];sources[path]=sha(ROOT/path)
        for address in {int(x,16) for x in re.findall(r'case (0x[0-9a-f]+):',(ROOT/path).read_text())}:
            assert address not in selected
            assert any(x['address']==address and x['build']=='nocd-40209ca7' for x in b['original'])
            selected[address]=bid
    selected[0x416590]='AI.owned-source-controller-chain'
    roots=set(inspection['roots']);assert len(roots)==72 and roots==set(selected)
    root_rows=[]
    for address,bid in sorted(selected.items()):
        b=by[bid]
        root_rows.append(dict(address=address,supplier=bid,scope=b['scope'],status=b['status'],
            implementation=b['implementation'],tests=b['tests'],
            current_run_evidence=[e for e in b['evidence'] if e.startswith('DSVC1.')],
            representation='logical ownership/grouping projection; opaque byte25 and successful allocator/vector domain remain explicit' if address==0x416590 else 'PE32 value-address service body; external providers remain explicit'))
    frontier=[]
    for target in sorted({x['target'] for x in rows if 'target' in x}-roots):
        leads=[b for b in register['behaviors'] if b['id']!='RS.actor-downstream-services' and b['implementation'] and any(x.get('address')==target and x['build']=='nocd-40209ca7' for x in b['original'])]
        frontier.append(dict(address=target,sites=[x for x in rows if x.get('target')==target],
            scoped_links=[dict(behavior=b['id'],scope=b['scope'],status=b['status'],implementation=b['implementation'],evidence=b['evidence']) for b in leads],
            disposition='Scoped supplier leads require exact semantic/composition review; entry association does not establish complete service recovery.' if leads else 'No explicitly registered implementation link at this entry; retain for original behavior recovery or explicit platform/runtime boundary classification.'))
    indirect=[]
    for row in rows:
        if 'unresolved_indirect' not in row:continue
        owner=row['owner'];operand=row['unresolved_indirect']
        if owner in {0x49c530,0x4de090,0x4943f0}:
            boundary='DirectSound COM status/stop/position dispatch; slot/ABI exercised with owned providers, actual buffer/device admission remains separate.'
        elif owner==0x59c441:
            boundary='Caller-supplied vector constructor. Normal463cb0 caller supplies464350 and exceptional destructor463270; actual constructor/destructor bodies and MSVC exception unwind are separate remaining recovery.'
        elif '0x5c50c0' in operand:
            boundary='GetFullPathNameA import; owned success/failure/path-component callback checked, actual Windows API implementation remains external.'
        elif '0x5c520c' in operand or '0x5c5208' in operand:
            boundary='Windows message/beep import; callback arguments remain explicit, physical UI/platform dispatch is not recovered engine behavior.'
        elif owner in {0x4d4970,0x4d9ac0}:
            boundary='Widget virtual dispatch; receiver/slot and caller state are checked, the widget method bodies remain downstream services.'
        else:boundary='Unresolved indirect dispatch retained; no target completeness inferred.'
        indirect.append(dict(row,boundary=boundary))
    report=dict(schema=1,success=True,static_review_only=True,sources=sources,
        inputs={str((inputs/name).relative_to(ROOT)):sha(inputs/name) for name in ['report.json','call-frontier.json']},
        original_sha256=inspection['original_sha256'],roots=root_rows,direct_frontier=frontier,indirect=indirect,
        counts=dict(direct_actor_entries=len(roots),value_address_entries=71,reused_source_controller_entries=1,
            direct_frontier_entries=len(frontier),frontier_entries_with_scoped_links=sum(bool(x['scoped_links']) for x in frontier),
            frontier_entries_without_implementation_link=sum(not x['scoped_links'] for x in frontier),indirect_sites=len(indirect)),
        all_downstream_transitive_services_complete=False,whole_session_integration=False,live_replacement=False,
        scope='Explicit72-entry direct-service reconciliation with four bounded value-address matrices and the existing source grouping/registration supplier. The83-entry further direct frontier,14 indirect sites, exceptional constructors/destructors and actual platform providers remain visible; this report does not convert them into completed services.')
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['counts']))

if __name__=='__main__':main()
