#!/usr/bin/env python3
"""Retain exact actor-session routing and the unresolved semantic/provider frontier."""
import argparse
import importlib.util
import json
from pathlib import Path
import re
from coverage_evidence import evidence_digest, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
module_spec = importlib.util.spec_from_file_location('prior_composition', ROOT/'tools/reconcile-service-composition.py')
prior = importlib.util.module_from_spec(module_spec)
module_spec.loader.exec_module(prior)
# These helpers execute within already recovered bodies rather than receiving
# another public dispatch entry. This is a reviewed implementation relationship,
# not proof that every original caller/branch was executed in the session matrix.
EMBEDDED = {
    0x40a6b0: ('actor_source_services.hpp', 'Wrapped squared coordinate distance'),
    0x40e290: ('actor_service_world.hpp', 'Map query'),
    0x40e2c0: ('actor_service_ui.hpp', 'Map/UI scalar helper'),
    0x40e4e0: ('actor_service_world.hpp', 'Map query'),
    0x40e650: ('actor_service_world.hpp', 'Map query'),
    0x40e8a0: ('actor_service_world.hpp', 'Map query'),
    0x40e8d0: ('actor_service_ui.hpp', 'Map/UI scalar helper'),
    0x40eb70: ('actor_service_world.hpp', 'Map query'),
    0x40eeb0: ('actor_source_services.hpp', 'Coordinate normalization'),
    0x40eec0: ('actor_service_ui.hpp', 'Map/UI scalar helper'),
    0x4132e0: ('actor_service_world.hpp', 'Map query'),
    0x4eaca0: ('actor_service_world.hpp', 'Map iterator'),
    0x4eade0: ('actor_service_world.hpp', 'Map iterator'),
    0x5065e0: ('actor_service_composition.hpp', 'Activation field helper'),
    0x509790: ('actor_service_ui.hpp', 'Widget field helper'),
    0x50da50: ('actor_service_controllers.hpp', 'Controller field helper'),
    0x534520: ('actor_ownership.hpp', 'Empty-column predicate for valid owned map cells; arbitrary malformed/unnormalized pointers excluded'),
    0x594490: ('actor_service_controllers.hpp', 'Controller scalar helper'),
    0x59bee0: ('actor_service_world.hpp', 'Map field helper'),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--comparison', type=Path, required=True)
    parser.add_argument('--inspection', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    expected, lifecycle, paths = prior.routes()
    all_routes = dict(expected)
    all_routes.update({entry: 'Composition.lifecycle' for entry in lifecycle})
    added = {}
    for model, filename in [('SourceServices','actor_source_services.hpp'),
                            ('TransitionServices','actor_transition_services.hpp'),
                            ('SessionComposition','actor_session.hpp')]:
        path = 'reconstruction/rendering/' + filename
        paths.append(path)
        entries = {int(v,16) for v in re.findall(r'case (0x[0-9a-f]+):',(ROOT/path).read_text())}
        if model == 'SessionComposition':
            # This dispatcher forwards source/transition entries to those models.
            entries -= set(added)
        for entry in entries:
            if entry in all_routes:
                raise ValueError('Duplicate public route: '+hex(entry))
            all_routes[entry] = model
            added[entry] = model
    comparison = read_json(args.comparison, root=ROOT)
    if not comparison['success'] or not comparison['sources_stable']:
        raise ValueError('Require successful source-stable session comparison')
    for path, digest in comparison['sources'].items():
        if evidence_digest(ROOT/path,root=ROOT) != digest:
            raise ValueError('Comparison source drift: '+path)
    supplied = {row['address']: row for row in comparison['supplied_providers']}
    inspected = read_json(args.inspection/'report.json',root=ROOT)
    if not inspected['success'] or inspected['original_sha256'] != comparison['original_sha256']:
        raise ValueError('Inspection/build mismatch')
    frontier = read_json(args.inspection/'call-frontier.json',root=ROOT)
    initial_path = ROOT/'research/runtime/downstream-service-results-20261021/service-matrix-extended.json'
    initial = read_json(initial_path,root=ROOT)
    leads = [row for row in initial['direct_frontier'] if not row['recovered_supplier'] and not row['runtime_boundary']]
    if len(leads) != 43:
        raise ValueError('Historical semantic frontier differs')
    dispositions=[]
    for row in leads:
        entry=row['address']
        result=dict(address=entry, historical_links=[b['behavior'] for b in row['scoped_links']])
        if entry in all_routes:
            result.update(disposition='concrete_route',model=all_routes[entry])
        elif entry in EMBEDDED:
            filename, scope=EMBEDDED[entry]
            result.update(disposition='embedded_scoped_helper',implementation='reconstruction/rendering/'+filename,scope=scope)
        elif entry in supplied:
            result.update(disposition='supplied_provider_remaining',provider=supplied[entry])
        else:
            result.update(disposition='unresolved_composition_remaining')
        dispositions.append(result)
    paths += ['reconstruction/rendering/'+filename for filename,_ in EMBEDDED.values()]
    paths += ['tools/reconcile-actor-session.py','tools/reconcile-service-composition.py','tools/coverage_evidence.py']
    result=dict(schema=1,success=True,static_review_only=True,
                scope='Exact native routing and explicit disposition of the historical43 semantic leads; no transitive/cadence or live closure.',
                sources={p:evidence_digest(ROOT/p,root=ROOT) for p in sorted(set(paths))},
                original_sha256=comparison['original_sha256'],
                inputs={str(p):evidence_digest(p,root=ROOT) for p in [args.comparison,args.inspection/'report.json',args.inspection/'call-frontier.json',initial_path]},
                concrete_routes=[dict(address=entry,model=model) for entry,model in sorted(all_routes.items())],
                added_routes=[dict(address=entry,model=model) for entry,model in sorted(added.items())],
                historical_semantic_leads=dispositions,
                direct_frontier=[dict(row,concrete_route=all_routes.get(row.get('target')),supplied_provider=supplied.get(row.get('target'))) for row in frontier if 'target' in row],
                unresolved_indirect_sites=[row for row in frontier if 'target' not in row],
                supplied_providers=comparison['supplied_providers'],
                whole_original_session=False,live_replacement=False,indirect_target_completeness=False)
    write_report(ROOT,args.output,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(routes=len(all_routes),added=len(added),semantic_dispositions={name:sum(row['disposition']==name for row in dispositions) for name in sorted({row['disposition'] for row in dispositions})})))


if __name__ == '__main__':
    main()
