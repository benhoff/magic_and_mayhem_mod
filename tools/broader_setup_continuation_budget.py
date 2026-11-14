"""One explicit charged zero-start setup replacement; all older reservations persist."""
from broader_copy_source_budget import review as original_review
from broader_player_acceptance import checked
from rendering_pilot import require

def review(reader,plan):
    q=checked(reader,plan['setup_continuation_review'])
    require(q['kind']=='B06_reviewed_zero_start_setup_continuation'and q['success']and q['original_game_runs']==0
            and q['Native_sources_unchanged']and q['old_source_current_credit']is False
            and q['reservations_refunded']is False and q['automatic_retries']is False,
            'Unreviewed zero-start setup continuation')
    base_plan=checked(reader,q['base_plan']);base=original_review(reader,base_plan)
    require(q['old_requests']==base['old_requests']and len(q['old_requests'])==71
            and sum(v['reserved_games']for v in q['old_requests'])==103,'Original71/103 history changed')
    failure=checked(reader,q['closed_setup_certificate']);closed=q['closed_setup_requests']
    require(len(closed)==1 and closed[0]==failure['failed_request']
            and failure['original_game_runs']==failure['new_native_executions']==0
            and failure['failed_request_must_not_be_rewritten_or_refunded']
            and failure['actual_start_proof']['before_player_or_Original_launch']
            and closed[0]['case_id']=='bp27-world-attachment-default640-01'
            and closed[0]['state']=='failed'and closed[0]['reserved_games']==1
            and closed[0]['acquisition_sha256']is None and closed[0]['validation']is None,
            'Closed setup reservation was lost, promoted or not proved zero-start')
    cases=q['positive_case_reservations'];expected={(k.replace('bp27-world-attachment-','bp28-world-attachment-',1)if k.startswith('bp27-world-attachment-')else k):v for k,v in base['new_case_reservations'].items()}
    require(cases==expected and sum(k.startswith('bp28-world-attachment-')for k in cases)==4,'Only the four attachment declarations may change identity')
    old={v['case_id']for v in q['old_requests']}|{closed[0]['case_id']}
    require(len(cases)==90 and sum(cases.values())==98 and set(cases).isdisjoint(old)
            and all(type(v)is int and v in(1,2)for v in cases.values()),'Positive90/98 scope changed')
    require(q['maximum_additional_charges']==202 and q['maximum_combined_with_WP10']==210
            and q['additional_charge_for_closed_setup']==1 and q['preserved_current_contexts']==18
            and q['preserved_current_actual_starts']==26,'Setup extension expanded beyond one retained charge')
    require(q['reviewer_sources']and all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Setup reviewer source changed')
    return q

def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==202
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==210,
            'Reviewed202/210 ceiling changed')
    return 202

def predecessors(reader,rows,plan):
    q=review(reader,plan);fixed={v['case_id']:v for v in q['old_requests']+q['closed_setup_requests']};new=q['positive_case_reservations'];seen=set();total=0
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate setup-continuation row');seen.add(cid)
        if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical or closed setup request rewritten')
        else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                     and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete continuation request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=202,'Setup reservation omitted or202charge ceiling exceeded')
    return total
