"""One exact data-binding continuation; preserve all prior charges and stop on failure."""
from rendering_pilot import require


def review(reader, plan):
    ref = plan['delegated_plan_review']
    require(reader.digest(ref['record']) == ref['sha256'], 'Publication review identity changed')
    value = reader.json(ref['record'])
    require(value['kind'] == 'broader_delegated_plan_continuation_review'
            and value['success'] is True and value['original_game_runs'] == 0
            and value['old_reserved_starts'] == 80 and value['retained_current_cases'] == 18
            and value['remaining_positive_cases'] == value['remaining_positive_original_starts'] == 72
            and value['new_additional_ceiling'] == 152 and value['new_combined_ceiling'] == 160
            and value['reservations_refunded'] is False and value['agent_set_limit'] is True
            and value['user_numeric_approval_claimed'] is False
            and value['automatic_repair_starts'] == 0 and value['manually_reviewed_replacement_starts'] == 5
            and value['changed_compiler_files'] == [] and value['changed_runtime_files'] == []
            and value['game_response_deadlines_changed'] is False,
            'Unbounded or source-waived publication continuation')
    require(all(reader.digest(n) == h for n, h in value['reviewer_sources'].items()),
            'Publication reviewer changed')
    build_ref = value['current_build']
    require(reader.digest(build_ref['record']) == build_ref['sha256'], 'Current build identity changed')
    build = reader.json(build_ref['record'])
    require(build['success'] and build['sources_stable']
            and all(reader.digest(n) == h for n, h in (build['compiler_sources'] | build['runtime_sources']).items()),
            'Native/compiler/runtime input changed')
    ref = value['corrected_build_binding']
    require(reader.digest(ref['record']) == ref['sha256'], 'Corrected build binding changed')
    declaration = reader.json(ref['record'])
    compiler = declaration['player_build']['compiler_sources']
    configuration = declaration['configuration_sources']
    require(declaration['success'] and declaration['new_native_builds'] == 0
            and declaration['full_build_sources'] == build['compiler_sources']
            and set(configuration) == {'apps/qt-shell/CMakeLists.txt'}
            and not set(compiler) & set(configuration)
            and compiler | configuration == build['compiler_sources'],
            'Compiler/configuration partition cannot drop a build dependency')
    for ref in value['current_controls']:
        require(reader.digest(ref['record']) == ref['sha256'], 'Publication control identity changed')
        controls = reader.json(ref['record'])
        require(controls['success'] and controls.get('sources_stable', True)
                and all(reader.digest(n) == h for n, h in controls.get('sources', {}).items()),
                'Current publication controls changed')
    old = value['old_requests']
    require(len(old) == 48 and len({r['case_id'] for r in old}) == 48
            and sum(r['reserved_games'] for r in old) == 80, 'Historical reservations changed')
    ids = value['new_case_ids']
    require(len(ids) == len(set(ids)) == 72 and not set(ids) & {r['case_id'] for r in old},
            'Duplicate or reused publication case identity')
    return value


def bound(reader, plan):
    review(reader, plan)
    require(plan['success'] and plan['previous_WP10_original_games'] == 8
            and plan['maximum_additional_original_games'] == plan['minimum_final_additional_reservations'] == 152
            and plan['maximum_combined_original_games'] == plan['maximum_combined_WP10_and_new_original_games'] == 160
            and plan['maximum_manually_reviewed_additional_repairs'] == 5,
            'Changed finite publication bound')
    return 152


def predecessors(reader, rows, plan):
    value = review(reader, plan)
    old = {r['case_id']: r for r in value['old_requests']}
    allowed = set(value['new_case_ids'])
    seen = set()
    total = 0
    minimal = {'case_id', 'report', 'reserved_games', 'state', 'acquisition_sha256'}
    full = minimal | {'plan', 'started_ns', 'validation'}
    for row in rows:
        cid = row['case_id']
        require(set(row) in (minimal, full) and cid not in seen, 'Unknown or duplicate request row')
        seen.add(cid)
        if cid in old:
            require(all(old[cid][key] == item for key, item in row.items()), 'Historical request rewritten')
        else:
            require(cid in allowed and row['state'] == 'admitted' and row['reserved_games'] == 1
                    and row['acquisition_sha256'] and cid in row['report'],
                    'Stop after every unknown, failed or incomplete publication request')
        total += row['reserved_games']
    require(set(old) <= seen, 'Historical reservation disappeared')
    return total
