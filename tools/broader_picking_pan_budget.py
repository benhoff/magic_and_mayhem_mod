"""Finite display-witness continuation; retain every historical failed request."""
from broader_picking_uncovered_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp39-world-picking-software800-01'
PILOT='bp40-world-picking-software800-01'

def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(fixed)==126 and sum(v['reserved_games']for v in old)==166,'Changed126/166 historical domain')
 require(len(new)==68 and sum(new.values())==76 and set(fixed).isdisjoint(new),'Changed68/76 continuation')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown or duplicate request fields');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after failed or incomplete witness request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=242,'Historical row omitted or242charge ceiling exceeded')
 return total

def review(reader,plan):
 q=checked(reader,plan['picking_pan_review'])
 require(q['kind']=='reviewed_picking_pan_source_continuation'and q['original_game_runs']==0
         and q['retained_current_cases']==22 and q['remaining_positive_cases']==68
         and q['remaining_positive_original_starts']==76 and q['new_additional_ceiling']==242
         and q['new_combined_ceiling']==250 and q['first_pilot']==PILOT
         and q['reservations_refunded']is False and q['automatic_retries']is False
         and q['user_numeric_approval_claimed']is False,'Changed finite witness continuation')
 require(q['software_inner_cap_review']==dict(requests_before=7,requests_after=8,original_games_before=7,original_games_after=8,only_case=PILOT)
         and q['native_or_capture_limits_changed']is False and q['response_deadlines_changed']is False
         and q['physical_pan_hold_release_policy_changed']is True and q['source_waivers']==[],'Unreviewed inner game cap or source/deadline change')
 require(parent_bound(reader,checked(reader,q['parent_global_plan']))==241,'Prior finite ceiling changed')
 negative=checked(reader,q['failed_pan_custody']);row=negative['request_row_preserved_exactly']
 require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='acquired'
         and row['reserved_games']==1 and negative['actual_original_starts']==1
         and negative['case_accepted']is False and negative['normal_quit_credit']is False
         and negative['completed_windows']==48 and negative['completed_rebuilds']==2 and negative['actual_acquisition_success']is True
         and negative['comparison_success']is False and negative['actual_normal_Quit_preserved']is True
         and negative['mismatching_pixels']==0,'Failed live-latest witness case changed or promoted')
 controls=checked(reader,q['pan_controls'])
 require(controls['tests']==163 and controls['fresh_tests']==29 and controls['retained_same_source_tests']==134
         and controls['pan_ack_tests']==18 and controls['pan_assessment_tests']==8 and controls['pan_binding_tests']==3
         and controls['failed_pan_not_waived']and controls['original_captured_source_pan_predicate_unchanged']
         and controls['physical_pan_hold_release_policy_changed']is True and controls['minimum_hold_seconds']==.22
         and controls['old_hold_plus_settle_seconds']==controls['new_hold_plus_settle_seconds']==1.02
         and controls['settlement_uses_remaining_existing_budget']and controls['original_five_World_thirty_second_progress_wait_unchanged']
         and controls['source_quiet_and_witness_deadlines_seconds']==8 and controls['rebuild_deadline_seconds']==30
         and controls['Native_runtime_compiler_player_sources_changed']is False
         and controls['original_game_runs']==controls['new_native_player_executions']==0 and controls['case_accepted']is False
         and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current bounded pan/source controls missing')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Witness reviewer source changed')
 validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
 return q

def bound(reader,plan):
 review(reader,plan)
 require(plan['success']and plan['previous_WP10_original_games']==8
         and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==242
         and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==250,'Changed242/250 ceiling')
 return 242

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
