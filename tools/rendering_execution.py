"""Validate the rendering execution DAG; static readiness never closes acceptance."""
from collections import Counter
from pathlib import Path
import subprocess

from coverage_claims import fingerprint
from coverage_evidence import Reader
from rendering_completion import PLAN, require
from rendering_gaps import review as review_gaps, unique

REQUIRED = {f'EP{i:02}' for i in range(1, 12)}
OPTIONAL = {'EP12'}
RULES = {'pilot_before_broad_execution', 'required_unknown_blocks_affected_claim_only',
         'reuse_requires_requirement_source_caller_config_scope',
         'fix_requires_reproduced_required_defect_or_concrete_missing_capability',
         'failures_preserved_and_pilot_rerun_after_shared_changes',
         'normal_fault_and_original_recovery_separate', 'no_fixed_fps_completion_gate',
         'no_runtime_port_without_required_contract', 'no_static_task_completion_or_engine_promotion'}
WITNESSES = {'tools/drawing_journeys.py', 'tools/drawing_session.py', 'tools/drawing_recorded_session.py',
             'tools/drawing_action_session.py', 'tools/check-drawing-journeys.py',
             'tools/retain-drawing-session.py', 'tests/fixtures/drawing-journeys/suite.json',
             'tests/test-drawing-journeys.py', 'tests/test-drawing-session.py',
             'apps/drawing-session/main.cpp', 'tools/test-native-owned-campaign.py',
             'tools/test-native-campaign.py'}
PACKS = {'entry-media', 'world-interaction', 'storage-display', 'campaign-next-map',
         'victory', 'defeat', 'fault-lifecycle'}
PREPARATORY_GUARDS = [['python3', '-B', 'tests/test-drawing-journeys.py'],
                      ['python3', '-B', 'tests/test-drawing-session.py']]


def schedule(tasks):
    """Return dependency waves without treating planned tasks as executed."""
    done, waves = set(), []
    while len(done) < len(tasks):
        ready = sorted(k for k, t in tasks.items() if k not in done and set(t['depends_on']) <= done)
        require(ready, 'Execution dependencies contain a cycle')
        waves.append(ready)
        done.update(ready)
    return waves


