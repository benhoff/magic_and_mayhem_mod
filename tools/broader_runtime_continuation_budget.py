"""Finite new runtime cohort; exact historical charges are never refunded."""
import re
from broader_player_acceptance import checked
from rendering_pilot import require

def review(reader,plan):
 q=checked(reader,plan['runtime_continuation_review'])
 require(q['kind']=='reviewed_fading_particle_runtime_continuation'and q['original_game_runs']==0
         and q['old_source_current_credit']is False and q['automatic_retries']is False
         and q['reservations_refunded']is False,'Unreviewed runtime continuation')
 old=q['old_requests'];new=q['new_case_reservations']
 require(len(old)==93 and len({v['case_id']for v in old})==93 and sum(v['reserved_games']for v in old)==133,'Changed historical93/133 denominator')
 require(len(new)==90 and sum(new.values())==98 and all(k.startswith('bp29-')and type(v)is int and v in(1,2)for k,v in new.items())and set(new).isdisjoint(v['case_id']for v in old),'Changed positive90/98 domain')
 catalog=checked(reader,q['prior_catalog']);expected={}
 for b in catalog['case_bindings']:
  phase=checked(reader,b['plan']);case=next(v for v in phase['broader_cases']if v['id']==b['case_id']);expected[re.sub(r'^bp[0-9]+-','bp29-',case['id'])]=case['reserved_original_games']
 require(new==expected,'Runtime positive declaration differs from the reviewed parent domain')
 require(q['maximum_additional_charges']==231 and q['maximum_combined_with_WP10']==239
         and q['prior_contexts_withheld_from_current_credit']==20,'Changed reviewed runtime bound')
 failure=checked(reader,q['failed_attachment_diagnosis']);require(failure['direct_failure']==dict(World=205,sequence=611,code=43,tag=14,caller='0x54a563',caller_rva='0x14a563',Original_exit=95,request_published=False)and failure['failed_request']in old,'Missing exact failed runtime request')
 controls=checked(reader,q['controls']);require(controls['sources_stable']and controls['fading']['context_pairs']==10 and controls['fading']['original_primary_cases']==240 and controls['old-sprite']['success'],'Missing current offline caller controls')
 require(all(reader.digest(n)==h for n,h in(q['reviewer_sources']|controls['sources']).items()),'Runtime continuation control source changed')
 return q

def bound(reader,plan):
 review(reader,plan)
 require(plan['success']and plan['previous_WP10_original_games']==8 and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==231 and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==239,'Reviewed231/239 ceiling changed')
 return 231

def predecessors(reader,rows,plan):
 q=review(reader,plan);fixed={v['case_id']:v for v in q['old_requests']};new=q['new_case_reservations'];seen=set();total=0
 minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
 for row in rows:
  cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate runtime request');seen.add(cid)
  if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical runtime request rewritten')
  else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]and row['acquisition_sha256']and cid in row['report'],'Stop after unknown/failed/incomplete new runtime request')
  total+=row['reserved_games']
 require(set(fixed)<=seen and total<=231,'Historical request omitted or231ceiling exceeded')
 return total
