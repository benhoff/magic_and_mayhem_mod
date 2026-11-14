"""Finite continuation after an exact zero-game font prerequisite refusal."""
from broader_player_acceptance import checked
from broader_runtime_continuation_budget import bound as original_bound
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp31-world-font-default800-01'
PILOT='bp32-world-font-default800-01'

def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old};seen=set();total=0
    require(len(fixed)==109 and sum(v['reserved_games']for v in old)==149,'Changed109/149 preserved request domain')
    require(len(new)==77 and sum(new.values())==85 and set(fixed).isdisjoint(new),'Changed77/85 continuation domain')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request fields');seen.add(cid)
        if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete prerequisite-continuation request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=234,'Historical request omitted or234ceiling exceeded')
    return total

def review(reader,plan):
    q=checked(reader,plan['font_prerequisite_review'])
    require(q['kind']=='reviewed_font_prerequisite_continuation'and q['original_game_runs']==0
            and q['retained_current_cases']==13 and q['remaining_positive_cases']==77
            and q['remaining_positive_original_starts']==85 and q['new_additional_ceiling']==234
            and q['new_combined_ceiling']==242 and q['first_pilot']==PILOT
            and q['reservations_refunded']is False and q['automatic_retries']is False
            and q['user_numeric_approval_claimed']is False,'Unreviewed font prerequisite continuation')
    prior=checked(reader,q['parent_global_plan']);require(prior['maximum_additional_original_games']==233 and original_bound(reader,reader.json(P+'plan-29.json'))==231,'Prior finite ceilings changed')
    failed=checked(reader,q['failed_font_custody']);row=failed['request_row_preserved_exactly']
    require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==1
            and failed['actual_original_starts']==0 and failed['case_accepted']is False,'Font prerequisite negative changed')
    refresh=checked(reader,q['control_refresh']);require(refresh['original_game_runs']==0
            and refresh['player_before']==refresh['player_after']=='8d606f2ce02514ab21b82f2e0ff5ec515320f01e93386971bcb1fb6c87d080ca'
            and refresh['scene_before']==refresh['scene_after']=='9b6e68c000459af0dbb454ad2306cbbc20b821c01f4e6ecd9d83d08c3ce2b7cb',
            'Focused refresh changed the frozen Native binaries')
    for ref in refresh['current_controls'].values():
        controls=checked(reader,ref);require(controls['sources_stable']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current prerequisite control drift')
    require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Prerequisite continuation reviewer changed')
    validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==234
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==242,
            'Changed234/242 finite charge ceiling')
    return 234

def predecessors(reader,rows,plan):
    q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
