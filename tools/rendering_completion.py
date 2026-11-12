"""Validate the authored completion contract without granting engine acceptance."""
import hashlib
from pathlib import Path
import subprocess

from coverage_evidence import Reader
from coverage_audit import pointer
from drawing_journeys import REQUIREMENTS, OPERATIONS, BUILD
from native_only_contract import POLICY, configure

PLAN = 'docs/native-rendering-closure.json'
DECISIONS = 'docs/native-rendering-dependencies.json'
GATES = {'identity', 'pixels-history', 'journeys', 'operations', 'transitions',
         'input-media', 'normal-lifetime', 'fault-lifetime', 'required-boundaries', 'separate-decisions'}
DECISION_IDS = {'SC.alternate-ownership', 'SC.padded-fullscreen', 'SC.natural-callers',
                'SC.prior-history', 'SC.map-save-domain', 'SC.screen-provider-lifetime',
                'SC.rgb555-live', 'SC.physical-compatibility'}
EVIDENCE_RULES = {'current_source_bindings_required', 'independent_reference_required',
    'component_proof_is_not_journey_acceptance', 'admission_is_not_complete_acceptance',
    'normal_and_fault_outcomes_separate', 'unexpected_refusal_blocks_normal_acceptance',
    'expected_negative_cases_require_state_and_return_checks', 'disabled_media_cannot_close_enabled_movies',
    'historical_hashes_immutable', 'no_status_promotion_from_static_review'}
CHANGE_RULES = {'contract_amendment_requires_exact_review', 'required_unknowns_cannot_be_silently_excluded',
    'new_evidence_id_for_new_execution', 'baseline_payload_immutable', 'acceptance_and_recovery_decisions_separate'}
