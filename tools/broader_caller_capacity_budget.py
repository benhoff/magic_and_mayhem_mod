"""One reviewed recorder-capacity continuation; preserve every old charge."""
from broader_player_acceptance import checked
from broader_runtime_continuation_budget import bound as prior_bound
from rendering_pilot import require

P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp29-world-caller-default800-01'
PILOT='bp30-world-caller-default800-01'

def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old};seen=set();total=0
    require(len(fixed)==104 and sum(v['reserved_games']for v in old)==144,'Changed104/144 preserved request domain')
    require(len(new)==80 and sum(new.values())==88 and set(fixed).isdisjoint(new),'Changed80/88 continuation domain')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'}
    full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request fields');seen.add(cid)
        if cid in fixed:
            require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:
            require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                    and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete capacity-continuation request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=232,'Historical request omitted or232ceiling exceeded')
    return total

def review(reader,plan):
    q=checked(reader,plan['caller_capacity_review'])
    require(q['kind']=='reviewed_caller_target_observation_continuation'and q['original_game_runs']==0
            and q['retained_current_cases']==10 and q['remaining_positive_cases']==80
            and q['remaining_positive_original_starts']==88 and q['new_additional_ceiling']==232
            and q['new_combined_ceiling']==240 and q['first_pilot']==PILOT
            and q['reservations_refunded']is False and q['automatic_retries']is False
            and q['user_numeric_approval_claimed']is False,'Unreviewed caller-capacity continuation')
    previous=checked(reader,q['parent_global_plan']);require(prior_bound(reader,previous)==231,'Prior231ceiling changed')
    old=q['old_requests'];require(len(old)==104 and sum(v['reserved_games']for v in old)==144,'Old104/144domain changed')
    negative=checked(reader,q['failed_case_custody']);failed=negative['request_row_preserved_exactly']
    require(failed['case_id']==FAILED and failed in old and failed['state']=='failed'
            and negative['actual_original_starts']==1 and negative['case_accepted']is False
            and negative['diagnosis']['rows']==262143 and negative['diagnosis']['original_exit_code']==99
            and negative['diagnosis']['normal_footer_present']is False,'Missing exact saturated failed run')
    controls=checked(reader,q['target_observation_controls'])
    require(controls['unit_tests']==21 and len(controls['results'])==7 and controls['original_game_runs']==0
            and controls['sources_stable']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Missing current target-observer controls')
    require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Capacity continuation reviewer changed')
    validate_rows(old,old,q['new_case_reservations'])
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==232
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==240,
            'Changed232/240 finite charge ceiling')
    return 232

def predecessors(reader,rows,plan):
    q=review(reader,plan)
    return validate_rows(rows,q['old_requests'],q['new_case_reservations'])

def phase_allowed(reader,phase,name):
    q=review(reader,checked(reader,phase['broader_plan']))
    require(phase['caller_capacity_review']==checked(reader,phase['broader_plan'])['caller_capacity_review']
            and phase['family']=='caller'and phase['supersedes_prospective_plan']['record']==P+'COPY-29-phase-05.json'
            and all(c['id']in q['new_case_reservations']for c in phase['broader_cases']),
            'Caller phase outside exact capacity continuation')
    return True
