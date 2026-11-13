"""One reviewed recorder-loader continuation; preserve every old charge."""
from broader_player_acceptance import checked
from broader_runtime_continuation_budget import bound as prior_bound
from rendering_pilot import require

P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp29-world-caller-default800-01'
PILOT='bp31-world-caller-default800-01'

def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old};seen=set();total=0
    require(len(fixed)==105 and sum(v['reserved_games']for v in old)==145,'Changed105/145 preserved request domain')
    require(len(new)==80 and sum(new.values())==88 and set(fixed).isdisjoint(new),'Changed80/88 continuation domain')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'}
    full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request fields');seen.add(cid)
        if cid in fixed:
            require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:
            require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                    and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete loader-continuation request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=233,'Historical request omitted or233ceiling exceeded')
    return total

def review(reader,plan):
    q=checked(reader,plan['caller_loader_review'])
    require(q['kind']=='reviewed_caller_loader_observation_continuation'and q['original_game_runs']==0
            and q['retained_current_cases']==10 and q['remaining_positive_cases']==80
            and q['remaining_positive_original_starts']==88 and q['new_additional_ceiling']==233
            and q['new_combined_ceiling']==241 and q['first_pilot']==PILOT
            and q['reservations_refunded']is False and q['automatic_retries']is False
            and q['user_numeric_approval_claimed']is False,'Unreviewed caller-loader continuation')
    previous=checked(reader,q['parent_global_plan']);require(previous['maximum_additional_original_games']==232 and prior_bound(reader,reader.json(P+'plan-29.json'))==231,'Prior finite ceilings changed')
    old=q['old_requests'];require(len(old)==105 and sum(v['reserved_games']for v in old)==145,'Old105/145domain changed')
    negative=checked(reader,q['failed_case_custody']);failed=negative['request_row_preserved_exactly']
    require(failed['case_id']==FAILED and failed in old and failed['state']=='failed'
            and negative['actual_original_starts']==1 and negative['case_accepted']is False
            and negative['diagnosis']['rows']==262143 and negative['diagnosis']['original_exit_code']==99
            and negative['diagnosis']['normal_footer_present']is False,'Missing exact saturated failed run')
    loader=checked(reader,q['failed_loader_custody']);zero=loader['request_row_preserved_exactly']
    require(zero in old and zero['case_id']=='bp30-world-caller-default800-01'and zero['state']=='failed'and zero['acquisition_sha256']is None and loader['actual_original_starts']==0 and loader['acquisition_report_absent'],'Zero-game loader refusal changed')
    recursive=checked(reader,q['recursive_loader_controls'])
    require(recursive['tests']==31 and recursive['original_game_runs']==0 and recursive['active_menu_module_instances']==2 and recursive['active_camera_module_instances']==1 and len(recursive['actual_loader_providers'])==2 and all(reader.digest(n)==h for n,h in recursive['sources'].items()),'Real recursive loader controls missing')
    controls=checked(reader,q['target_observation_controls'])
    require(controls['unit_tests']==21 and len(controls['results'])==7 and controls['original_game_runs']==0
            and controls['sources_stable']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Missing current target-observer controls')
    require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Capacity continuation reviewer changed')
    validate_rows(old,old,q['new_case_reservations'])
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==233
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==241,
            'Changed233/241 finite charge ceiling')
    return 233

def predecessors(reader,rows,plan):
    q=review(reader,plan)
    return validate_rows(rows,q['old_requests'],q['new_case_reservations'])

def phase_allowed(reader,phase,name):
    q=review(reader,checked(reader,phase['broader_plan']))
    require(phase['caller_loader_review']==checked(reader,phase['broader_plan'])['caller_loader_review']
            and phase['family']=='caller'and phase['supersedes_prospective_plan']['record'].startswith(P+'capacity-30-phase-')
            and all(c['id']in q['new_case_reservations']for c in phase['broader_cases']),
            'Caller phase outside exact loader continuation')
    return True
