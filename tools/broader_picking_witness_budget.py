"""Finite display-witness continuation; retain every historical failed request."""
from broader_picking_pose_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp36-world-picking-default800-01'
PILOT='bp37-world-picking-default800-01'

def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(fixed)==122 and sum(v['reserved_games']for v in old)==162,'Changed122/162 historical domain')
 require(len(new)==69 and sum(new.values())==77 and set(fixed).isdisjoint(new),'Changed69/77 continuation')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown or duplicate request fields');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after failed or incomplete witness request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=239,'Historical row omitted or239charge ceiling exceeded')
 return total

def review(reader,plan):
 q=checked(reader,plan['picking_witness_review'])
 require(q['kind']=='reviewed_picking_presented_witness_continuation'and q['original_game_runs']==0
         and q['retained_current_cases']==21 and q['remaining_positive_cases']==69
         and q['remaining_positive_original_starts']==77 and q['new_additional_ceiling']==239
         and q['new_combined_ceiling']==247 and q['first_pilot']==PILOT
         and q['reservations_refunded']is False and q['automatic_retries']is False
         and q['user_numeric_approval_claimed']is False,'Changed finite witness continuation')
 require(parent_bound(reader,checked(reader,q['parent_global_plan']))==238,'Prior finite ceiling changed')
 negative=checked(reader,q['failed_witness_custody']);row=negative['request_row_preserved_exactly']
 require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'
         and row['reserved_games']==1 and negative['actual_original_starts']==1
         and negative['case_accepted']is False and negative['normal_quit_credit']is False
         and negative['completed_windows']==38,'Failed live-latest witness case changed or promoted')
 controls=checked(reader,q['witness_controls'])
 require(controls['tests']==76 and controls['focused_witness_tests']==20
         and controls['original_predicate_eligible_generations']==15 and len(controls['buffered_replayed_facts'])==15
         and controls['original_game_runs']==controls['new_native_player_executions']==0
         and controls['case_accepted']is False and controls['original_ready_predicate_unchanged']
         and controls['physical_actions_changed']is False and controls['event_deadline_us']==8000000
         and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current actual-swap controls missing')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Witness reviewer source changed')
 validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
 return q

def bound(reader,plan):
 review(reader,plan)
 require(plan['success']and plan['previous_WP10_original_games']==8
         and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==239
         and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==247,'Changed239/247 ceiling')
 return 239

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
