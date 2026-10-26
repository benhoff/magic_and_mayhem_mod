"""Read-only rendering validation gaps; function links never imply full branches."""
from collections import Counter
from functools import lru_cache
import json
from pathlib import Path

from coverage_audit import audit, pointer
from coverage_claims import scenario_contract
from coverage_evidence import Reader
from drawing_journeys import CATALOG, inventory
from rendering_coverage import MATRIX, REGISTER, SCOPE, reconcile

SUBSYSTEMS = frozenset(('rendering', 'native_rendering', 'rendering_inventory',
                       'rendering_reconstruction', 'rendering_surfaces', 'rendering_sprites',
                       'renderer', 'native_world', 'drawing_inventory', 'terrain_lighting'))
SOURCE_ROOTS = ('renderer/', 'reconstruction/rendering/')


def selected(behavior):
    return behavior['subsystem'] in SUBSYSTEMS or any(
        name.startswith(SOURCE_ROOTS) for name in behavior['implementation'])


def frontier(register, audit_report, load_claims):
    """Derive current proof per behavior/scenario, keeping historical milestones."""
    valid = not audit_report['errors']
    audited = {b['id']: b for b in audit_report.get('behaviors', [])}
    rows = []
    for behavior in register['behaviors']:
        if not selected(behavior):
            continue
        bid = behavior['id']
        current = audited.get(bid, {}).get('current_validation', {})
        stages = {}
        for stage in ('implementation', 'comparison', 'integration', 'replacement'):
            proof = current.get(stage, {'state': 'pending', 'evidence': []})
            stages[stage] = dict(state=proof['state'] if valid else 'unknown',
                                 evidence=proof.get('evidence', []) if valid else [])
        scenarios = []
        for scenario in register['scenarios']:
            if bid not in scenario['behaviors']:
                continue
            proofs = {}
            for stage, value in stages.items():
                candidates = set(value['evidence']) & set(scenario['evidence'])
                if stage == 'integration' and scenario['level'] != behavior['status']['integration']:
                    candidates = set()
                matching = []
                for eid in sorted(candidates):
                    claims, sources = load_claims(eid)
                    if not set(scenario['tests']) <= set(sources):
                        continue
                    if any(c['behavior'] == bid and c.get('scenarios', {}).get(scenario['id']) == scenario_contract(scenario) for c in claims):
                        matching.append(eid)
                proofs[stage] = matching
            scenarios.append(dict(id=scenario['id'], level=scenario['level'], scope=scenario['scope'], current_evidence=proofs))
        if behavior['status']['implementation'] == 'none' and behavior['subsystem'] in ('rendering_inventory', 'drawing_inventory'):
            next_action = 'refine inventory and implementation links'
        elif behavior['status']['implementation'] == 'none':
            next_action = 'review absent implementation claim'
        elif behavior['kind'] == 'recovered' and stages['comparison']['state'] != 'current':
            next_action = 'obtain current original comparison'
        elif stages['integration']['state'] != 'current':
            next_action = 'choose or validate integration boundary'
        else:
            next_action = 'exercise remaining branches and scenarios'
        rows.append(dict(id=bid, title=behavior['title'], kind=behavior['kind'], subsystem=behavior['subsystem'],
                         scope=behavior['scope'], original=behavior['original'], status=behavior['status'],
                         current_validation=stages, scenarios=scenarios,
                         remaining_gaps=behavior.get('remaining_gaps', []), next_action=next_action,
                         complete_branch_coverage=None))
    selected_ids = {r['id'] for r in rows}
    by_id = {r['id']: r for r in rows}
    scenario_rows = []
    for scenario in register['scenarios']:
        inside = [bid for bid in scenario['behaviors'] if bid in selected_ids]
        if not inside:
            continue
        outside = [bid for bid in scenario['behaviors'] if bid not in selected_ids]
        proved, missing = [], []
        for bid in inside:
            proof = next(s for s in by_id[bid]['scenarios'] if s['id'] == scenario['id'])
            (proved if proof['current_evidence']['integration'] else missing).append(bid)
        scenario_rows.append(dict(id=scenario['id'], level=scenario['level'], scope=scenario['scope'],
                                  selected_behaviors=inside, outside_audit_behaviors=outside,
                                  current_integrated_behaviors=proved,
                                  behaviors_without_current_integration_proof=missing,
                                  current_integration_for_all_selected=not missing))
    dispatch = [d for d in register['dispatch_tables'] if not d['complete'] and (
        d['id'].startswith(('RI.', 'world-', 'kind8-', 'minimap-')) or
        any(selected_ids & set(e.get('behaviors', [])) for e in d['entries']))]
    return dict(audit_valid=valid, errors=audit_report['errors'], behaviors=rows, scenarios=scenario_rows, unresolved_dispatch=dispatch,
                counts=dict(registered_behaviors=len(rows),
                            registered_scenarios=len(scenario_rows),
                            scenarios_missing_selected_integration=sum(bool(s['behaviors_without_current_integration_proof']) for s in scenario_rows),
                            kinds=dict(Counter(r['kind'] for r in rows)),
                            next_actions=dict(Counter(r['next_action'] for r in rows)),
                            current_validation={stage: dict(Counter(r['current_validation'][stage]['state'] for r in rows))
                                                for stage in ('implementation', 'comparison', 'integration', 'replacement')}),
                overall_completion_percent=None,
                scope='Reviewed registered rendering contracts only. Original addresses cover their stated scope; branch completeness is unmeasured. Historical milestones, current evidence, integration and replacement are independent. Missing/current-stale proof is a validation gap, not an inferred implementation defect.')


