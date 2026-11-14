"""A wholly fresh B05 cohort; every earlier request remains charged and immutable."""
from rendering_pilot import require


def review(reader,plan):
 ref=plan['viewport_source_review'];require(reader.digest(ref['record'])==ref['sha256'],'Viewport source review changed');q=reader.json(ref['record'])
 require(q['kind']=='broader_viewport_source_continuation_review'and q['success']is True and q['original_game_runs']==0 and q['old_source_current_credit']is False and q['old_reserved_starts']==49 and q['new_additional_ceiling']==147 and q['new_combined_ceiling']==155 and q['fresh_positive_cases']==90 and q['fresh_positive_original_starts']==98 and q['reservations_refunded']is False and q['agent_set_limit']is True and q['user_numeric_approval_claimed']is False and q['automatic_repair_starts']==0 and q['queue_limits_increased']is False and q['game_response_deadlines_changed']is False,'No refund, source waiver or unbounded viewport continuation')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Viewport-review implementation changed')
 b=reader.json(q['current_build']['record']);require(reader.digest(q['current_build']['record'])==q['current_build']['sha256']and b['success']and b['sources_stable']and b['old_B04_preserved']and all(reader.digest(n)==h for n,h in(b['compiler_sources']|b['runtime_sources']).items()),'Current native/compiler/runtime source changed')
 require(q['changed_compiler_files']==['apps/qt-shell/acceptance_capture.hpp']and q['changed_runtime_files']==[],'Unreviewed native source delta')
 for ref in q['current_controls']:
  require(reader.digest(ref['record'])==ref['sha256'],'Current control identity changed');c=reader.json(ref['record']);require(c['success']and c.get('sources_stable',True)and all(reader.digest(n)==h for n,h in c.get('sources',{}).items()),'Viewport/native control changed')
 old=q['old_requests'];require(len(old)==25 and len({v['case_id']for v in old})==25 and sum(v['reserved_games']for v in old)==49,'Historical request denominator changed')
 return q


def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['maximum_additional_original_games']==147 and plan['maximum_combined_original_games']==155 and plan['maximum_combined_WP10_and_new_original_games']==155 and plan['minimum_final_additional_reservations']==147 and plan['maximum_manually_reviewed_additional_repairs']==0 and plan['previous_WP10_original_games']==8,'Changed viewport-source bound')
 return 147


def predecessors(reader,rows,plan):
 q=review(reader,plan);old={v['case_id']:v for v in q['old_requests']};seen=set();total=0
 allowed={'case_id','report','reserved_games','state','acquisition_sha256'};full=allowed|{'plan','started_ns','validation'}
 for row in rows:
  require(set(row)in(allowed,full)and row['case_id']not in seen,'Unknown or duplicate request row shape');seen.add(row['case_id'])
  if row['case_id']in old:require(all(old[row['case_id']][k]==v for k,v in row.items()),'Historical source-phase request rewritten')
  else:require(row['case_id'].startswith('bp16-')and row['state']=='admitted'and row['reserved_games']in(1,2)and row['acquisition_sha256']and row['case_id']in row['report'],'Stop after every unknown, incomplete or unreviewed current request')
  total+=row['reserved_games']
 require(set(old)<=seen,'A historical reservation disappeared')
 return total