POLICY_SOURCES = {'run-native.sh', 'run-x86.sh', 'tools/run-native-game.py', 'tools/run-native-owned.py',
    'tools/native_only_contract.py', 'tools/native_output_owner.py', 'tools/terrain_light_session.py',
    'apps/qt-shell/CMakeLists.txt', 'apps/qt-shell/main.cpp',
    'compat/legacy/world_frame.cpp', 'compat/legacy/world_frame.hpp'}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(plan, decisions, suite, baseline):
    contract = plan['completion_contract']
    require(contract['schema'] == 1 and contract['revision'] == 1
            and contract['id'] == 'native-rendering-completion-v1', 'Unsupported completion contract')
    require(contract['claim_state'] == 'pending_acceptance', 'A static completion contract cannot certify acceptance')
    require(baseline['kind'] == 'reviewed_rendering_baseline' and baseline['success'] is True
            and baseline['static_review_only'] is True and baseline['engine_execution_performed'] is False,
            'Expected the reviewed static baseline')
    require(contract['reviewed_source_commit'] == contract['baseline']['source_commit']
            == plan['reviewed_commit'] == baseline['subject']['commit'], 'Baseline source revision differs')
    require(plan['original_sha256'] == suite['source_sha256'] == BUILD
            and plan['original_build'] == 'nocd-40209ca7', 'Original build differs from the journey contract')
    require(suite['requirements'] == {k: list(v) for k, v in REQUIREMENTS.items()}
            and suite['operation_requirements'] == list(OPERATIONS), 'Journey/operation denominator changed')
    require({j['id']: j['requirements'] for j in plan['journeys']} == suite['requirements']
            and [o['id'] for o in plan['operations']] == list(OPERATIONS), 'Closure requirements differ')
    acceptance = contract['acceptance']
    require(acceptance['all_journeys_required'] is True and acceptance['all_operations_required'] is True,
            'Completion cannot waive existing journeys or operations')
    require(acceptance['journey_catalog'] == plan['scope']['journey_catalog']
            and acceptance['journey_catalog_sha256'] == plan['scope']['journey_catalog_sha256'],
            'Acceptance catalog differs')
    require(len(acceptance['gates']) == len(GATES) and {g['id'] for g in acceptance['gates']} == GATES
            and all(g['condition'].strip() for g in acceptance['gates']), 'Missing or duplicate completion gate')
    for field, keys in [('evidence_rules', EVIDENCE_RULES), ('change_control', CHANGE_RULES)]:
        require(set(contract[field]) == keys and all(v is True for v in contract[field].values()),
                'Missing or weakened completion rule: ' + field)
    performance = plan['scope']['performance_policy']
    require(performance['fps'] == 'diagnostic' and performance['minimum_fps_required'] is False
            and performance['response_deadlines_required'] is True
            and performance['resource_and_progress_checks_required'] is True,
            'FPS policy or functional completion bounds changed')
    require(plan['scope']['exclusions'] == baseline['baseline_scope']['exclusions'],
            'Existing exclusions cannot silently expand or shrink')
    roles = {p['id']: p['role'] for p in plan['scope']['profiles']}
    require(roles == {'public-resident': 'validation_harness', 'strict-native-only': 'primary_player',
                     'isolated-recovery': 'separate_recovery_profile'}, 'Profile boundaries changed')
    require(contract['primary_profile'] == 'strict-native-only' and contract['runtime_policy'] == POLICY
            and contract['player_entries'] == ['run-native.sh', 'run-x86.sh'], 'Primary player admission differs')
    configurations = contract['configurations']
    require(len(configurations) == 2 and {c['id'] for c in configurations} == {'player-default', 'player-software'},
            'Missing or duplicate player configuration')
    for row in configurations:
        require(row['state'] == 'target_acceptance_pending' and row['profile'] == contract['primary_profile']
                and row['world_format'] == 'RGB565' and row['frame_sizes'] == [[640, 480], [800, 600]]
                and row['lighting'] == 'native' and row['lighting_capture'] == 'none'
                and row['movies'] == 'native_enabled', 'Configuration cannot claim unproved or disabled coverage')
        require(row['arguments'] == ([] if row['id'] == 'player-default' else ['--software-rendering']),
                'Configuration differs from the reviewed public entry')
        configure({}, row['arguments'])  # Pure admission only; never stages or launches anything.
        require(all(isinstance(row[k], str) and row[k].strip() for k in ('host', 'display', 'backend', 'music')),
                'Configuration identity is incomplete')
    suppliers = contract['supplier_policy']
    require(suppliers['decisions'] == DECISIONS and decisions['completion_contract'] == contract['id']
            and suppliers['original_preparation_allowed'] is True
            and suppliers['original_pixel_fallback_allowed'] is False
            and suppliers['native_scene_port_required'] is False
            and suppliers['lighting_default'] == 'native'
            and suppliers['lighting_overrides'] == ['original', 'observe']
            and suppliers['lighting_worker_failure'].strip(), 'Supplier or fallback policy changed')
    require({r['contract']: r['role'] for r in decisions['groups']}
            == {r['contract']: r['role'] for r in baseline['source_role_decisions']},
            'Required/retained original service roles changed without a new boundary review')
    expected_sources = POLICY_SOURCES | {p for row in baseline['source_role_decisions']
                                        for p in row['source_witnesses']}
    require(set(contract['reviewed_sources']) == expected_sources,
            'Policy/source witness set cannot silently shrink or change')
    groups = {g['id'] for g in plan['contracts']}
    rows = contract['scope_decisions']
    require(len(rows) == len(DECISION_IDS) and {d['id'] for d in rows} == DECISION_IDS,
            'A required scope decision disappeared or was duplicated')
    for row in rows:
        require(set(row['contracts']) <= groups and row['contracts']
                and row['blocking_condition'].strip() and row['close_when'].strip() and row['documents'],
                'Incomplete scope decision: ' + row['id'])
        require(row['state'] in ('unresolved', 'resolved'), 'Scope decision must be explicit')
        if row['state'] == 'resolved':
            require(row['resolution_evidence'] and row.get('resolution_reason', '').strip()
                    and row.get('resolved_boundary', '').strip(), 'Resolved decision needs evidence and an exact boundary')
        else:
            require(row['resolution_evidence'] == [], 'An unresolved decision cannot claim resolution evidence')
    return dict(schema=1, success=True, kind='static_rendering_completion_contract_review',
        contract=contract['id'], contract_revision=contract['revision'],
        reviewed_source_commit=contract['reviewed_source_commit'],
        counts=dict(journeys=len(REQUIREMENTS), operations=len(OPERATIONS), gates=len(GATES),
                    configurations=len(configurations), unresolved_scope_decisions=sum(d['state'] == 'unresolved' for d in rows)),
        engine_execution_performed=False, original_media_consumed=False, engine_acceptance_assessed=False,
        original_code_executed=False, native_equivalence=False, complete_replacement=False,
        status_promotions=[], scope='Authored target, supplier/configuration boundaries and completion rules only. No new original/native/game execution, acceptance or status promotion.')


