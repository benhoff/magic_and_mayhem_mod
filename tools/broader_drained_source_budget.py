"""One reviewed drain-refused replacement; retain every earlier reservation."""
from broader_worker_source_budget import bound as parent_bound
from rendering_pilot import require

def review(reader,plan):
 ref=plan['drain_continuation_review'];require(reader.digest(ref['record'])==ref['sha256'],'Drain review identity changed');q=reader.json(ref['record'])
 require(q['success']and q['kind']=='broader_drained_admission_continuation_review'and q['original_game_runs']==0 and q['old_additional_ceiling']==128 and q['new_additional_ceiling']==130 and q['old_combined_ceiling']==136 and q['new_combined_ceiling']==138 and q['retained_predecessor_reservations']==42 and q['remaining_positive_cases']==85 and q['remaining_positive_original_starts']==88 and q['reservations_refunded']is False and q['agent_set_limit']is True and q['user_numeric_approval_claimed']is False and q['automatic_repair_starts']==0 and q['native_player_changed']is False and q['response_deadlines_changed']is False,'Unreviewed drain continuation')
 parent=q['parent_global_plan'];require(reader.digest(parent['record'])==parent['sha256'],'Parent plan changed');require(parent_bound(reader,reader.json(parent['record']))==128,'Parent budget changed')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Continuation implementation changed')
 for ref in q['controls']:
  require(reader.digest(ref['record'])==ref['sha256'],'Continuation controls changed');v=reader.json(ref['record']);require(v['success']and v.get('sources_stable',True)and all(reader.digest(n)==h for n,h in v.get('sources',{}).items()),'Stale continuation controls')
 require(len(q['predecessor_requests'])==21 and sum(v['reserved_games']for v in q['predecessor_requests'])==42,'Previous charges changed')
 negative=q['negative_custody'];require(reader.digest(negative['record'])==negative['sha256'],'Negative custody changed');n=reader.json(negative['record']);require(n['success']and n['case_accepted']is False and n['reservations_refunded']is False and n['request_row_preserved_exactly']in q['predecessor_requests']and n['request_row_preserved_exactly']['state']=='failed'and n['request_row_preserved_exactly']['reserved_games']==2,'Refused transition must stay charged and negative')
 return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['maximum_additional_original_games']==130 and plan['maximum_combined_original_games']==138 and plan['maximum_combined_WP10_and_new_original_games']==138 and plan['minimum_final_additional_reservations']==130 and plan['maximum_manually_reviewed_additional_repairs']==0 and plan['previous_WP10_original_games']==8,'Unbounded continuation')
 return 130

def predecessors(reader,rows,plan):
 q=review(reader,plan);old={v['case_id']:v for v in q['predecessor_requests']};wanted=set(q['remaining_case_ids']);seen=set();total=0
 require(len(wanted)==85 and len(q['remaining_case_ids'])==85,'Changed prospective case denominator')
 allowed={'case_id','report','reserved_games','state','acquisition_sha256'};full=allowed|{'plan','started_ns','validation'}
 for row in rows:
  require(set(row)in(allowed,full),'Unknown request row shape')
  if row['case_id']in old:
   require(row['case_id']not in seen and all(old[row['case_id']][k]==v for k,v in row.items()),'Previous request rewritten');seen.add(row['case_id'])
  else:
   require(row['case_id']in wanted and row['state']=='admitted'and row['reserved_games']in(1,2)and row['acquisition_sha256']and row['case_id']in row['report'],'Stop after unknown or incomplete continuation request')
  require(row['case_id']not in seen or row['case_id']in old,'Duplicate continuation request');seen.add(row['case_id']);total+=row['reserved_games']
 require(set(old)<=seen,'An earlier reservation disappeared')
 return total
