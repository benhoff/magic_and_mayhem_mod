#!/usr/bin/env python3
"""Account for concrete actor routes, inspected call frontiers and supplied ends.

This review never turns an address association into session validation.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
from coverage_evidence import read_json, evidence_digest, write_report

ROOT = Path(__file__).resolve().parents[1]
MODELS = {
    'Services': 'actor_services.hpp', 'Controllers': 'actor_service_controllers.hpp',
    'WorldServices': 'actor_service_world.hpp', 'UiServices': 'actor_service_ui.hpp',
    'Dependencies': 'actor_service_dependencies.hpp', 'PopupServices': 'actor_service_popups.hpp',
    'CollectionServices': 'actor_service_collections.hpp', 'EffectServices': 'actor_service_effects.hpp',
    'ConstructionServices': 'actor_service_construction.hpp', 'Providers': 'actor_service_providers.hpp',
}


def routes():
    expected = {}
    paths = ['tools/reconcile-service-composition.py', 'tools/coverage_evidence.py']
    for model, name in MODELS.items():
        path = 'reconstruction/rendering/' + name
        paths.append(path)
        for value in re.findall(r'case (0x[0-9a-f]+):', (ROOT/path).read_text()):
            entry = int(value, 16)
            if entry in expected:
                raise ValueError('Duplicate concrete route: ' + value)
            expected[entry] = model
    path = 'reconstruction/rendering/actor_service_composition.hpp'
    paths.append(path)
    code = (ROOT/path).read_text()
    actual = {}
    for cases, model in re.findall(r'((?:\s*case 0x[0-9a-f]+:)+)\s*return (\w+)<M>', code):
        for value in re.findall(r'case (0x[0-9a-f]+):', cases):
            actual[int(value, 16)] = model
    lifecycles = {0x506290, 0x506d30, 0x51f5e0}
    if actual != expected or set(map(lambda v: int(v, 16), re.findall(r'case (0x[0-9a-f]+):', code))) != set(expected) | lifecycles:
        raise ValueError('Composition routing differs from concrete model entries')
    return expected, lifecycles, paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inspection', type=Path, required=True)
    parser.add_argument('--comparison', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    expected, lifecycles, paths = routes()
    inspection = read_json(args.inspection/'report.json', root=ROOT)
    comparison = read_json(args.comparison, root=ROOT)
    if not inspection['success'] or not inspection['static_only'] or not comparison['success']:
        raise ValueError('Unsuccessful input')
    if inspection['original_sha256'] != comparison['original_sha256']:
        raise ValueError('Original builds differ')
    entries = set(expected) | lifecycles
    inspected = {int(value, 16) for value in read_json(args.inspection/'instructions.json', root=ROOT)}
    if not entries <= inspected:
        raise ValueError('Missing inspected concrete bodies')
    for name, digest in comparison['sources'].items():
        if evidence_digest(ROOT/name, root=ROOT) != digest:
            raise ValueError('Comparison source drift: ' + name)
    frontier = read_json(args.inspection/'call-frontier.json', root=ROOT)
    supplied = {row['address']: row for row in comparison['supplied_providers']}
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    leads = {}
    for behavior in register['behaviors']:
        if not behavior['implementation']:
            continue
        for ref in behavior['original']:
            if ref['build'] == 'nocd-40209ca7':
                leads.setdefault(ref['address'], []).append(behavior['id'])
    direct, indirect = [], []
    for row in frontier:
        if row['owner'] not in entries:
            continue
        if 'target' not in row:
            indirect.append(row)
            continue
        target = row['target']
        direct.append(dict(row, concrete_route=expected.get(target),
                           supplied_endpoint=supplied.get(target),
                           scoped_leads=leads.get(target, [])))
    result = dict(schema=1, success=True, static_review_only=True,
                  original_sha256=inspection['original_sha256'],
                  sources={p: evidence_digest(ROOT/p, root=ROOT) for p in paths},
                  inputs={str(p): evidence_digest(p, root=ROOT) for p in
                          [args.inspection/'report.json', args.inspection/'instructions.json',
                           args.inspection/'call-frontier.json', args.comparison]},
                  concrete_routes=[dict(address=a, model=m) for a, m in sorted(expected.items())],
                  lifecycle_routes=sorted(lifecycles), direct_frontier=direct, indirect_frontier=indirect,
                  supplied_providers=comparison['supplied_providers'],
                  native_completed_service_entries=comparison['native_completed_service_entries'],
                  native_coverage_is_not_original_entry_tracing=True,
                  whole_session_integration=False, live_replacement=False,
                  indirect_target_completeness=False,
                  scope='Concrete routing and selected body/caller proof. Scoped leads, supplied endpoints, '
                        'all indirect sites and new transitive frontiers remain explicit; no full session closure.')
    write_report(ROOT, args.output, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(routes=len(entries), native_completed=len(result['native_completed_service_entries']),
                          direct_sites=len(direct), indirect_sites=len(indirect))))


if __name__ == '__main__':
    main()
