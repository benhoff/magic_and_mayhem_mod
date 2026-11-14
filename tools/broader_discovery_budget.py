"""Preserve every reservation while binding the current strict declaration."""
from broader_strict_binding_budget import bound as parent_bound,predecessors as parent_predecessors
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp40-boundary-software800-01'
PILOT='bp43-boundary-software800-01'
def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(old)==len(fixed)==142 and sum(v['reserved_games']for v in old)==191,'Changed142/191 historical domain')
 require(len(new)==55 and sum(new.values())==55 and set(fixed).isdisjoint(new)and all(v==1 for v in new.values()),'Changed55/55 continuation')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after failed/incomplete new request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=246,'Historical row omitted or246 ceiling exceeded');return total

def review(reader,plan):
 q=checked(reader,plan['discovery_binding_review']);require(q['kind']=='reviewed_current_discovery_namespace_continuation'and q['success']and q['original_game_runs']==q['new_native_executions']==0,'Wrong strict declaration review')
 require(q['retained_current_cases']==35 and q['remaining_positive_cases']==q['remaining_positive_original_starts']==55 and q['new_additional_ceiling']==246 and q['new_combined_ceiling']==254 and q['first_pilot']==PILOT,'Changed finite strict continuation')
 require(q['reservations_refunded']is False and q['automatic_retries']is False and q['source_waivers']==[]and q['Native_runtime_controller_sources_changed']is False,'Unreviewed source/charge waiver')
 parent=checked(reader,q['parent_global_plan']);require(parent_bound(reader,parent)==245,'Prior ceiling changed')
 failure=checked(reader,q['setup_diagnosis']);row=failure['failed_request'];require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==1 and row['acquisition_sha256']==failure['acquisition']['sha256'] and failure['original_game_runs']==failure['new_native_executions']==0,'Failed setup row changed/promoted')
 require(parent_predecessors(reader,[v for v in q['old_requests']if v['case_id']!=FAILED],parent)==190,'Parent history changed')
 controls=checked(reader,q['controls']);require(controls['success']and controls['original_game_runs']==controls['new_native_executions']==0 and controls['tests']>=12 and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Missing current strict binding controls')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Strict binding reviewer source changed')
 validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations']);return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==246 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==254,'Changed246/254 ceiling');return 246

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
