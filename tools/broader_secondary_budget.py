"""Preserve the failed interaction charge and bound one reviewed runtime replacement."""
from broader_discovery_budget import bound as parent_bound, predecessors as parent_predecessors
from broader_player_acceptance import checked
from rendering_pilot import require
P = 'research/runtime/broader-player-acceptance-results-20261110/'
FAILED = 'bp40-world-interaction-default800-01'
PILOT = 'bp44-world-interaction-default800-01'


def validate_rows(rows, old, new):
    fixed = {v['case_id']: v for v in old}
    require(len(old) == len(fixed) == 153 and sum(v['reserved_games'] for v in old) == 202,
            'Changed153/202 historical domain')
    require(len(new) == 45 and sum(new.values()) == 45 and set(fixed).isdisjoint(new)
            and all(type(v) is int and v == 1 for v in new.values())
            and new.get(PILOT) == 1 and FAILED not in new, 'Changed45/45 replacement continuation')
    minimal = {'case_id', 'report', 'reserved_games', 'state', 'acquisition_sha256'}
    full = minimal | {'plan', 'started_ns', 'validation'}
    seen, total = set(), 0
    for row in rows:
        cid = row['case_id']
        require(set(row) in (minimal, full) and cid not in seen, 'Unknown/duplicate request')
        seen.add(cid)
        if cid in fixed:
            require(all(fixed[cid][k] == v for k, v in row.items()), 'Historical request rewritten')
        else:
            require(cid in new and row['state'] == 'admitted' and row['reserved_games'] == new[cid]
                    and row['acquisition_sha256'] and cid in row['report'],
                    'Stop after failed/incomplete new request')
        total += row['reserved_games']
    require(set(fixed) <= seen and total <= 247, 'Historical row omitted or247ceiling exceeded')
    return total


def review(reader, plan):
    q = checked(reader, plan['secondary_caller_review'])
    require(q['kind'] == 'reviewed_secondary_caller_runtime_continuation'
            and q['original_game_runs'] == q['new_native_executions'] == 0, 'Wrong runtime continuation review')
    require(q['retained_current_cases'] == q['remaining_positive_cases'] == q['remaining_positive_original_starts'] == 45
            and q['new_additional_ceiling'] == 247 and q['new_combined_ceiling'] == 255
            and q['first_pilot'] == PILOT and q['positive_contexts'] == 90 and q['positive_original_starts'] == 98,
            'Changed finite replacement domain')
    require(q['reservations_refunded'] is False and q['automatic_retries'] is False
            and q['source_waivers'] == [] and q['Native_ELF_or_base_runtime_sources_changed'] is False
            and q['derived_runtime_variant_explicit'] is True, 'Unreviewed source/charge waiver')
    parent = checked(reader, q['parent_global_plan'])
    require(parent_bound(reader, parent) == 246, 'Prior finite ceiling changed')
    failure = checked(reader, q['direct_refusal_diagnosis'])
    row = failure['failed_request']
    require(row in q['old_requests'] and row['case_id'] == FAILED and row['state'] == 'failed'
            and row['reserved_games'] == 1 and row['acquisition_sha256'] == failure['acquisition']['sha256']
            and failure['failed_actual_Original_starts'] == 1
            and failure['direct_failure'] == dict(World=228, sequence=2147, code=43, tag=12,
                caller='0x500a16', caller_rva='0x100a16', Original_exit=95, request_published=False),
            'Actual failed runtime row changed/promoted')
    require(parent_predecessors(reader, [v for v in q['old_requests'] if v['case_id'] != FAILED], parent) == 201,
            'Parent history changed')
    custody = checked(reader, q['negative_custody'])
    require(custody['case'] == FAILED and custody['case_accepted'] is False
            and custody['requests_unchanged'] and not custody['reservations_refunded']
            and len(custody['results']) == 3 and len(custody['typed_bookkeeping']) == 2
            and all(v['replicas_verified'] == 2 and v['real_restore_verified'] for v in custody['results']),
            'Failed runtime evidence not retained')
    variant = checked(reader, q['runtime_variant_conjunction'])
    require(variant['focused_python_tests'] == 54 and variant['production_runtime_execution_pending'] is True
            and variant['source_hash_waivers'] == [] and variant['historical_hashes_overwritten'] is False
            and all(reader.digest(n) == h for n, h in variant['sources'].items()), 'Runtime variant controls changed')
    retained = checked(reader, q['retained_current_check'])
    require(retained['qualified_contexts'] == len(retained['results']) == 45
            and retained['old_runtime_sources_and_ELF_still_exact_current'] is True
            and retained['new_native_source_waivers'] == [], 'Retained current phase changed')
    controls = checked(reader, q['controls'])
    require(controls['success'] and controls['tests'] >= 12 and controls['original_game_runs'] == 0
            and all(reader.digest(n) == h for n, h in controls['sources'].items()), 'New budget controls stale')
    child = checked(reader, q['actual_subprocess_controls'])
    require(child['tests'] == 12 and child['staged_scene_and_glyph_verified']
            and child['staged_compiler_manifest_verified'] and child['original_game_runs'] == child['new_native_executions'] == 0
            and all(reader.digest(n) == h for n,h in child['sources'].items()), 'Actual preparer subprocess controls changed')
    require(all(reader.digest(n) == h for n, h in q['reviewer_sources'].items()), 'Runtime continuation reviewer changed')
    validate_rows(q['old_requests'], q['old_requests'], q['new_case_reservations'])
    return q


def bound(reader, plan):
    review(reader, plan)
    require(plan['success'] and plan['previous_WP10_original_games'] == 8
            and plan['maximum_additional_original_games'] == plan['minimum_final_additional_reservations'] == 247
            and plan['maximum_combined_original_games'] == plan['maximum_combined_WP10_and_new_original_games'] == 255,
            'Changed247/255 ceiling')
    return 247


def predecessors(reader, rows, plan):
    q = review(reader, plan)
    return validate_rows(rows, q['old_requests'], q['new_case_reservations'])
