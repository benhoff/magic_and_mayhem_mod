"""Exact finite B06 source cohort; never refund or repeat a historical request."""
from rendering_pilot import require

def review(reader,plan):
 ref=plan['COPY_source_review'];require(reader.digest(ref['record'])==ref['sha256'],'COPY source review identity changed');q=reader.json(ref['record'])
 require(q['kind']=='broader_COPY_source_cohort_review'and q['success']and q['original_game_runs']==0 and q['old_reserved_starts']==103 and q['fresh_positive_cases']==90 and q['fresh_positive_original_starts']==98 and q['new_additional_ceiling']==201 and q['new_combined_ceiling']==209 and q['old_source_current_credit']is False and q['reservations_refunded']is False and q['automatic_repair_starts']==0 and q['game_response_deadlines_changed']is False,'Changed finite COPY source domain')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'COPY reviewer source changed')
 for ref in q['current_controls']:
  require(reader.digest(ref['record'])==ref['sha256'],'COPY control identity changed');c=reader.json(ref['record']);require(c['success']and c.get('sources_stable',True)and all(reader.digest(n)==h for n,h in c['sources'].items()),'COPY control source changed')
 ref=q['current_build'];require(reader.digest(ref['record'])==ref['sha256'],'COPY build identity changed');b=reader.json(ref['record']);require(b['success']and b['sources_stable']and all(reader.digest(n)==h for n,h in(b['compiler_sources']|b['runtime_sources']).items()),'COPY Native/compiler/runtime source changed')
 old=q['old_requests'];new=q['new_case_reservations'];require(len(old)==71 and len({v['case_id']for v in old})==71 and sum(v['reserved_games']for v in old)==103,'Historical COPY reservations changed');require(len(new)==90 and sum(new.values())==98 and set(new).isdisjoint(v['case_id']for v in old)and all(type(n)is int and n in(1,2)for n in new.values()),'Changed new COPY reservations')
 return q

def bound(reader,plan):
 review(reader,plan);require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==201 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==209,'COPY cumulative ceiling changed');return 201

def predecessors(reader,rows,plan):
 q=review(reader,plan);old={v['case_id']:v for v in q['old_requests']};new=q['new_case_reservations'];seen=set();total=0
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate COPY request row');seen.add(cid)
  if cid in old:require(all(old[cid][k]==v for k,v in row.items()),'Historical COPY request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after an unknown/failed/incomplete COPY request')
  total+=row['reserved_games']
 require(set(old)<=seen,'Historical COPY reservation disappeared');require(total<=201,'COPY source budget exceeded');return total
