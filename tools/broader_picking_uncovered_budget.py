"""Finite display-witness continuation; retain every historical failed request."""
from broader_picking_rebuild_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp38-world-picking-software800-01'
PILOT='bp39-world-picking-software800-01'

def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(fixed)==125 and sum(v['reserved_games']for v in old)==165,'Changed125/165 historical domain')
 require(len(new)==68 and sum(new.values())==76 and set(fixed).isdisjoint(new),'Changed68/76 continuation')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown or duplicate request fields');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after failed or incomplete witness request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=241,'Historical row omitted or241charge ceiling exceeded')
 return total

def review(reader,plan):
 q=checked(reader,plan['picking_uncovered_review'])
 require(q['kind']=='reviewed_picking_uncovered_source_continuation'and q['original_game_runs']==0
         and q['retained_current_cases']==22 and q['remaining_positive_cases']==68
         and q['remaining_positive_original_starts']==76 and q['new_additional_ceiling']==241
         and q['new_combined_ceiling']==249 and q['first_pilot']==PILOT
         and q['reservations_refunded']is False and q['automatic_retries']is False
         and q['user_numeric_approval_claimed']is False,'Changed finite witness continuation')
 require(q['software_inner_cap_review']==dict(requests_before=6,requests_after=7,original_games_before=6,original_games_after=7,only_case=PILOT)
         and q['native_or_capture_limits_changed']is False and q['response_deadlines_changed']is False
         and q['source_waivers']==[],'Unreviewed inner game cap or source/deadline change')
 require(parent_bound(reader,checked(reader,q['parent_global_plan']))==240,'Prior finite ceiling changed')
 negative=checked(reader,q['failed_uncovered_custody']);row=negative['request_row_preserved_exactly']
 require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'
         and row['reserved_games']==1 and negative['actual_original_starts']==1
         and negative['case_accepted']is False and negative['normal_quit_credit']is False
         and negative['completed_windows']==44 and negative['completed_rebuilds']==2,'Failed live-latest witness case changed or promoted')
 controls=checked(reader,q['uncovered_controls'])
 require(controls['tests']==134 and controls['focused_target_tests']==16 and controls['focused_binding_tests']==8
         and controls['decoded_metadata_tests']==9 and controls['old_point_covering_source_emissions']
         and controls['source_only_uncovered_point']==[402,277]
         and controls['Original_hit_feedback_passed_to_selector']is False and controls['Original_hit_order_assumed']is False
         and controls['controller_deadlines_changed']is False and controls['quiet_source_seconds']==controls['picking_witness_seconds']==8
         and controls['rebuild_seconds']==30 and controls['original_game_runs']==controls['new_native_player_executions']==0
         and controls['case_accepted']is False and all(reader.digest(n)==h for n,h in controls['sources'].items()),
         'Current source-only body exclusion or strict metadata controls missing')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Witness reviewer source changed')
 validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
 return q

def bound(reader,plan):
 review(reader,plan)
 require(plan['success']and plan['previous_WP10_original_games']==8
         and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==241
         and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==249,'Changed241/249 ceiling')
 return 241

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
