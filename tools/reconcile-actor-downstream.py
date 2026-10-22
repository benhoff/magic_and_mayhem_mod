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
DIRECT_SUPPLIERS={
 'RS.actor-downstream-value-services':'reconstruction/rendering/actor_services.hpp',
 'RS.actor-downstream-controller-services':'reconstruction/rendering/actor_service_controllers.hpp',
 'RS.actor-downstream-world-services':'reconstruction/rendering/actor_service_world.hpp',
 'RS.actor-downstream-ui-services':'reconstruction/rendering/actor_service_ui.hpp',
}

SUPPLIERS={**DIRECT_SUPPLIERS,
 'RS.actor-downstream-dependency-services':'reconstruction/rendering/actor_service_dependencies.hpp',
 'RS.actor-downstream-popup-services':'reconstruction/rendering/actor_service_popups.hpp',
 'RS.actor-downstream-collection-services':'reconstruction/rendering/actor_service_collections.hpp',
 'RS.actor-downstream-effect-services':'reconstruction/rendering/actor_service_effects.hpp',
 'RS.actor-downstream-construction-services':'reconstruction/rendering/actor_service_construction.hpp',
}
CRT_BOUNDARIES={0x59c1fd:'MSVC sprintf: caller-owned destination and format arguments; concrete CRT formatting remains an explicit runtime service.',0x59c4c3:'MSVC fclose: FILE lifetime and OS handle retirement remain an explicit runtime service.',0x59cae2:'MSVC fopen: path/mode inputs and success/failure FILE token remain an explicit runtime service.',0x59cb60:'MSVC memmove: native byte-overlap semantics use std::memmove; concrete CRT implementation is not engine ownership.',0x59d50b:'MSVC fprintf: FILE token, format and variable arguments remain an explicit runtime service.'}

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
    all_selected=dict(selected)
    selected={address:bid for address,bid in selected.items() if bid in DIRECT_SUPPLIERS}
    selected[0x416590]='AI.owned-source-controller-chain'
    roots=set(inspection['roots']);assert len(roots)==72 and roots==set(selected)
    root_rows=[]
    for address,bid in sorted(selected.items()):
        b=by[bid]
        root_rows.append(dict(address=address,supplier=bid,scope=b['scope'],status=b['status'],
            implementation=b['implementation'],tests=b['tests'],
            current_run_evidence=[e for e in b['evidence'] if e.startswith(('DSVC1.','DSVC2.'))],
            representation='logical ownership/grouping projection; opaque byte25 and successful allocator/vector domain remain explicit' if address==0x416590 else 'PE32 value-address service body; external providers remain explicit'))
    frontier=[]
    for target in sorted({x['target'] for x in rows if 'target' in x}-roots):
        leads=[b for b in register['behaviors'] if b['id']!='RS.actor-downstream-services' and b['implementation'] and any(x.get('address')==target and x['build']=='nocd-40209ca7' for x in b['original'])]
        owned_supplier=all_selected.get(target)
        disposition=('Recovered value-address body with its own bounded comparison; composed only in the suites declaring that supplier.' if owned_supplier else CRT_BOUNDARIES.get(target,'Existing semantic supplier requires caller-specific composition; association alone grants no completion.'))
        frontier.append(dict(address=target, recovered_supplier=owned_supplier, runtime_boundary=CRT_BOUNDARIES.get(target),sites=[x for x in rows if x.get('target')==target],
            scoped_links=[dict(behavior=b['id'],scope=b['scope'],status=b['status'],implementation=b['implementation'],evidence=b['evidence']) for b in leads],
            disposition=disposition))
    indirect=[]
    for row in rows:
        if 'unresolved_indirect' not in row:continue
        owner=row['owner'];operand=row['unresolved_indirect']
        if owner in {0x49c530,0x4de090,0x4943f0}:
            boundary='DirectSound COM status/stop/position dispatch; slot/ABI exercised with owned providers, actual buffer/device admission remains separate.'
        elif owner==0x59c441:
            boundary='Caller-supplied vector constructor. Normal463cb0 caller supplies464350 and exceptional destructor463270; actual464350/463270 bodies and59bf87/59c4ab cleanup now have dedicated ownership comparisons. OS SEH delivery and throwing during cleanup remain separate; normal vectors here retain a supplied callback.'
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
            frontier_entries_without_implementation_link=sum(not x['scoped_links'] for x in frontier),newly_recovered_frontier_entries=sum(bool(x['recovered_supplier']) for x in frontier),explicit_crt_boundaries=sum(bool(x['runtime_boundary']) for x in frontier),existing_supplier_composition_boundaries=sum(not x['recovered_supplier'] and not x['runtime_boundary'] for x in frontier),additional_value_address_bodies=len(all_selected)-71,indirect_sites=len(indirect)),
        all_downstream_transitive_services_complete=False,whole_session_integration=False,live_replacement=False,
        scope='Explicit72-entry direct services plus the complete initial83-entry frontier disposition. The34 previously unlinked engine/runtime cleanup entries now have native bodies; five CRT boundaries are named. Existing scoped suppliers and14 indirect sites retain caller-specific composition limitations. Additional constructor/canvas helper bodies have dedicated comparisons. This is static accounting, not transitive session integration or live replacement proof.')
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['counts']))

if __name__=='__main__':main()
