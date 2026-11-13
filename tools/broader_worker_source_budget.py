"""Preserve all B02 charges while requiring a wholly fresh source-bound cohort."""
from rendering_pilot import require

def review(reader,plan):
 ref=plan['recorder_source_review'];require(reader.digest(ref['record'])==ref['sha256'],'Recorder source review changed');q=reader.json(ref['record'])
 require(q['kind']=='broader_recorder_source_continuation_review'and q['success']is True and q['original_game_runs']==0 and q['old_source_current_credit']is False and q['old_reserved_starts']==30 and q['old_additional_ceiling']==126 and q['new_additional_ceiling']==128 and q['old_combined_ceiling']==134 and q['new_combined_ceiling']==136 and q['fresh_positive_cases']==90 and q['fresh_positive_original_starts']==98 and q['reservations_refunded']is False and q['agent_set_limit']is True and q['user_numeric_approval_claimed']is False and q['automatic_repair_starts']==0 and q['queue_limits_increased']is False,'No refund, source waiver or unbounded source continuation')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Source-review implementation changed')
 build=q['current_build'];require(reader.digest(build['record'])==build['sha256'],'Current build identity changed');b=reader.json(build['record']);require(b['success']and b['sources_stable']and b['old_B03_preserved']and all(reader.digest(n)==h for n,h in (b['compiler_sources']|b['runtime_sources']).items()),'Current native/compiler/runtime source changed')
 require(q['changed_compiler_files']==['apps/qt-shell/gui_surface_loss_probe.hpp']and q['changed_runtime_files']==[],'Unreviewed native source delta')
 for ref in q['current_controls']:
  require(reader.digest(ref['record'])==ref['sha256'],'Current control identity changed');c=reader.json(ref['record']);require(c['success']and c.get('sources_stable',True)and all(reader.digest(n)==h for n,h in c.get('sources',{}).items()),'Recorder/native control changed')
 old=q['old_requests'];require(len(old)==15 and len({v['case_id']for v in old})==15 and sum(v['reserved_games']for v in old)==30,'Historical request denominator changed')
 return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['maximum_additional_original_games']==128 and plan['maximum_combined_original_games']==136 and plan['maximum_combined_WP10_and_new_original_games']==136 and plan['minimum_final_additional_reservations']==128 and plan['maximum_manually_reviewed_additional_repairs']==0 and plan['previous_WP10_original_games']==8,'Changed source-phase bound')
 return 128

def predecessors(reader,rows,plan):
 q=review(reader,plan);old={v['case_id']:v for v in q['old_requests']};seen=set();total=0
 allowed={'case_id','report','reserved_games','state','acquisition_sha256'};full=allowed|{'plan','started_ns','validation'}
 for row in rows:
  require(set(row)in(allowed,full),'Unreviewed request row shape')
  if row['case_id']in old:
   require(row['case_id']not in seen and all(old[row['case_id']][k]==v for k,v in row.items()),'Historical source-phase request rewritten');seen.add(row['case_id'])
  else:
   require(row['case_id'].startswith('bp13-')and row['state']=='admitted'and row['reserved_games']in(1,2)and row['acquisition_sha256']and row['case_id']in row['report'],'Stop after every unknown, incomplete or unreviewed current request')
  total+=row['reserved_games']
 require(seen==set(old),'A historical reservation disappeared')
 return total
