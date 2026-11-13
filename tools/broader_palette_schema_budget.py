"""Finite continuation preserving the Native-UI-only palette schema refusal."""
from broader_player_acceptance import checked
from broader_font_reference_budget import bound as parent_bound
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261150/'
FAILED='bp33-world-palette-default640-01'
PILOT='bp34-world-palette-default640-01'
def validate_rows(rows,old,new):
 fixed={v['case_id']:v for v in old};seen=set();total=0
 require(len(fixed)==115 and sum(v['reserved_games']for v in old)==155,'Changed115/155 preserved request domain')
 require(len(new)==73 and sum(new.values())==81 and set(fixed).isdisjoint(new),'Changed73/81 continuation domain')
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request fields');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete palette-schema-continuation request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=236,'Historical request omitted or236ceiling exceeded');return total

def review(reader,plan):
 q=checked(reader,plan['palette_schema_review'])
 require(q['kind']=='reviewed_palette_counter_schema_continuation'and q['original_game_runs']==0 and q['retained_current_cases']==17 and q['remaining_positive_cases']==73 and q['remaining_positive_original_starts']==81 and q['new_additional_ceiling']==236 and q['new_combined_ceiling']==244 and q['first_pilot']==PILOT and q['reservations_refunded']is False and q['automatic_retries']is False and q['user_numeric_approval_claimed']is False,'Unreviewed font-reference continuation')
 parent=checked(reader,q['parent_global_plan']);require(parent_bound(reader,parent)==235,'Prior finite ceiling changed')
 failed=checked(reader,q['failed_schema_custody']);row=failed['request_row_preserved_exactly'];require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'and row['reserved_games']==1 and failed['actual_original_starts']==0 and failed['case_accepted']is False,'Font reference negative changed')
 fonts=checked(reader,q['qualified_font_conjunction']);require(len(fonts['contexts'])==4 and fonts['pixels']==838645500 and fonts['mismatching_pixels']==0,'Exact completed font conjunction missing')
 controls=checked(reader,q['schema_controls']);require(controls['tests']==9 and controls['remaining_positive_contexts']==73 and controls['distinct_declared_counter_paths']==10 and controls['original_game_runs']==controls['new_native_player_executions']==controls['new_request_reservations']==0 and controls['sources_stable']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current counter schema audit missing')
 require(failed['actual_native_UI_launches']==1,'Palette NativeUI setup execution erased')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Font reference reviewer changed');validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations']);return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==236 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==244,'Changed236/244 finite charge ceiling');return 236

def predecessors(reader,rows,plan):
 q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
