"""Load admitted current replacements without refreshing any historical execution."""
import json,sqlite3
from pathlib import Path
from broader_player_acceptance import checked,native_phase
from broader_worker_source_budget import predecessors as reviewed_with_setup_refusals
from broader_worker_source_budget import bound as cumulative_bound
from native_render_driver import require_nvidia
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
BINARY='449b1758070ae151646f6afe6e0530669a8f3d7b3a64bc9374f611456745eef3'
def specs(r):
 result=[]
 for group in('world','menu','component'):
  bundle=r.json(P+group+'-request-plans-bp13-01.json');require(bundle['success'],'Failed prospective bundle')
  for item in bundle['plans']:
   phase=checked(r,item)
   result.extend(dict(plan=item['record'],case=v,role=group,family=phase['family'])for v in phase['broader_cases'])
 require(len(result)==76,'Changed normal primary cohort')
 for family,prefix in [('loss','surface-loss-restore-results-20261109'),('transition','rendering-transition-recovery-results-20261107')]:
  name='research/runtime/'+prefix+'/bp13-input-plan-01.json';phase=r.json(name);result.extend(dict(plan=name,case=v,role='fault',family=family)for v in phase['broader_cases'])
 require(len(result)==84 and len({v['case']['id']for v in result})==84,'Changed primary cohort')
 for name,role in[(P+'strict-input-plan-13.json','strict'),(P+'boundary-input-plan-13.json','boundary')]:
  phase=r.json(name);result.extend(dict(plan=name,case=v,role=role,family=role)for v in phase['broader_cases'])
 require(len(result)==90 and len({v['case']['id']for v in result})==90,'Changed supplemental cohort')
 return result

def requests(root,r):
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select * from requests order by started_ns')]
 g=r.json(P+'plan-13.json');maximum=cumulative_bound(r,g);charge=reviewed_with_setup_refusals(r,rows,g);require(charge<=maximum,'Cumulative original start ceiling exceeded');return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,prior_WP10_original_starts=8,maximum_combined_original_starts=136,all_reservations_retained=True,rows=rows)

def current(root,r,spec,row):
 ref=lambda n:dict(record=n,sha256=r.digest(n));case=spec['case'];require(row['state']=='admitted'and row['report']==case['report']and json.loads(row['plan'])==ref(spec['plan']),'Missing or foreign current admission: '+case['id'])
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
 attempt='01'
 if case['id']=='bp13-loss-default640-01':
  resume=checked(r,ref(P+case['id']+'-preflight-resume-review-01.json'));require(resume['original_game_runs']==0 and resume['original_starts_reserved']==0 and resume['storage_reservation_and_floor_unchanged'],'Unreviewed pilot preflight continuation');attempt='02'
 co=checked(r,ref(P+case['id']+'-coordinator-'+attempt+'.json'));require(co['validation']==vr,'Coordinator names another validation')
 # Raw evidence must already have a positive paired handoff, not just a report.
 handoff=checked(r,ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'Unretained current observations')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers)

def replacement(record,historical,*,binding):
 v=record['value'];role=record['spec']['role'];return dict(historical_primary=historical,profile=v['profile'],success=v['success'],sources_stable=v['sources_stable'],current_native_phase=record['phase'],accepted=True,role='world'if role=='boundary'else role,validation=record['validation'],acquisition=record['acquisition'],whole_viewport_credit=v.get('whole_viewport_credit',role in('world','boundary')),semantic_binding=binding)

