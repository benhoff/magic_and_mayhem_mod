"""Finite continuation preserving the failed hash-only ANI inverse request."""
from broader_picking_target_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp35-world-picking-default800-01'
PILOT='bp36-world-picking-default800-01'

def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old};seen=set();total=0
    require(len(fixed)==121 and sum(v['reserved_games']for v in old)==161,'Changed121/161 historical domain')
    require(len(new)==69 and sum(new.values())==77 and set(fixed).isdisjoint(new),'Changed69/77 continuation')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown or duplicate request fields');seen.add(cid)
        if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                     and row['acquisition_sha256']and cid in row['report'],'Stop after failed or incomplete observed-ANI request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=238,'Historical row omitted or238charge ceiling exceeded')
    return total

def review(reader,plan):
    q=checked(reader,plan['picking_pose_review'])
    require(q['kind']=='reviewed_picking_exact_ANI_observation_continuation'
            and q['original_game_runs']==0 and q['retained_current_cases']==21
            and q['remaining_positive_cases']==69 and q['remaining_positive_original_starts']==77
            and q['new_additional_ceiling']==238 and q['new_combined_ceiling']==246
            and q['first_pilot']==PILOT and q['reservations_refunded']is False
            and q['automatic_retries']is False and q['user_numeric_approval_claimed']is False,
            'Changed finite exact-ANI continuation')
    require(parent_bound(reader,checked(reader,q['parent_global_plan']))==237,'Prior finite ceiling changed')
    negative=checked(reader,q['failed_index_custody']);row=negative['request_row_preserved_exactly']
    require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==1
            and negative['actual_original_starts']==1 and negative['case_accepted']is False
            and negative['normal_quit_credit']is False and negative['completed_windows']==20,
            'Failed inverse-index game changed or promoted')
    diagnosis=checked(reader,q['retained_source_ambiguity'])
    require(diagnosis['exact_prethrow_frame_recorded']is False and len(diagnosis['ambiguous_sources'])==11
            and diagnosis['case_accepted']is False,'Retained ambiguity finding changed')
    controls=checked(reader,q['pose_controls'])
    require(controls['tests']==56 and controls['exact_codec_equivalence_captures']==4
            and controls['exact_source_ANI_target_probes']==16 and controls['current_picking_case_proved']is False
            and controls['original_game_runs']==controls['new_native_player_executions']==0
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current exact-ANI source controls missing')
    require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Observed-ANI reviewer source changed')
    validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==238
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==246,
            'Changed238/246 charge ceiling')
    return 238

def predecessors(reader,rows,plan):
    q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
