"""Check required rendering gaps and preserve bounded evidence, without acceptance."""
import json
from pathlib import Path
import subprocess

from coverage_audit import pointer, source_hashes
from coverage_claims import fingerprint
from coverage_evidence import Reader
from rendering_completion import PLAN, require, review as review_completion

POLICY = {'missing_proof_does_not_establish_missing_code',
          'component_or_historical_proof_does_not_close_requirement',
          'required_unknowns_cannot_be_silently_excluded', 'source_freshness_is_not_scope_acceptance',
          'static_register_cannot_promote_engine_status', 'task_order_not_assessed'}
CLASSIFICATIONS = {'missing_proof', 'missing_code', 'missing_integration'}


def unique(rows, expected, label):
    require(len(rows) == len(expected) and {r['id'] for r in rows} == set(expected),
            'Missing, duplicate or unexpected ' + label)


def validate(plan, registered):
    """Pure authored-data guards. Proof execution and scope resolutions are separate work."""
    gaps, contract = plan['required_gap_register'], plan['completion_contract']
    require(gaps['schema'] == 1 and gaps['revision'] == 1
            and gaps['id'] == 'native-rendering-required-gaps-v1', 'Unsupported gap register')
    require(gaps['completion_contract'] == dict(id=contract['id'], revision=contract['revision'],
                                               sha256=fingerprint(contract)), 'Completion contract binding differs')
    require(gaps['baseline'] == contract['baseline']
            and gaps['reviewed_source_commit'] == contract['reviewed_source_commit'], 'Baseline binding differs')
    require(set(gaps['assessment_policy']) == POLICY
            and all(v is True for v in gaps['assessment_policy'].values()), 'Gap assessment policy weakened')
    briefs = {r['id']: r for r in gaps['task_briefs']}
    require(len(briefs) == len(gaps['task_briefs']) and briefs, 'Duplicate or absent closing task')
    for row in briefs.values():
        require(set(row) == {'id', 'title', 'kind', 'close_when'}
                and all(isinstance(v, str) and v.strip() for v in row.values())
                and row['kind'] in {'acceptance', 'requiredness', 'diagnostic', 'closure_review'},
                'Closing task must be an unordered, concrete brief')
    bids = {b['id'] for b in registered['behaviors']}
    eids = {e['id'] for e in registered['evidence']}
    used = set()

    def links(row):
        require(row['closing_task'] in briefs, 'Unknown closing task: ' + row['id'])
        used.add(row['closing_task'])
        require(set(row.get('behavior_leads', [])) <= bids, 'Unknown behavior lead: ' + row['id'])
        require(set(row.get('evidence_leads', [])) <= eids, 'Unknown evidence lead: ' + row['id'])
        require(not ({'priority', 'depends_on', 'order', 'accepted', 'completion_percent'} & row.keys()),
                'Task ordering or acceptance is outside the gap register')

    def mandatory(row):
        links(row)
        require(row.get('state', 'open') == 'open', 'Static gap review cannot close acceptance')
        require(row['classification'] in CLASSIFICATIONS and row.get('remaining', '').strip(),
                'Incomplete mandatory gap assessment: ' + row['id'])
        if row['classification'] in {'missing_code', 'missing_integration'}:
            require(row.get('code_witnesses') and row.get('evidence_leads')
                    and row.get('missing_branch', '').strip(),
                    'Missing code/integration needs source witnesses, evidence and a concrete missing branch')

    journeys = {r['id']: r for r in plan['journeys']}
    unique(gaps['journeys'], journeys, 'journey assessment')
    configurations = {c['id'] for c in contract['configurations']}
    for row in gaps['journeys']:
        links(row)
        require(row['state'] == 'open' and row['classification'] in CLASSIFICATIONS
                and row['assessment'].strip() and row['implementation_assessment'].strip(),
                'Incomplete or closed journey assessment')
        require(set(row['configurations']) == configurations, 'Journey configuration omitted')
        unique(row['subrequirements'], journeys[row['id']]['requirements'], 'journey subrequirement')
        for leaf in row['subrequirements']:
            mandatory(leaf)
    unique(gaps['operations'], [r['id'] for r in plan['operations']], 'operation assessment')
    for row in gaps['operations']:
        mandatory(row)
        require(set(row['configurations']) == configurations, 'Operation configuration omitted')
    unique(gaps['gates'], [r['id'] for r in contract['acceptance']['gates']], 'completion-gate assessment')
    for row in gaps['gates']:
        mandatory(row)
    decisions = {r['id']: r for r in contract['scope_decisions']}
    unique(gaps['scope_decisions'], decisions, 'scope decision assessment')
    for row in gaps['scope_decisions']:
        links(row)
        require(row['classification'] == 'requiredness_unresolved' and row['state'] == 'open'
                and row['required_when'] == decisions[row['id']]['blocking_condition']
                and decisions[row['id']]['state'] == 'unresolved',
                'Conditional scope requires an explicit reviewed contract resolution')
        require(briefs[row['closing_task']]['kind'] == 'requiredness'
                and briefs[row['closing_task']]['close_when'] == decisions[row['id']]['close_when'],
                'Conditional scope closure differs from completion contract')
    unique(gaps['observations'], ['OBS.cursor-refusals', 'OBS.late-history'], 'retained observation')
    for row in gaps['observations']:
        links(row)
        require(row['evidence'] in eids and row['meaning'].strip()
                and row['classification'] in {'recorded_failure', 'requiredness_unresolved'},
                'Observation lost its evidence or bounded interpretation')
    unique(gaps['diagnostics'], ['DIAG.current-timing'], 'optional diagnostic')
    for row in gaps['diagnostics']:
        links(row)
        require(row['classification'] == 'diagnostic' and row['required_for_completion'] is False
                and row['remaining'].strip(), 'Optional timing became a completion requirement')
    require(used == set(briefs), 'Unlinked closing task')
    leaves = [s for j in gaps['journeys'] for s in j['subrequirements']]
    mandatory_rows = leaves + gaps['operations'] + gaps['gates']
    return dict(schema=1, success=True, kind='static_rendering_required_gap_review',
        contract=contract['id'], gap_register=gaps['id'], completion_contract=gaps['completion_contract'],
        baseline=gaps['baseline'], reviewed_source_commit=gaps['reviewed_source_commit'],
        counts=dict(journeys=len(journeys), journey_subrequirements=len(leaves),
            operations=len(gaps['operations']), completion_gates=len(gaps['gates']),
            mandatory_assessments=len(mandatory_rows), conditional_scope_decisions=len(decisions),
            retained_observations=len(gaps['observations']), optional_diagnostics=len(gaps['diagnostics']),
            unordered_task_briefs=len(briefs)),
        mandatory_classifications={c: sum(r['classification'] == c for r in mandatory_rows)
                                   for c in sorted(CLASSIFICATIONS)},
        engine_execution_performed=False, original_media_consumed=False, original_code_executed=False,
        engine_acceptance_assessed=False, native_equivalence=False, complete_replacement=False,
        execution_order_assessed=False, status_promotions=[], overall_completion_percent=None,
        scope='Requirement coverage and bounded evidence leads only. Counts overlap and are not features, runs, effort or completion percentages. No new engine acceptance or task ordering.')


