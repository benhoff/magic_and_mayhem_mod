"""Explicit planning importance, independent of code and execution milestones."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
from collections import Counter
import hashlib
from pathlib import Path
from coverage_claims import behavior_contract

LABELS = {5: 'Critical', 4: 'High', 3: 'Medium', 2: 'Low', 1: 'Supporting'}
WORK_TYPES = {'capability', 'original_recovery', 'aggregate', 'acceptance', 'diagnostic'}
WORK_LABELS = {
    'implemented_within_scope': 'Implemented within scope',
    'partially_implemented': 'Partially implemented',
    'unresolved_aggregate': 'Unresolved aggregate',
    'acceptance_failure': 'Acceptance failure',
    'not_implemented': 'Not implemented',
}
ROOT = Path(__file__).resolve().parents[1]


def task_work_review(review, labels, behavior, root):
    """Static code assessment, independent of historical execution stages."""
    implementation = review.get('implementation')
    if implementation not in set(WORK_LABELS) - {'acceptance_failure'}:
        raise ValueError('Unknown implementation work label')
    if any(labels.get(key, {}).get('label') != label or
           not labels.get(key, {}).get('definition', '').strip()
           for key, label in WORK_LABELS.items()):
        raise ValueError('Work labels need the reviewed definitions')
    gaps = review.get('implementation_gaps')
    if (not isinstance(gaps, list) or
            any(not isinstance(gap, str) or not gap.strip() for gap in gaps) or
            (implementation in {'partially_implemented', 'not_implemented'}) != bool(gaps)):
        raise ValueError('Missing implementation needs a concrete code or recovery gap')
    if (not review.get('current_state', '').strip() or
            review.get('validation_state') not in {'pending', 'historical', 'recorded'} or
            not review.get('validation_summary', '').strip() or
            not review.get('acceptance_summary', '').strip() or
            review.get('acceptance') not in {'pending', 'acceptance_failure'}):
        raise ValueError('Work review needs independent code, validation and acceptance states')
    evidence = review.get('evidence', [])
    if (not evidence or not set(evidence).issubset(behavior['evidence']) or
            (review['acceptance'] == 'acceptance_failure' and not review.get('failure_evidence')) or
            not set(review.get('failure_evidence', [])).issubset(evidence)):
        raise ValueError('Work review needs registered evidence and an explicit failure attribution')
    if review['validation_state'] == 'recorded' and (
            not review.get('validation_evidence') or
            not set(review['validation_evidence']).issubset(evidence)):
        raise ValueError('Recorded bounded validation needs explicit registered execution links')
    steps = review.get('next_steps', [])
    if (not steps or any(step.get('kind') not in {'implementation', 'validation', 'scope_review', 'diagnosis'}
                         or not step.get('action', '').strip() for step in steps) or
            (implementation == 'implemented_within_scope' and any(step['kind'] == 'implementation' for step in steps))):
        raise ValueError('Next steps must distinguish missing code from review and validation')
    sources = review.get('source_sha256', {})
    witnesses = review.get('code_witnesses', [])
    required = set(behavior['implementation']) | set(behavior['tests']) | set(behavior['documents'])
    research_witnesses = review.get('research_witnesses', [])
    if (not sources or not required.issubset(sources) or
            (implementation != 'not_implemented' and not witnesses) or
            (implementation == 'not_implemented' and (witnesses or not research_witnesses))):
        raise ValueError('Work review needs reviewed source fingerprints and code witnesses')
    domains = review.get('scope_domains', [])
    if domains:
        if review.get('scope_resolution_record') not in sources:
            raise ValueError('Scope resolution needs a source-bound review record')
        for domain in domains:
            if (not domain.get('name', '').strip() or
                    domain.get('disposition') not in {'supplied', 'guarded_refusal', 'outside_contract', 'unresolved'} or
                    not domain.get('finding', '').strip() or not domain.get('validation', '').strip() or
                    not domain.get('suppliers') or not domain.get('paths') or
                    not set(domain['paths']).issubset(sources)):
                raise ValueError('Scope domains need supplied/refused/excluded findings and separate validation')
            if domain['disposition'] == 'unresolved' and (
                    implementation != 'unresolved_aggregate' or
                    not domain.get('required_case', '').strip() or
                    not domain.get('closure_condition', '').strip()):
                raise ValueError('Unresolved scope needs its exact required case and closure condition')
    profile_record = review.get('acceptance_profile_record')
    profiles, diagnostics = [], {}
    if profile_record:
        if profile_record not in sources or review.get('acceptance_profile_evidence') not in evidence:
            raise ValueError('Acceptance profiles need a source-bound registered execution record')
        profile_path = (root/profile_record).resolve()
        if (Path(profile_record).is_absolute() or not profile_path.is_relative_to(root.resolve()) or
                Path(profile_record).parts[:2] != ('research', 'runtime') or
                hashlib.sha256(evidence_bytes(profile_path)).hexdigest() != sources[profile_record]):
            raise ValueError('Acceptance profile execution record changed or escaped research')
        import json
        execution = json.loads(evidence_text(profile_path))
        if (execution.get('success') is not True or
                execution.get('sources_stable') is not True or execution.get('inputs_stable') is not True or
                execution.get('policy') != 'critical-rendering-correct-output-v1' or
                execution.get('whole_renderer_accepted') is not False or
                execution.get('all_critical_behaviors_accepted') is not False or
                set(execution.get('accepted_profiles', {})) != {'software', 'hardware'}):
            raise ValueError('Acceptance record must retain its explicit policy and wider boundaries')
        for backend, cohort in execution['accepted_profiles'].items():
            if set(cohort) != {'corrected_storage', 'quick_battle'}:
                raise ValueError('Acceptance profile cohort incomplete')
            for name, count in (('corrected_storage', 222), ('quick_battle', 323)):
                item = cohort[name]
                if item.get('outcome') != 'PASS' or item.get('frames') != count or item.get('mismatches') != 0:
                    raise ValueError('Acceptance profile result is not a complete passing cohort')
                profiles.append(dict(backend=backend, name=name, **item))
        diagnostics = execution.get('legacy_diagnostics', {})
        if (set(diagnostics) != {'software', 'hardware'} or
                any(item.get('outcome') != 'EXPECTED_DIFFERENCE' or item.get('blocking') is not False
                    for item in diagnostics.values())):
            raise ValueError('Acceptance profiles must preserve the legacy diagnostics separately')
    pending = review.get('pending_source_review', [])
    if not isinstance(pending, list) or not set(pending).issubset(sources):
        raise ValueError('Pending review inputs must have explicit source links')
    changed = []
    for name, expected in sources.items():
        path = (root / name).resolve()
        if (Path(name).is_absolute() or not path.is_relative_to(root.resolve()) or
                Path(name).parts[0] in {'original', 'working', '.git'} or
                len(expected) != 64 or any(c not in '0123456789abcdef' for c in expected)):
            raise ValueError('Invalid work review source')
        if not path.is_file() or hashlib.sha256(evidence_bytes(path)).hexdigest() != expected:
            changed.append(name)
    for witness in witnesses:
        name = witness['path']; line = witness['line']; text = witness['text']
        if (name not in sources or name not in behavior['implementation'] or
                type(line) is not int or line < 1 or not text.strip()):
            raise ValueError('Invalid implementation witness')
        if name not in changed:
            lines = evidence_text(root / name).splitlines()
            if line > len(lines) or lines[line - 1] != text:
                raise ValueError('Implementation witness differs from reviewed source')
    for witness in research_witnesses:
        name = witness['path']; line = witness['line']; text = witness['text']
        if (name not in sources or name not in set(behavior['documents']) | set(behavior['tests']) or
                type(line) is not int or line < 1 or not text.strip()):
            raise ValueError('Invalid research witness')
        if name not in changed:
            lines = evidence_text(root / name).splitlines()
            if line > len(lines) or lines[line - 1] != text:
                raise ValueError('Research witness differs from reviewed source')
    return {**review, 'acceptance_profiles': profiles, 'legacy_diagnostics': diagnostics,
            'label': labels[implementation]['label'],
            'freshness': 'historical' if changed or pending else 'current', 'changed_sources': changed}


def tranche_view(plan, register, rated, root=ROOT, *, priority=5, size=11, count=3):
    """Validate ordered work batches without claiming execution."""
    if plan.get('state') != 'planned' or not plan.get('completion_rule', '').strip():
        raise ValueError('Tranche metadata must describe a plan, not execution')
    tranches = plan['tranches']
    if plan.get('review_document'):
        document = Path(plan['review_document'])
        path = (Path(root) / document).resolve()
        if (document.is_absolute() or '..' in document.parts or
                document.parts[:2] != ('research', 'runtime') or document.suffix != '.md' or
                not path.is_relative_to((Path(root) / 'research/runtime').resolve()) or
                not path.is_file()):
            raise ValueError('Plan review document must be existing local runtime research')
    if len(tranches) != count or [t['order'] for t in tranches] != list(range(1,count+1)):
        raise ValueError('Expected ordered tranches: '+str(count))
    by_id = {b['id']: b for b in register['behaviors']}
    seen = set(); earlier = set(); result = []
    for tranche in tranches:
        if (tranche['id'] in earlier or len(tranche['tasks']) != size or
                not set(tranche['depends_on']).issubset(earlier) or
                not all(tranche.get(k, '').strip() for k in ('title', 'goal', 'exit_gate'))):
            raise ValueError('Invalid tranche size, order or prerequisite')
        tasks = []; prior_tasks = set()
        if 'execution_review' in tranche:
            review = tranche['execution_review']
            if (not review.get('scope', '').strip() or not review.get('run_rules') or
                    not review.get('result_contract')):
                raise ValueError('Execution review needs bounded run and result rules')
        for task in tranche['tasks']:
            bid = task['behavior']
            if bid in seen or bid not in rated:
                raise ValueError('Duplicate, unknown or unrated tranche behavior: ' + bid)
            seen.add(bid); b = by_id[bid]; rating = rated[bid]
            reviewed_statuses = {'partial', 'scoped'} if priority in {2, 5} else {'partial', 'none'}
            if (task.get('reviewed_implementation') not in reviewed_statuses or
                    not all(task.get(k, '').strip() for k in (
                        'mode', 'next_action', 'implementation_boundary', 'validation',
                        'exit_condition', 'remaining_at_review'))):
                raise ValueError('Task needs concrete actions, checks and remaining boundary')
            if (not task.get('checks') or not task.get('documents') or
                    not set(task['checks']).issubset(b['tests']) or
                    not set(task['documents']).issubset(b['documents'])):
                raise ValueError('Task checks and research must be registered links')
            brief = task.get('execution_brief')
            if 'execution_review' in tranche and not brief:
                raise ValueError('Reviewed tranche task needs an execution brief')
            if brief:
                if (not set(brief['requires']).issubset(prior_tasks) or
                        len(brief['requires']) != len(set(brief['requires']))):
                    raise ValueError('Task prerequisite must precede it in this tranche')
                if (not brief.get('inputs') or not brief.get('runners') or
                        not brief.get('acceptance') or not brief.get('handoff', '').strip() or
                        not isinstance(brief.get('fixture_gaps'), list) or
                        any(not isinstance(v, str) or not v.strip()
                            for key in ('inputs', 'acceptance', 'fixture_gaps') for v in brief[key])):
                    raise ValueError('Execution brief needs inputs, entrypoints and pass criteria')
                for runner in brief['runners']:
                    if (runner.get('path') not in task['checks'] or
                            not runner.get('command', '').strip() or
                            not runner.get('coverage', '').strip() or
                            runner.get('claims_binding') not in {'supported', 'needs_prospective_wrapper'}):
                        raise ValueError('Entrypoint needs registered link, template and evidence limit')
            # Ratings/scopes/statuses can change after the plan is written.
            # Flag the plan; never silently mark a task executed or complete.
            current = (task.get('contract_sha256') == rating['contract_sha256'] and
                       rating['review_state'] == 'current' and rating['weight'] == priority and
                       b['status']['implementation'] == task['reviewed_implementation'])
            work = task.get('work_review')
            if tranche.get('work_review_required') and not work:
                raise ValueError('Reviewed tranche needs an implementation work assessment per task')
            if work:
                work = task_work_review(work, plan.get('work_labels', {}), b, Path(root))
                if any(not set(domain['suppliers']).issubset(by_id)
                       for domain in work.get('scope_domains', [])):
                    raise ValueError('Scope resolution references an unknown supplier contract')
                current = current and work['freshness'] == 'current'
            tasks.append({**task, **({'work_review': work} if work else {}),
                          'review_state': 'current' if current else 'needs_review',
                          'implementation': b['status']['implementation']})
            prior_tasks.add(bid)
        result.append({**tranche, 'tasks': tasks})
        earlier.add(tranche['id'])
    unfinished = {e['behavior'] for e in rated.values()
                  if e['weight'] == priority and e['implementation'] in
                  ({'partial', 'none'} if priority == 4 else {'partial'})}
    return {**plan, 'tranches': result, 'selected_count': len(seen),
            ('unplanned_low' if priority == 2 else 'unplanned_high' if priority == 4 else 'unplanned_critical'): sorted(unfinished - seen)}


def importance_view(catalog, register, tracks, root=ROOT):
    """Validate curated weights; new contracts stay unrated, changed scopes stale.

    This is a planning judgment about the documented contract, not an execution
    claim. Never infer completion fractions, dependencies or implementation work.
    """
    if catalog.get('schema') != 1 or not catalog.get('purpose', '').strip():
        raise ValueError('Invalid importance catalog')
    track = next((t for t in tracks if t['id'] == catalog['track']), None)
    if track is None:
        raise ValueError('Unknown importance track')
    by_id = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    entries = []; seen = set()
    for item in catalog['entries']:
        bid = item['behavior']; weight = item['weight']
        if bid in seen or bid not in by_id or bid not in track['behaviors']:
            raise ValueError('Duplicate, unknown or out-of-track importance behavior: ' + bid)
        seen.add(bid)
        if type(weight) is not int or weight not in LABELS:
            raise ValueError('Importance must be an integer from 1 to 5')
        if (not item.get('reason', '').strip() or
                item.get('work_type') not in WORK_TYPES):
            raise ValueError('Importance needs a reason and explicit work type')
        contract = item.get('contract_sha256', '')
        if len(contract) != 64 or any(c not in '0123456789abcdef' for c in contract):
            raise ValueError('Importance needs a reviewed contract fingerprint')
        b = by_id[bid]
        entries.append({**item, 'label': LABELS[weight],
                        'review_state': 'current' if behavior_contract(b, builds) == contract else 'needs_review',
                        'implementation': b['status']['implementation']})
    entries.sort(key=lambda e: (-e['weight'], e['behavior']))
    result = {**{k: v for k, v in catalog.items() if k != 'entries'},
              'entries': entries, 'labels': LABELS,
              'unrated': sorted(set(track['behaviors']) - seen),
              'partial_counts': dict(Counter(e['weight'] for e in entries if e['implementation'] == 'partial'))}
    if 'tranche_plan' in catalog:
        result['tranche_plan'] = tranche_view(catalog['tranche_plan'], register,
                                            {e['behavior']: e for e in entries}, root)
    if 'high_tranche_plan' in catalog:
        result['high_tranche_plan'] = tranche_view(catalog['high_tranche_plan'], register,
                                                 {e['behavior']: e for e in entries}, root,
                                                 priority=4, size=17)
        critical_ids = {task['behavior'] for tranche in result.get('tranche_plan', {}).get('tranches', [])
                        for task in tranche['tasks']}
        high_ids = {task['behavior'] for tranche in result['high_tranche_plan']['tranches']
                    for task in tranche['tasks']}
        if critical_ids & high_ids:
            raise ValueError('A behavior cannot be assigned to both priority plans')
    if 'low_tranche_plan' in catalog:
        result['low_tranche_plan'] = tranche_view(catalog['low_tranche_plan'], register,
                                                 {e['behavior']:e for e in entries},root,
                                                 priority=2,size=20,count=1)
        selected = [t['behavior'] for tr in result['low_tranche_plan']['tranches'] for t in tr['tasks']]
        native = {b['id'] for b in register['behaviors'] if b.get('subsystem') == 'native_rendering'}
        current_low = sorted(e['behavior'] for e in entries if e['weight']==2 and e['behavior'] in native)
        # Preserve the reviewed batch when a merge adds an earlier sorting ID.
        # New contracts need their own assignment and cannot displace old results.
        cohort = catalog['low_tranche_plan'].get('selection_cohort', current_low)
        if (not isinstance(cohort, list) or len(cohort) < 20 or
                any(not isinstance(bid, str) for bid in cohort) or
                cohort != sorted(set(cohort)) or not set(cohort).issubset(native)):
            raise ValueError('Low batch needs an ordered, unique reviewed native cohort')
        if selected != cohort[:20]:
            raise ValueError('Low batch must select the first20 reviewed native rendering behaviors in UI order')
        result['low_tranche_plan']['unplanned_low'] = sorted(set(current_low) - set(selected))
    return result