def replacements(r,inventory,records):
 mapping={};by_context={}
 for rec in records:
  s=rec['spec'];c=s['case'];key=(s['family'],c['profile'],tuple(c['size']));require(key not in by_context or s['role']=='strict','Duplicate current semantic context');by_context[key]=rec
  if s['role']in('world','component'):
   old=c['historical_primary']['acquisition'];key=(old['record'],old['sha256']);require(key not in mapping,'Duplicate historical replacement');mapping[key]=replacement(rec,old,binding=dict(kind='same-authored-finite-route',historical_pair=c['historical_primary'],current_family=s['family'],original_and_native_source_phases_preserved=True))
 for item in inventory['cases']:
  old=item['acquisition'];key=(old['record'],old['sha256'])
  if key in mapping:continue
  a=r.json(old['record']);profile=item['profile'];roles=item['roles'];cfg=a['configuration'];size=tuple([800,600]if cfg.get('high_resolution',False)else[640,480])
  if'historical-semantic-menu-or-movie'in roles:
   families={'rendering-main-setup-results':'menu','rendering-enabled-intros-results':'intros','rendering-movie-skip-results':'skip'};family=next((v for k,v in families.items()if k in old['record']),None)
   if family is None:
    require('rendering-functional-ten-results'in old['record'],'Unknown semantic primary');family='defeat';size=(800,600);binding=dict(kind='natural-defeat-results-replaces-forced-results',historical_native_test_driver_retained=True,natural_result_required=True,normal_results_to_Main_return_required=True)
   else:binding=dict(kind='same-native-menu-or-media-semantic-policy',current_family=family)
  elif'historical-failed-and-fresh-epochs'in roles:
   family='loss'if'surface-loss-restore-results'in old['record']else'transition';size=tuple(a.get('size',cfg.get('actual_size',cfg.get('size',[]))))
   if not size:
    # Exact historical fault case metadata, never a pathname-size guess.
    sizes={tuple(e['size'])for e in a['proof']['epochs']if'size'in e};require(len(sizes)==1,'Missing original fault extent');size=sizes.pop()
   binding=dict(kind='same-declared-fault-and-fresh-generation',historical_failure_preserved=True,normal_failed_epoch_credit=False)
  elif'current-WP10-finite-session'in roles:
   family='quick'if a['scenario']=='normal-quick'else'storage';require(a['scenario']in('normal-quick','normal-storage'),'Unknown old finite WP10 scenario');size=tuple(cfg.get('actual_size',cfg.get('size',[])))
   # The paired output establishes the original extent independently.
   if not size:
    from pixels_history_gate import Join
    output=Join(r).pair(item['pairs'][0],profile);size=tuple(output['size'])
   binding=dict(kind='current-finite-route-followup',old_WP10_execution_remains_historical=True,old_source_phase_promoted=False)
  else:raise ValueError('Unmapped inventory primary: '+old['record'])
  context=(family,profile,size);require(context in by_context,'Missing exact current semantic context: '+repr(context));mapping[key]=replacement(by_context[context],old,binding=binding)
 require(set(mapping)=={(v['acquisition']['record'],v['acquisition']['sha256'])for v in inventory['cases']},'Inventory mapping lost or introduced a primary')
 return mapping


def continued_specs(r):
 """Retain five complete cases; replace only the refused context and unstarted plans."""
 catalog=r.json(P+'drain-continuation-catalog-01.json')
 require(catalog['success']and catalog['original_game_runs']==0 and catalog['retained_current_cases']==5 and catalog['remaining_positive_cases']==85,'Changed continuation inventory')
 replacements_by_old={v['previous_case_id']:v for v in catalog['case_bindings']}
 require(len(replacements_by_old)==85,'Duplicate continuation binding')
 result=[]
 for item in specs(r):
  key=item['case']['id']
  if key not in replacements_by_old:result.append(item);continue
  value=replacements_by_old[key];phase=checked(r,value['plan']);case=next(c for c in phase['broader_cases']if c['id']==value['case_id'])
  require(case['profile']==item['case']['profile']and case['size']==item['case']['size']and case['provider']==item['case']['provider']and case['reserved_original_games']==item['case']['reserved_original_games'],'Changed authored finite execution context')
  result.append(dict(item,plan=value['plan']['record'],case=case))
 require(len(result)==90 and len({v['case']['id']for v in result})==90 and sum(v['case']['reserved_original_games']for v in result)==98,'Changed complete positive denominator')
 return result


def continued_requests(root,r):
 from broader_drained_source_budget import bound,predecessors
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
 plan=r.json(P+'plan-14.json');maximum=bound(r,plan);charge=predecessors(r,rows,plan);require(charge<=maximum,'Continuation original-start ceiling exceeded')
 return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,prior_WP10_original_starts=8,maximum_combined_original_starts=138,all_reservations_retained=True,rows=rows)
