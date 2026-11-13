"""Conjoin completed B05 source inputs and one zero-start storage continuation."""
import json
from broader_player_viewport_matrix import P,BINARY,replacements
from broader_arguments_source_budget import bound,predecessors
from broader_publication_applicability import reviewed_sources
from pathlib import Path
import sqlite3
from broader_player_acceptance import checked,native_phase
from native_render_driver import require_nvidia
from rendering_pilot import require

def specs(r):
 c=checked(r,dict(record=P+'arguments-source-catalog-01.json',sha256=r.digest(P+'arguments-source-catalog-01.json')))
 require(c['retained_current_cases']==18 and c['remaining_positive_cases']==72 and c['total_positive_cases']==90 and c['total_positive_original_starts']==98 and c['native_source_waivers']==[],'Changed publication cohort denominator')
 result=[]
 for b in c['case_bindings']:
  phase=checked(r,b['plan']);case=next(v for v in phase['broader_cases']if v['id']==b['case_id']);result.append(dict(plan=b['plan']['record'],case=case,role=b['role'],family=b['family']))
 require(len(result)==90 and len({v['case']['id']for v in result})==90 and sum(v['case']['reserved_original_games']for v in result)==98,'Changed complete cohort')
 return result

def requests(root,r):
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
 g=r.json(P+'plan-21.json');maximum=bound(r,g);charge=predecessors(r,rows,g);require(charge<=maximum,'Cumulative start ceiling exceeded')
 return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,prior_WP10_original_starts=8,maximum_combined_original_starts=160,all_reservations_retained=True,rows=rows)

def coordinator_sources(r,phase,co,role):
 result=dict(co.get('sources',{}))
 if co.get('source'):result[co['source']['path']]=co['source']['sha256']
 if co.get('retention_operator'):result[co['retention_operator']['path']]=co['retention_operator']['sha256']
 if role=='strict':
  require(co['kind']=='broader_strict_case_coordinator','Unknown strict coordinator schema')
  global_plan=checked(r,phase['broader_plan']);new=phase.get('request_driver')=='tools/run-broader-publication-source-case.py';review=checked(r,global_plan['journal_publication_review'if new else'viewport_source_review'])
  name='tools/run-broader-publication-strict-case.py'if new else'tools/run-broader-viewport-strict-case.py'
  require(phase['sources'].get(name)==review['reviewer_sources'].get(name)and name in phase['sources'],'Strict coordinator not pinned by the execution plan and source review')
  for n in(name,'tools/retire-broader-strict.py'):
   require(n in phase['sources'],'Missing strict orchestration source pin')
   h=phase['sources'][n];require(n not in result or result[n]==h,'Conflicting strict orchestration fingerprints');result[n]=h
 require(result,'Missing coordinator provenance')
 return result

def current(root,r,spec,row):
 ref=lambda n:dict(record=n,sha256=r.digest(n));case=spec['case'];require(row['state']=='admitted'and row['report']==case['report']and json.loads(row['plan'])==ref(spec['plan']),'Missing or foreign current admission: '+case['id'])
 ar=ref(case['report']);require(row['acquisition_sha256']==ar['sha256'],'Acquisition identity changed');a=checked(r,ar);vr=ref(row['validation']);v=checked(r,vr);require(v['acquisition']==ar and v['sources_stable']and v['profile']==case['profile'],'Mixed current case validation')
 catalog=checked(r,ref(P+'arguments-source-catalog-01.json'));identity=dict(case_id=case['id'],acquisition=ar,validation=vr);current_sources,publication_review=reviewed_sources(r,v['sources'],identity,catalog['publication_applicability'],purpose='validation');phase=native_phase(root,r,a,BINARY);require(phase['current_native_source_candidate'],'Noncurrent native source phase: '+case['id'])
 role=spec['role'];drivers=[e['resident']['renderer']for e in a['proof']['epochs']]if role=='fault'else[a['resident']['renderer']]
 if case['provider']=='nvidia':require_nvidia(drivers)
 else:require(drivers and all(d.lower().startswith(('llvmpipe','softpipe'))for d in drivers),'Actual software provider missing')
 require(a.get('original_game_runs',1)==case['reserved_original_games'],'Unexpected original start count')
 if role=='fault':require(v['expected_failed_epoch_normal_credit']is False and v['independent_pixels']['mismatching_pixels']==0 and len(v['epoch_progress'])==2,'Failed/fresh epoch boundary weakened')
 elif role=='strict':require(v['functional_acceptance']['verified']and v['stress_seconds']==case['stress_seconds']and v['response_progress']['verified'],'Strict readiness missing')
 else:require(v['lifetime'],'Normal owner retirement missing')
 if role=='component':require(v['whole_viewport_credit']is False,'Selected planes given false viewport credit')
 attempt='01'
 if case['id']=='bp20-world-attachment-default640-01':
  review=checked(r,ref(P+'relative-preferences-admission-review-01.json'));require(review['original_game_runs']==review['new_request_reservations']==review['new_native_executions']==review['new_pixel_comparisons']==0 and review['acquisition']==ar and review['new_validation']==vr and review['all_other_world_assertions_byte_identical']and not review['native_code_or_limits_changed']and not review['pixel_or_lifetime_assertions_waived']and all(r.digest(n)==h for n,h in review['sources'].items()),'Unreviewed retained preference analysis');attempt='02'
 if case['id']in('bp16-boundary-default800-01','bp16-boundary-software800-01'):
  resume=checked(r,ref(P+case['id']+'-preflight-resume-review-01.json'));require(resume['original_game_runs']==0 and resume['original_starts_reserved']==0 and resume['storage_reservation_and_floor_unchanged']and resume['game_deadlines_and_actions_unchanged']and resume['request_case_and_global_plan_unchanged'],'Unreviewed zero-start storage continuation');attempt='02'
 co_ref=ref(P+case['id']+'-coordinator-'+attempt+'.json');co=checked(r,co_ref);require(co['validation']==vr,'Coordinator names another validation')
 orchestration_sources=coordinator_sources(r,r.json(spec['plan']),co,role)
 orchestration_sources,coordinator_review=reviewed_sources(r,orchestration_sources,identity,catalog['publication_applicability'],purpose='coordinator')
 if attempt=='02'and role=='boundary':
  rr=co['zero_start_preflight_resume'];require(rr==ref(P+case['id']+'-preflight-resume-review-01.json')and co['coordinator_actions_deadlines_and_reservations_unchanged'],'Missing exact zero-start continuation');review=checked(r,rr);require(r.digest(review['operator']['record'])==review['operator']['sha256'],'Continuation operator changed')
 # Raw evidence must already have a positive paired handoff, not just a report.
 handoff=checked(r,ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'Unretained current observations')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers,coordinator=co_ref,coordinator_sources=orchestration_sources,current_sources=current_sources,publication_applicability=[v for v in(publication_review,coordinator_review)if v])
