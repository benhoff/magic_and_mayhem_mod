"""Use completed input metadata for unstarted cases; preserve one retained reanalysis."""
import json
from pathlib import Path
import broader_player_worker_matrix as legacy
from broader_player_worker_matrix import P,BINARY,replacements,continued_requests as requests
from broader_player_acceptance import checked,native_phase
from native_render_driver import require_nvidia
from rendering_pilot import require

def specs(r):
 catalog=r.json(P+'input-complete-continuation-catalog-01.json');require(catalog['success']and catalog['original_game_runs']==0 and catalog['unstarted_cases']==84 and catalog['original_reports_changed']is False,'Unreviewed input completion');bindings={v['case_id']:v for v in catalog['case_bindings']};require(len(bindings)==84,'Duplicate complete-input case')
 result=[]
 for s in legacy.continued_specs(r):
  key=s['case']['id']
  if key not in bindings:result.append(s);continue
  b=bindings[key];require(b['previous_plan']['record']==s['plan']and r.digest(s['plan'])==b['previous_plan']['sha256'],'Foreign prospective parent');phase=checked(r,b['plan']);case=next(c for c in phase['broader_cases']if c['id']==key)
  require(all(case[k]==s['case'][k]for k in ['id','report','profile','provider','size','reserved_original_games']),'Authored execution context changed');result.append(dict(s,plan=b['plan']['record'],case=case))
 require(len(result)==90 and sum(s['case']['reserved_original_games']for s in result)==98,'Changed90/98positive denominator');return result

def current(root,r,spec,row):
 if spec['case']['id']=='bp13-transition-default800-02':return _current_reanalyzed(root,r,spec,row)
 return legacy.current(root,r,spec,row)

def _current_reanalyzed(root,r,spec,row):
 ref=lambda n:dict(record=n,sha256=r.digest(n));case=spec['case'];require(case['id']=='bp13-transition-default800-02','Only the exact metadata reanalysis may use coordinator02');resume=checked(r,ref(P+case['id']+'-input-resume-review-01.json'));require(resume['original_game_runs']==0 and resume['new_acquisition_or_comparison']==False and resume['old_negative_validation_preserved']and resume['all_original_acceptance_assertions_preserved'],'Unreviewed metadata reanalysis');require(row['state']=='admitted'and row['report']==case['report']and json.loads(row['plan'])==ref(spec['plan']),'Missing or foreign current admission: '+case['id'])
 ar=ref(case['report']);require(row['acquisition_sha256']==ar['sha256'],'Acquisition identity changed');a=checked(r,ar);vr=ref(row['validation']);v=checked(r,vr);require(v['acquisition']==ar and v['sources_stable']and v['profile']==case['profile'],'Mixed current case validation')
 require(all(r.digest(n)==h for n,h in v['sources'].items()),'Current execution or analyzer source changed: '+case['id']);phase=native_phase(root,r,a,BINARY);require(phase['current_native_source_candidate'],'Noncurrent native source phase: '+case['id'])
 role=spec['role'];drivers=[e['resident']['renderer']for e in a['proof']['epochs']]if role=='fault'else[a['resident']['renderer']]
 if case['provider']=='nvidia':require_nvidia(drivers)
 else:require(drivers and all(d.lower().startswith(('llvmpipe','softpipe'))for d in drivers),'Actual software provider missing')
 require(a.get('original_game_runs',1)==case['reserved_original_games'],'Unexpected original start count')
 if role=='fault':require(v['expected_failed_epoch_normal_credit']is False and v['independent_pixels']['mismatching_pixels']==0 and len(v['epoch_progress'])==2,'Failed/fresh epoch boundary weakened')
 elif role=='strict':require(v['functional_acceptance']['verified']and v['stress_seconds']==case['stress_seconds']and v['response_progress']['verified'],'Strict readiness missing')
 else:require(v['lifetime'],'Normal owner retirement missing')
 if role=='component':require(v['whole_viewport_credit']is False,'Selected planes given false viewport credit')
 attempt='02'
 if case['id']=='bp13-loss-default640-01':
  resume=checked(r,ref(P+case['id']+'-preflight-resume-review-01.json'));require(resume['original_game_runs']==0 and resume['original_starts_reserved']==0 and resume['storage_reservation_and_floor_unchanged'],'Unreviewed pilot preflight continuation');attempt='02'
 co=checked(r,ref(P+case['id']+'-coordinator-'+attempt+'.json'));require(co['validation']==vr,'Coordinator names another validation')
 # Raw evidence must already have a positive paired handoff, not just a report.
 handoff=checked(r,ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'Unretained current observations')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers)
