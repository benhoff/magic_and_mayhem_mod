"""Bound the reviewed zero-execution collection-map repair without refunding charges."""
from broader_picking_pan_budget import bound as parent_bound
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp40-loss-default640-01'
PILOT='bp41-loss-default640-01'
def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(old)==len(fixed)==128 and sum(v['reserved_games']for v in old)==169,'Changed128/169 historical domain')
 require(len(new)==67 and sum(new.values())==75 and set(fixed).isdisjoint(new),'Changed67/75 continuation')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after failed/incomplete new request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=244,'Historical row omitted or244 ceiling exceeded')
 return total

def review(reader,plan):
 q=checked(reader,plan['collection_map_review']);require(q['kind']=='reviewed_zero_execution_collection_map_continuation'and q['success']and q['original_game_runs']==q['new_native_executions']==0,'Wrong collection review')
 require(q['retained_current_cases']==23 and q['remaining_positive_cases']==67 and q['remaining_positive_original_starts']==75 and q['new_additional_ceiling']==244 and q['new_combined_ceiling']==252 and q['first_pilot']==PILOT,'Changed finite collection continuation')
 require(q['reservations_refunded']is False and q['automatic_retries']is False and q['source_waivers']==[]and q['Native_runtime_controller_sources_changed']is False,'Unreviewed source/charge waiver')
 require(parent_bound(reader,checked(reader,q['parent_global_plan']))==242,'Prior ceiling changed')
 failure=checked(reader,q['setup_diagnosis']);row=failure['failed_request'];require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==2 and row['acquisition_sha256']is None and failure['original_game_runs']==failure['new_native_executions']==0 and failure['reservations_refunded']is False,'Failed setup row changed/promoted')
 controls=checked(reader,q['controls']);require(controls['success']and controls['original_game_runs']==controls['new_native_executions']==0 and controls['tests']>=10 and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Missing current collection controls')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Collection reviewer source changed')
 validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations']);return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==244 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==252,'Changed244/252 ceiling');return 244

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
