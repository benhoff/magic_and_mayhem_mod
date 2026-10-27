#!/usr/bin/env python3
"""Identify actor installed-world cases without promoting component evidence."""
import argparse
import json
from pathlib import Path
import re

from coverage_evidence import evidence_digest, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
PARENT = 'RS.actor-downstream-services'
BUILD = 'nocd-40209ca7'
FRONTIER = 'research/runtime/emitter-families-results-20261022/reconciliation.json'
TRANSITIONS = 'reconstruction/rendering/actor_transition_services.hpp'

# Authored acceptance requirements, not generated observations or closure claims.
# Addresses are build-specific supplier leads; they do not imply whole-function proof.
CASES = (
    ('AD.T1', 'timing', 'World clock and owner allocation',
     ('AI.owner-brain-count-driver', 'AI.owner-task6-scheduler'),
     (0x46afc0, 0x415c40),
     'Compare real World call order and clock/budget publication at zero, odd and exhausted budgets, disabled brains, retained planning phases and task expiry; cover the timing/rdtsc mode separately from count mode.'),
    ('AD.T2', 'timing', 'World-produced sources and task recipients',
     ('RS.actor-session-source-lifecycle', 'AI.owner-knowledge-source-cache-ownership'),
     (0x416590, 0x416fe0, 0x4289e0, 0x428b40),
     'Drive registration, cache invalidation, planning, recipient handoff, task retirement and slot reuse from one installed World; compare shared identities, queues, source backlinks and callback-time bytes through restart.'),
    ('AD.R1', 'resource', 'Installed profile and animation publication',
     ('RS.actor-concrete-provider-frontier', 'RS.actor-profile-transform-providers'),
     (0x4644d0, 0x5050d0, 0x505220, 0x57d310),
     'Bind a declared installed profile set and actual file/profile backend to activation, animation events and retirement; compare missing, short, failed-allocation and reload prefixes, pointer publication and retained ownership. Admit or reject excluded profile domains explicitly.'),
    ('AD.R2', 'resource', 'Palette, font and surface lifetime',
     ('RS.actor-font-providers', 'RS.actor-palette-builder-providers'),
     (0x57d690, 0x4a5ae0, 0x4a5ce0, 0x58c8a0),
     'Compose installed sprite/font loading and palette chains with actual surface bind/unbind, restore/reload and session teardown. Check partial publication and allocation failures, reload-retained buffers and callback-visible ownership; select a concrete platform adapter.'),
    ('AD.C1', 'callback', 'Transition handlers over shared actor storage',
     ('RS.actor-session-source-lifecycle', 'AI.behavior-handlers', 'AI.route-displacement',
      'AI.corpse-lifecycle', 'AI.autonomous-behaviors', 'AI.owned-command-behaviors-chain'),
     (0x50aa30,),
     'Replace each of the fifteen selected handler endpoints with an explicit adapter to its recovered contract over the same actor/map/profile/resource state. Compare callback publication before entry, immediate and deferred transitions, nested reset/transition, death and special handlers. Preserve unsupported-kind/null behavior as a separate admission case.'),
    ('AD.C2', 'callback', 'Reentrant retirement and generation ownership',
     ('RS.actor-session-source-lifecycle', 'RS.actor-emitting-update-composition'),
     (0x506d30, 0x51f5e0, 0x545910),
     'Compare callbacks that release actors, retain task recipients, append/reject children, evict particles or request reload during iteration. Check high-water rereads, release-before-cleanup order, source-protected reuse refusal and eventual retirement over shared installed generations.'),
    ('AD.C3', 'callback', 'Scenario, spell and subtype endpoints',
     ('RS.actor-service-composition', 'RS.actor-concrete-provider-frontier'),
     (0x416d80, 0x469170, 0x4694e0, 0x522770),
     'Inventory concrete targets reachable in the declared installed scenarios; compose evaluator/action/finalization, spell and special-subtype bodies with their actual receivers and effects. Retain every unassigned direct endpoint and indirect call site; an address association cannot close a route.'),
    ('AD.P1', 'timing', 'World particle cadence and resource failure',
     ('RS.actor-emitter-family-composition', 'RS.actor-emitting-update-composition'),
     (0x493f20, 0x5457e0, 0x545910),
     'Drive particles from original World producers and clock with installed terrain and animation resources. Compare seeded RNG streams, reverse tick order, owner changes, plane allocation failure, child transfer and retirement; preloaded terrain and synthetic callback controls remain bounded component evidence.'),
    ('AD.A1', 'resource', 'Audio query and retirement on the session clock',
     ('RS.actor-downstream-value-services', 'RS.actor-downstream-dependency-services'),
     (0x49c530, 0x4de090, 0x4943f0),
     'Compose real actor sound admission, scheduler lookup/retirement and the chosen audio backend during activation, animation, release and restart; compare nullable handles, status/stop/position ordering and failure lifetime at callback boundaries.'),
)