def review(root):
    root = Path(root).resolve()
    completion = review_completion(root)
    with Reader(root) as reader:
        plan = reader.json(PLAN)
        gaps = plan['required_gap_register']
        register_path = 'research/runtime/coverage/register.json'
        # Bind leads to the product baseline, not this tooling batch's changing bookkeeping.
        original_register = json.loads(subprocess.check_output(
            ['git', '-C', str(root), 'show', gaps['reviewed_source_commit'] + ':' + register_path]))
        report = validate(plan, original_register)
        current = reader.json(register_path)
        validate(plan, current)
        current_evidence = {e['id']: e for e in current['evidence']}
        original_evidence = {e['id']: e for e in original_register['evidence']}
        baseline = reader.json(gaps['baseline']['record'])
        baseline_evidence = {e['id']: e for e in baseline['evidence']}
        rows = gaps['journeys'] + gaps['operations'] + gaps['diagnostics']
        eids = {e for r in rows for e in r['evidence_leads']} | {r['evidence'] for r in gaps['observations']}
        lead_records = []
        payloads = {}
        for eid in sorted(eids):
            ev = current_evidence[eid]
            require(ev == original_evidence[eid], 'Historical evidence definition changed: ' + eid)
            require(reader.digest(ev['record']) == ev['sha256'], 'Evidence payload differs: ' + eid)
            payload = reader.json(ev['record'])
            payloads[eid] = payload
            require(all(pointer(payload, a['pointer']) == a['equals'] for a in ev['assertions']),
                    'Evidence lead assertion failed: ' + eid)
            sources = source_hashes(pointer(payload, ev['sources_pointer']))
            inputs = ev.get('recorded_inputs', [])
            bindings = ev.get('source_bindings', {p: p for p in sources})
            changed = []
            for recorded, name in bindings.items():
                if recorded in inputs:
                    continue
                path = Path(name)
                require(not path.is_absolute() and '..' not in path.parts
                        and path.parts[0] not in ('original', 'working', '.git'), 'Unsafe evidence source')
                if not (root/path).is_file() or reader.digest(name) != sources[recorded]:
                    changed.append(name)
            old = baseline_evidence.get(eid)
            lead_records.append(dict(id=eid, record=ev['record'], sha256=ev['sha256'], kind=ev['kind'],
                scope=ev['scope'], payload_and_assertions_verified=True,
                baseline_source_binding_state=old['source_binding_state'] if old else 'not_in_selected_baseline',
                current_source_binding_state='changed' if changed else 'matching_recorded_sources',
                changed_sources=sorted(changed), closes_requirement=False))
        observations = {
            'OBS.cursor-refusals': dict(evidence='PLAYERMERGE27.storage01',
                selectors={'/failed_copy_calls': 2, '/unprepared_copy_calls': 2,
                           '/copy_output/verified': False,
                           '/copy_output/successful_emitted_outputs_verified': True,
                           '/copy_output/native_copies_compared': 4188}),
            'OBS.late-history': dict(evidence='RRG26.natural-native',
                selectors={'/independent_original_pixels_match': True, '/diagnostic_original_live_match': True,
                           '/live_original_pixels_match': False, '/fresh_live_execution': False,
                           '/original_pixels_used_as_native_inputs': False})}
        for row in gaps['observations']:
            spec = observations[row['id']]
            require(row['evidence'] == spec['evidence'] and all(pointer(payloads[spec['evidence']], p) == v
                    for p, v in spec['selectors'].items()), 'Retained negative observation changed')
        bids = {b for r in rows for b in r['behavior_leads']}
        baseline_behaviors = {b['id']: b for b in baseline['rendering']['behaviors']}
        behavior_leads = []
        for bid in sorted(bids):
            old = baseline_behaviors.get(bid)
            behavior_leads.append(dict(id=bid, selected_baseline=old is not None,
                baseline_status=old['status'] if old else None,
                baseline_current_validation=old['current_validation'] if old else None,
                closes_requirement=False))
        sources = dict(completion['sources'])
        for path in ['tools/rendering_gaps.py', 'tests/test-rendering-gaps.py',
                     'research/runtime/rendering-required-gaps-20261030.md']:
            sources[path] = reader.digest(path)
        report.update(sources=sources, completion_review_passed=completion['success'],
            evidence_leads=lead_records, behavior_leads=behavior_leads,
            retained_observation_selectors=observations,
            source_binding_matches_do_not_grant_acceptance=True,
            gaps=gaps, conditional_scope_boundaries=plan['completion_contract']['scope_decisions'])
        if 'scope_review' in completion:
            report['scope_review'] = completion['scope_review']
        return report