def validate(plan):
    execution, gaps, contract = plan['execution_plan'], plan['required_gap_register'], plan['completion_contract']
    require(execution['schema'] == 1 and execution['revision'] == 1
            and execution['id'] == 'native-rendering-execution-v1', 'Unsupported execution plan')
    require(execution['state'] == 'planned_not_executed', 'A static plan cannot claim execution')
    for name, subject in [('completion_contract', contract), ('required_gap_register', gaps)]:
        require(execution[name] == dict(id=subject['id'], revision=subject['revision'], sha256=fingerprint(subject)),
                'Execution input binding differs: ' + name)
    require(execution['baseline'] == contract['baseline']
            and execution['reviewed_source_commit'] == contract['reviewed_source_commit'], 'Execution baseline differs')
    require(execution['strategy'] == 'evidence_led_dependency_and_reuse_heuristic'
            and execution['time_optimality_proved'] is False and execution['objective'].strip()
            and execution['optimality'].strip(), 'Unmeasured optimality cannot become a completion claim')
    require(set(execution['scheduling_rules']) == RULES
            and all(v is True for v in execution['scheduling_rules'].values()), 'Scheduling rule weakened')
    require(set(execution['reviewed_sources']) == WITNESSES, 'Execution source witness set changed')
    unique(execution['tasks'], REQUIRED | OPTIONAL, 'execution task')
    tasks = {t['id']: t for t in execution['tasks']}
    briefs = {b['id'] for b in gaps['task_briefs']}
    owners = {}
    for tid, task in tasks.items():
        require(task['state'] == 'planned' and task['execution_evidence'] == [],
                'Static task cannot claim execution or closure: ' + tid)
        require(task['required_for_completion'] is (tid in REQUIRED), 'Optional diagnostic entered completion path')
        require(task['engine_owner'] == 'engineering', 'Technical work cannot become an automatic user blocker')
        require(all(isinstance(task[k], str) and task[k].strip() for k in
                    ('title', 'kind', 'action', 'exit_condition', 'scheduling_reason', 'on_failure'))
                and task['deliverables'] and all(v.strip() for v in task['deliverables'])
                and task['reuse_paths'], 'Execution task lacks an action, artifact, exit or reuse path: ' + tid)
        deps = task['depends_on']
        require(len(deps) == len(set(deps)) and set(deps) <= set(tasks) and tid not in deps,
                'Unknown, duplicate or self dependency: ' + tid)
        require(not (tid in REQUIRED and set(deps) & OPTIONAL), 'Diagnostic dependency blocks mandatory work')
        require(len(task['closing_briefs']) == len(set(task['closing_briefs'])), 'Duplicate closing brief')
        for brief in task['closing_briefs']:
            require(brief in briefs and brief not in owners, 'Unknown or multiply owned closing brief: ' + brief)
            owners[brief] = tid
    require(set(owners) == briefs, 'Closing brief is missing an execution owner')
    require(owners['RG.diagnostics'] == 'EP12' and owners['RG.closure-decisions'] == 'EP11',
            'Diagnostic and artifact 5 handoff boundaries changed')
    waves = schedule(tasks)

    def ancestors(tid):
        found, pending = set(), list(tasks[tid]['depends_on'])
        while pending:
            key = pending.pop()
            if key not in found:
                found.add(key)
                pending.extend(tasks[key]['depends_on'])
        return found

    require(ancestors('EP03') == {'EP01', 'EP02'},
            'Pilot must follow input/requiredness triage, not global scope resolution')
    for tid in ('EP04', 'EP05', 'EP06', 'EP08', 'EP09'):
        require('EP03' in ancestors(tid), 'Broad execution must wait for the acceptance pilot: ' + tid)
    require('EP04' in ancestors('EP09'), 'Fault tests require a healthy normal lifetime')
    require(set(tasks['EP07']['depends_on']) == {'EP02', 'EP04', 'EP05', 'EP06'}
            and set(tasks['EP08']['depends_on']) == {'EP04', 'EP05', 'EP06'},
            'Final scope closure and software deltas must consume actual primary evidence')
    require(set(tasks['EP10']['depends_on']) == {'EP07', 'EP08', 'EP09'}
            and set(tasks['EP11']['depends_on']) == {'EP10'}
            and REQUIRED - {'EP10', 'EP11'} <= ancestors('EP10'),
            'Final join or artifact 5 handoff omits required work')
    order = execution['recommended_order']
    require(len(order) == len(REQUIRED) and set(order) == REQUIRED
            and execution['optional_tasks'] == ['EP12'], 'Recommended order omits or adds mandatory work')
    positions = {t: i for i, t in enumerate(order)}
    require(all(positions[d] < positions[t] for t in order for d in tasks[t]['depends_on']),
            'Recommended order contradicts dependencies')
    require(execution['first_task'] == order[0] == 'EP01' and tasks['EP01']['depends_on'] == []
            and [t for t in order if not tasks[t]['depends_on']] == ['EP01'], 'First executable task is ambiguous')
    first = execution['first_action']
    require(first['task'] == execution['first_task'] and first['instruction'].strip()
            and first['output'].strip() and first['limits'].strip() and first['starting_inputs']
            and first['source_only_checks'] == PREPARATORY_GUARDS, 'First action or preparatory guard scope differs')
    unique(execution['session_packs'], PACKS, 'candidate session pack')
    configurations = {c['id'] for c in contract['configurations']}
    journeys = {j['id'] for j in gaps['journeys']}
    covered = set()
    for pack in execution['session_packs']:
        require(pack['state'] == 'candidate_not_executed' and pack['task'] in {'EP04', 'EP05', 'EP09'}
                and set(pack['configurations']) == configurations and pack['route'].strip() and pack['proof'].strip(),
                'Candidate pack cannot omit configuration or manufacture coverage')
        require(len(pack['journeys']) == len(set(pack['journeys'])) and set(pack['journeys']) <= journeys,
                'Unknown or duplicate packed journey')
        covered.update(pack['journeys'])
    require(covered == journeys, 'Candidate packs omit a required journey')
    choices = {p['id']: p for p in execution['product_choices']}
    unique(execution['product_choices'], {'PC.domain', 'PC.expansion'}, 'product choice')
    for row in choices.values():
        require(row['blocks_plan'] is False and row['contract_amendment_required_for_target_change'] is True
                and row['topic'].strip() and row['recommendation'].strip() and row['ask_user_when'].strip(),
                'Product choice cannot silently change target or block the plan')
    decisions = {d['id']: d for d in contract['scope_decisions']}
    unique(execution['scope_routes'], decisions, 'scope decision route')
    for row in execution['scope_routes']:
        decision = decisions[row['id']]
        require(row['state'] == 'unresolved' and row['technical_owner'] == 'engineering'
                and row['triage_task'] == 'EP02' and row['resolution_task'] == 'EP07'
                and row['blocking_condition'] == decision['blocking_condition']
                and row['close_when'] == decision['close_when'] and row['user_choice'] in (None, *choices),
                'Required scope condition, ownership or resolution was lost')
        require(owners['RG.' + row['id']] == row['resolution_task'], 'Scope closing brief has the wrong owner')
    evidence = execution['evidence_contract']
    require(all(evidence[k].strip() for k in ('storage', 'source_binding', 'acceptance', 'invalidation'))
            and evidence['repair_loop'] == ['retain_new_failure_receipt', 'classify_requiredness_and_provenance',
                'isolate_smallest_concrete_gap', 'implement_bounded_fix_if_needed',
                'rerun_changed_proof_and_affected_consumers', 'rejoin_with_new_evidence_ids'],
            'Failure retention, bounded repair or evidence invalidation rule changed')
    required_waves = [[t for t in wave if t in REQUIRED] for wave in waves]
    return dict(schema=1, success=True, kind='static_rendering_execution_plan_review',
        contract=contract['id'], execution_plan=execution['id'],
        counts=dict(tasks=len(tasks), required_tasks=len(REQUIRED), optional_tasks=len(OPTIONAL),
                    closing_briefs=len(owners), candidate_session_packs=len(PACKS),
                    scope_decisions=len(decisions), product_choice_categories=len(choices)),
        task_owners=owners, dependency_waves=[w for w in required_waves if w],
        recommended_order=order, initial_ready_tasks=['EP01'], next_action=first,
        optimality_proved=False, task_execution_performed=False, engine_execution_performed=False,
        original_media_consumed=False, original_code_executed=False, engine_acceptance_assessed=False,
        native_equivalence=False, complete_replacement=False, status_promotions=[],
        scope='Static dependency, reuse, requiredness and acceptance-pilot plan. No task is executed or closed, no original/native/game result is acquired and no engine status is promoted. Dependency waves are not measured elapsed-time critical paths.')