def callback_targets(text):
    """Read only the reviewed selector, never arbitrary case labels in the header."""
    selector = text.split('inline U behaviorCallback(U kind) {', 1)[1].split(
        '// Recovered transition control only.', 1)[0]
    rows = re.findall(r'case (\d+): return (0x[0-9a-f]+);', selector)
    result = [(int(kind), int(address, 16)) for kind, address in rows]
    if len(result) != 15 or len({kind for kind, _ in result}) != 15:
        raise ValueError('Transition selector changed: review the callback inventory')
    return result


def component_leads(register, address):
    return [dict(behavior=b['id'], scope=b['scope'], status=b['status'],
                 implementation=b['implementation'], evidence=b['evidence'])
            for b in register['behaviors'] if b['id'] != PARENT and b['implementation']
            and any(link.get('build') == BUILD and link.get('address') == address
                    for link in b['original'])]


def inventory(register, frontier, selector):
    builds = {b['id']: b for b in register['builds']}
    if frontier.get('original_sha256') != builds[BUILD]['source_sha256']:
        raise ValueError('Frontier original build mismatch')
    if not frontier.get('success') or not frontier.get('static_review_only'):
        raise ValueError('Require successful historical static frontier')
    by_id = {b['id']: b for b in register['behaviors']}
    cases = []
    for identity, category, title, dependencies, addresses, acceptance in CASES:
        missing = set(dependencies) - by_id.keys()
        if missing:
            raise ValueError('Unknown dependency contracts: ' + ', '.join(sorted(missing)))
        cases.append(dict(id=identity, category=category, title=title, status='pending',
                          dependencies=list(dependencies),
                          original_addresses=list(addresses), acceptance=acceptance))
    callbacks = [dict(kind=kind, address=address, component_leads=component_leads(register, address),
                      installed_actor_composition='pending', case='AD.C1')
                 for kind, address in callback_targets(selector)]
    providers = []
    for specification in frontier['supplied_providers']:
        address = specification['address']
        # 1001xxxx addresses are harness-owned ABI tokens, never original bodies.
        leads = [] if address >= 0x10000000 else component_leads(register, address)
        providers.append(dict(specification=specification, component_leads=leads,
                              disposition='authored_fixture_token' if address >= 0x10000000
                              else 'component_lead_requires_composition' if leads
                              else 'no_registered_implementation_lead',
                              exercised_by_installed_world='unproven'))
    sites = {}
    for row in frontier['unresolved_indirect_sites']:
        key = (row['owner'], row['site'], row.get('operand', row.get('unresolved_indirect')))
        sites[key] = dict(owner=key[0], site=key[1], operand=key[2],
                          target_completeness='pending')
    return dict(schema=1, success=True, static_review_only=True, behavior=PARENT,
                original_sha256=frontier['original_sha256'], cases=cases, callbacks=callbacks,
                historical_supplied_endpoints=providers,
                historical_unresolved_indirect_sites=list(sites.values()),
                counts=dict(required_cases=len(cases), selected_callbacks=len(callbacks),
                            callbacks_with_component_leads=sum(bool(c['component_leads']) for c in callbacks),
                            historical_routes=len(frontier['concrete_routes']),
                            historical_endpoint_specifications=len(providers),
                            unique_historical_indirect_sites=len(sites)),
                required_cases_closed=0, whole_original_session=False, live_replacement=False,
                limitations=['Component links and historical route inventories grant no new execution, freshness, integration or replacement credit.',
                             'The historical frontier is not an exhaustive current whole-engine call graph.',
                             'Malformed/indeterminate original inputs, Windows SEH and throwing cleanup require explicit admission limits or separate evidence.',
                             'Supported-session acceptance, physical legacy driver equivalence and live replacement are independent milestones.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    register = read_json(ROOT/'research/runtime/coverage/register.json', root=ROOT)
    frontier = read_json(ROOT/FRONTIER, root=ROOT)
    report = inventory(register, frontier, (ROOT/TRANSITIONS).read_text())
    paths = ['tools/audit-actor-dependencies.py', TRANSITIONS]
    report['sources'] = {p: evidence_digest(ROOT/p, root=ROOT) for p in paths}
    report['inputs'] = {FRONTIER: evidence_digest(ROOT/FRONTIER, root=ROOT)}
    # Bind exactly the inspected register bytes without making its generated
    # report reference part of a self-referential execution source claim.
    report['reviewed_register_sha256'] = evidence_digest(
        ROOT/'research/runtime/coverage/register.json', root=ROOT)
    write_report(ROOT, args.output, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(report['counts']))


if __name__ == '__main__':
    main()
