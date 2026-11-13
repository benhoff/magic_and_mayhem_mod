"""Finite continuation preserving the exact missing font-pilot argument refusal."""
from broader_player_acceptance import checked
from broader_prerequisite_budget import bound as parent_bound
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp32-world-font-default640-01'
PILOT='bp33-world-font-default640-01'
def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(fixed)==111 and sum(v['reserved_games']for v in old)==151,'Changed111/151 preserved request domain')
 require(len(new)==76 and sum(new.values())==84 and set(fixed).isdisjoint(new),'Changed76/84 continuation domain')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request fields');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete font-reference-continuation request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=235,'Historical request omitted or235ceiling exceeded');return total

def review(reader,plan):
 q=checked(reader,plan['font_reference_review'])
 require(q['kind']=='reviewed_font_pilot_reference_continuation'and q['original_game_runs']==0 and q['retained_current_cases']==14 and q['remaining_positive_cases']==76 and q['remaining_positive_original_starts']==84 and q['new_additional_ceiling']==235 and q['new_combined_ceiling']==243 and q['first_pilot']==PILOT and q['reservations_refunded']is False and q['automatic_retries']is False and q['user_numeric_approval_claimed']is False,'Unreviewed font-reference continuation')
 parent=checked(reader,q['parent_global_plan']);require(parent_bound(reader,parent)==234,'Prior finite ceiling changed')
 failed=checked(reader,q['failed_reference_custody']);row=failed['request_row_preserved_exactly'];require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==1 and failed['actual_original_starts']==0 and failed['case_accepted']is False,'Font reference negative changed')
 pilot=checked(reader,q['qualified_font_pilot']);require(pilot['qualified_current_contexts']==14 and pilot['conservative_charges_after']==150 and pilot['original_game_runs_added']==0,'Exact retained font qualification missing')
 binding=checked(reader,pilot['positive_pilot_binding']);comparison=checked(reader,binding['comparison']);require(binding['payload_bytes_unchanged']and comparison['font_lifetimes_verified']and comparison['independent_output_verified']and comparison['native_menu_fonts']['verified']and comparison['sources_stable']and all(reader.digest(n)==h for n,h in comparison['sources'].items()),'Font reference not current and positive')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Font reference reviewer changed');validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations']);return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==235 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==243,'Changed235/243 finite charge ceiling');return 235

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