def review(root):
    root = Path(root).resolve()
    prerequisite = review_gaps(root)
    with Reader(root) as reader:
        plan = reader.json(PLAN)
        execution = plan['execution_plan']
        report = validate(plan)
        sources = dict(prerequisite['sources'])
        paths = set(execution['reviewed_sources'])
        paths.update(p for t in execution['tasks'] for p in t['reuse_paths'])
        paths.update(execution['first_action']['starting_inputs'])
        paths.update({'tools/rendering_execution.py', 'tests/test-rendering-execution.py',
                      'docs/native-rendering-execution.md', 'research/runtime/rendering-execution-plan-20261030.md'})
        for name in sorted(paths):
            path = Path(name)
            require(not path.is_absolute() and '..' not in path.parts
                    and path.parts[0] not in ('original', 'working', '.git')
                    and (root/path).is_file(), 'Missing or unsafe execution input: ' + name)
            sources[name] = reader.digest(name)
        for name, expected in execution['reviewed_sources'].items():
            require(sources[name] == expected, 'Reviewed execution tooling changed: ' + name)
            for commit in (execution['reviewed_source_commit'], execution['planning_base_commit']):
                data = subprocess.check_output(['git', '-C', str(root), 'show', commit + ':' + name])
                import hashlib
                require(hashlib.sha256(data).hexdigest() == expected,
                        'Execution tooling differs from reviewed revision: ' + name)
        catalog = reader.json(plan['scope']['journey_catalog'])
        adapters = Counter(c['adapter'] for c in catalog['cases'])
        report.update(sources=sources, prerequisite_review_passed=prerequisite['success'],
            completion_contract=execution['completion_contract'], required_gap_register=execution['required_gap_register'],
            baseline=execution['baseline'], reviewed_source_commit=execution['reviewed_source_commit'],
            planning_base_commit=execution['planning_base_commit'], execution_tooling_witnesses_verified=True,
            suite_frontier=dict(case_adapters=dict(adapters), generic_session_cases=adapters['session'],
                generic_admission_and_backend_interface_exists=True,
                complete_journey_acceptance_established=False,
                scope='Existing recorded-action cases retain observed-EGL boundaries; admission/ABI exist but the current catalog has no generic presented-frame session case. Pilot must bind actual public-player integration rather than relabel these cases.'),
            evidence_applicability_seed=prerequisite['evidence_leads'],
            seed_is_not_requirement_acceptance=True, authored_plan=execution)
        return report