def build_report(root):
    root = Path(root).resolve()
    with Reader(root) as reader:
        register = reader.json(REGISTER)
        audited = audit(root, register)
        evidence = {e['id']: e for e in register['evidence']}

        @lru_cache(maxsize=128)
        def claims(eid):
            e = evidence[eid]
            record = reader.json(e['record'])
            sources = pointer(record, e['sources_pointer'])
            if isinstance(sources, list):
                sources = {v['path']: v['sha256'] for v in sources}
            return pointer(record, e['claims_pointer']), sources

        result = frontier(register, audited, claims)
        result['schema'] = 1
        result['category_reconciliation'] = reconcile(root, register, reader.json(MATRIX), reader.json(SCOPE), audited)
        result['journey_requirements'] = inventory(root)
        result['journey_requirements']['execution_performed'] = False
        result['journey_requirements']['current_journey_parity_assessed'] = False
        result['journey_requirements']['generic_session_catalog_count'] = sum(
            c['adapter'] == 'session' for c in result['journey_requirements']['cases'])
        result['journey_requirements']['scope'] = 'Static acceptance inventory; this audit runs no journey. A catalog label, component result or observed EGL subset cannot close generic action-bound presented-frame acceptance.'
        result['binary_unknowns_all_subsystems'] = {k: audited.get('counts', {}).get(k) for k in (
            'unclassified_functions', 'unassigned_executable_ranges', 'unresolved_indirect_flows', 'incomplete_dispatch_tables')}
        result['inputs'] = {name: reader.digest(name) for name in (REGISTER, MATRIX, SCOPE, CATALOG)}
        result['sources'] = {name: reader.digest(name) for name in (
            'tools/rendering_frontier.py', 'tools/audit-rendering-frontier.py', 'tools/rendering_coverage.py',
            'tools/coverage_audit.py', 'tools/coverage_claims.py', 'tools/drawing_journeys.py',
            'tests/test-rendering-frontier.py')}
        result['audit_warnings'] = audited['warnings']
        return result


def markdown(report):
    lines = ['# Rendering validation frontier', '', report['scope'], '',
             'No overall completion percentage is computed. Counts refer only to the selected registered checklist. Pending rows can share changed dependencies; they are not a count of independent defects or engineering tasks.', '',
             '| Current registered proof | Current | Pending | None | Unknown |',
             '| --- | ---: | ---: | ---: | ---: |']
    for stage, states in report['counts']['current_validation'].items():
        lines.append('| '+stage+' | '+' | '.join(str(states.get(k, 0)) for k in ('current', 'pending', 'none', 'unknown'))+' |')
    lines += ['', '## Reviewed drawing-route boundaries', '',
              '| Route | Scope | Remaining | Current original subset proofs |', '| --- | --- | --- | --- |']
    for row in report['category_reconciliation']['drawing_routes']['rows']:
        proofs = ', '.join(p['evidence'] for p in row['validation'] if p['current'] and p['domain'] == 'original_executable') or 'none'
        cells = (row['id'], row['scope'], row['remaining'], proofs)
        lines.append('| '+' | '.join(str(v).replace('|', '\\|').replace('\n', ' ') for v in cells)+' |')
    lines += ['', '## Formal journey acceptance requirements', '',
              'This is an acceptance inventory, not a new journey execution. The SQLite report separately retains every catalog adapter and the current registered scenario proof.', '',
              f"Configured generic presented-session catalogs: {report['journey_requirements']['generic_session_catalog_count']}. Recorded EGL subsets and component fixtures keep their separate boundaries.", '',
              '| Journey | Required gates |', '| --- | --- |']
    for row in report['journey_requirements']['scenarios']:
        lines.append('| '+row['id']+' | '+', '.join(row['required'])+' |')
    lines += ['', '## Current registered scenario integration', '',
              'Counts apply to the selected rendering contracts within each scenario scope. Other declared behaviors remain outside this audit; a passing subset cannot establish their integration.', '',
              '| Scenario | Boundary | Selected contracts with current proof / selected | Other declared contracts |',
              '| --- | --- | ---: | ---: |']
    for row in report['scenarios']:
        lines.append(f"| {row['id']} | {row['level']} | {len(row['current_integrated_behaviors'])}/{len(row['selected_behaviors'])} | {len(row['outside_audit_behaviors'])} |")
    lines += ['', '## Next work by evidence boundary', '']
    for action, count in sorted(report['counts']['next_actions'].items()):
        lines.append(f'- {action}: {count} registered contracts.')
    lines += ['', 'The full SQLite report includes exact behavior IDs, original build/address scopes, current per-scenario evidence, explicit remaining gaps and unresolved dispatch tables. A function link or one passing scenario does not establish other branches.', '']
    if report['errors']:
        lines += ['## Audit errors', ''] + ['- '+e for e in report['errors']]
    return '\n'.join(lines)
