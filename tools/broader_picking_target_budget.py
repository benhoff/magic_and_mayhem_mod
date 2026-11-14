"""Finite continuation retaining the exact failed original-cell picking request."""
from broader_palette_schema_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp34-world-picking-default800-01'
PILOT='bp35-world-picking-default800-01'

def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old};seen=set();total=0
    require(len(fixed)==120 and sum(v['reserved_games']for v in old)==160,'Changed120/160 historical request domain')
    require(len(new)==69 and sum(new.values())==77 and set(fixed).isdisjoint(new),'Changed69/77 continuation domain')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'}
    full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id']
        require(set(row)in(minimal,full)and cid not in seen,'Unknown or duplicate request fields')
        seen.add(cid)
        if cid in fixed:
            require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:
            require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                    and row['acquisition_sha256']and cid in row['report'],
                    'Stop after unknown, failed or incomplete picking-target request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=237,'Historical request omitted or237charge ceiling exceeded')
    return total

def review(reader,plan):
    q=checked(reader,plan['picking_target_review'])
    require(q['kind']=='reviewed_picking_animation_target_continuation'
            and q['original_game_runs']==0 and q['retained_current_cases']==21
            and q['remaining_positive_cases']==69 and q['remaining_positive_original_starts']==77
            and q['new_additional_ceiling']==237 and q['new_combined_ceiling']==245
            and q['first_pilot']==PILOT and q['reservations_refunded']is False
            and q['automatic_retries']is False and q['user_numeric_approval_claimed']is False,
            'Changed finite picking-target continuation')
    require(parent_bound(reader,checked(reader,q['parent_global_plan']))==236,'Prior finite ceiling changed')
    failed=checked(reader,q['failed_picking_custody']);row=failed['request_row_preserved_exactly']
    require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'
            and row['reserved_games']==1 and failed['actual_original_starts']==1
            and failed['case_accepted']is False and failed['normal_quit_credit']is False,
            'Failed picking request changed or received acceptance credit')
    placement=checked(reader,q['animation_placement_review'])
    require(placement['kind']=='reviewed_finite_picking_animation_placement'
            and placement['new_target_ground_hit_proved']is False and placement['case_accepted']is False,
            'Offline target derivation promoted to live picking evidence')
    controls=checked(reader,q['adapter_controls'])
    require(controls['tests']==29 and controls['actual_recursive_controller_binding_verified']
            and controls['exact_ready_and_quiet_source_functions_preserved']
            and controls['original_game_runs']==controls['new_native_player_executions']==controls['new_request_reservations']==0
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),
            'Missing current picking adapter controls')
    require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Picking reviewer source changed')
    validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==237
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==245,
            'Changed237/245 finite charge ceiling')
    return 237

def predecessors(reader,rows,plan):
    q=review(reader,plan)
    return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
