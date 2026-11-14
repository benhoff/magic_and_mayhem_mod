"""Read only wholly fresh admitted B05 cases; no historical Native source waiver."""
import json,sqlite3
from pathlib import Path
from broader_player_acceptance import checked,native_phase
from broader_viewport_source_budget import bound,predecessors
from broader_player_worker_matrix import replacements,replacement
from native_render_driver import require_nvidia
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
BINARY='275da459c0fa30750efa7f1623921cb5bd0bd775b3c478b871b88a8bf9fbd31e'
def specs(r):
 c=checked(r,dict(record=P+'viewport-source-catalog-01.json',sha256=r.digest(P+'viewport-source-catalog-01.json')));require(c['total_positive_cases']==90 and c['total_positive_original_starts']==98 and c['retained_current_cases']==0 and c['old_source_current_credit']is False,'Fresh90/98cohort required');result=[]
 for b in c['case_bindings']:
  phase=checked(r,b['plan']);case=next(v for v in phase['broader_cases']if v['id']==b['case_id']);result.append(dict(plan=b['plan']['record'],case=case,role=b['role'],family=b['family']))
 require(len(result)==90 and len({v['case']['id']for v in result})==90 and sum(v['case']['reserved_original_games']for v in result)==98,'Changed fresh cohort denominator');return result

def requests(root,r):
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
 g=r.json(P+'plan-16.json');maximum=bound(r,g);charge=predecessors(r,rows,g);require(charge<=maximum,'Cumulative start ceiling exceeded');return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,prior_WP10_original_starts=8,maximum_combined_original_starts=155,all_reservations_retained=True,rows=rows)

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
 co=checked(r,ref(P+case['id']+'-coordinator-'+attempt+'.json'));require(co['validation']==vr,'Coordinator names another validation')
 # Raw evidence must already have a positive paired handoff, not just a report.
 handoff=checked(r,ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'Unretained current observations')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers)