def review(root, *, include_scope_review=True):
    root = Path(root).resolve()
    with Reader(root) as reader:
        plan, decisions = reader.json(PLAN), reader.json(DECISIONS)
        contract = plan['completion_contract']
        spec = contract['baseline']
        require(reader.digest(spec['record']) == spec['sha256'], 'Immutable baseline payload differs')
        baseline = reader.json(spec['record'])
        suite_path = plan['scope']['journey_catalog']
        require(reader.digest(suite_path) == plan['scope']['journey_catalog_sha256'], 'Acceptance catalog hash differs')
        report = validate(plan, decisions, reader.json(suite_path), baseline)
        witnesses = contract['reviewed_sources']
        require(witnesses, 'Missing source-reviewed policy witnesses')
        for name, expected in witnesses.items():
            path = Path(name)
            require(not path.is_absolute() and '..' not in path.parts
                    and path.parts[0] not in ('original', 'working', '.git'), 'Unsafe source witness')
            require(reader.digest(name) == expected, 'Reviewed policy source changed: ' + name)
            old = subprocess.check_output(['git', '-C', str(root), 'show', spec['source_commit'] + ':' + name])
            require(hashlib.sha256(old).hexdigest() == expected, 'Witness differs from the baseline source: ' + name)
        for decision in contract['scope_decisions']:
            for name in decision['documents']:
                require(not Path(name).is_absolute() and '..' not in Path(name).parts
                        and (root/name).resolve().is_relative_to(root) and (root/name).is_file(),
                        'Missing scope-decision research: ' + name)
        resolved_ids = {eid for d in contract['scope_decisions'] for eid in d['resolution_evidence']}
        resolution_records = []
        if resolved_ids:
            evidence = {e['id']: e for e in reader.json('research/runtime/coverage/register.json')['evidence']}
            for eid in sorted(resolved_ids):
                require(eid in evidence, 'Unknown scope-resolution evidence: ' + eid)
                entry = evidence[eid]
                require(reader.digest(entry['record']) == entry['sha256'], 'Scope-resolution payload differs: ' + eid)
                payload = reader.json(entry['record'])
                require(all(pointer(payload, a['pointer']) == a['equals'] for a in entry['assertions']),
                        'Scope-resolution evidence assertion failed: ' + eid)
                resolution_records.append(dict(id=eid, record=entry['record'], sha256=entry['sha256']))
        paths = [PLAN, DECISIONS, suite_path, 'docs/native-rendering-closure.md',
                 'docs/native-player-launch.md', 'docs/live-drawing-replacement.md', 'tools/rendering_completion.py',
                 'tools/reconcile-rendering-dependencies.py', 'tests/test-rendering-completion.py', *witnesses]
        report.update(baseline=dict(spec, payload_verified=True), policy_witnesses_match_baseline=True,
                      sources={p: reader.digest(p) for p in paths},
                      scope_resolution_records=resolution_records,
                      authored_decisions=contract['scope_decisions'], configurations=contract['configurations'])
        if include_scope_review and 'scope_review' in plan:
            from rendering_scope import load
            report['scope_review'] = load(root, reader, plan)
            report['sources'].update({p: reader.digest(p) for p in
                                     ['tools/rendering_scope.py', plan['scope_review']['record']]})
        return report
