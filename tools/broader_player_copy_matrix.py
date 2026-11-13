"""Complete fresh B06 player matrix, without historical Native source waivers."""
import json,sqlite3
from pathlib import Path
from broader_player_acceptance import checked,native_phase
from broader_player_worker_matrix import replacements,replacement
from broader_copy_source_budget import bound,predecessors
from native_render_driver import require_nvidia
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
BINARY='8d606f2ce02514ab21b82f2e0ff5ec515320f01e93386971bcb1fb6c87d080ca'
def specs(r):
 c=checked(r,dict(record=P+'COPY-source-catalog-02.json',sha256=r.digest(P+'COPY-source-catalog-02.json')));require(c['total_positive_cases']==90 and c['total_positive_original_starts']==98 and c['retained_current_cases']==0 and c['old_source_current_credit']is False,'Fresh B06 90/98domain required');out=[]
 for b in c['case_bindings']:
  p=checked(r,b['plan']);case=next(v for v in p['broader_cases']if v['id']==b['case_id']);out.append(dict(plan=b['plan']['record'],case=case,role=b['role'],family=b['family']))
 require(len(out)==90 and len({v['case']['id']for v in out})==90 and sum(v['case']['reserved_original_games']for v in out)==98,'B06 denominator changed');return out

def requests(root,r):
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
 g=r.json(P+'plan-27.json');limit=bound(r,g);charge=predecessors(r,rows,g);return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=limit,prior_WP10_original_starts=8,maximum_combined_original_starts=209,all_reservations_retained=True,rows=rows)

def current(root,r,spec,row):
 ref=lambda n:dict(record=n,sha256=r.digest(n));case=spec['case'];require(row['state']=='admitted'and row['report']==case['report']and json.loads(row['plan'])==ref(spec['plan']),'Missing/foreign B06 admission: '+case['id']);ar=ref(case['report']);require(row['acquisition_sha256']==ar['sha256'],'B06 acquisition changed');a=checked(r,ar);vr=ref(row['validation']);v=checked(r,vr);require(v['acquisition']==ar and v['sources_stable']and v['profile']==case['profile']and all(r.digest(n)==h for n,h in v['sources'].items()),'B06 mixed/stale validation');phase=native_phase(root,r,a,BINARY);require(phase['current_native_source_candidate'],'Noncurrent B06 Native source candidate');role=spec['role'];drivers=[e['resident']['renderer']for e in a['proof']['epochs']]if role=='fault'else[a['resident']['renderer']]
 if case['provider']=='nvidia':require_nvidia(drivers)
 else:require(drivers and all(d.lower().startswith(('llvmpipe','softpipe'))for d in drivers),'B06 software provider missing')
 require(a.get('original_game_runs',1)==case['reserved_original_games'],'B06 actual start count differs')
 if role=='fault':require(v['expected_failed_epoch_normal_credit']is False and v['independent_pixels']['mismatching_pixels']==0 and len(v['epoch_progress'])==2,'B06 failed/fresh boundary weakened')
 elif role=='strict':require(v['functional_acceptance']['verified']and v['stress_seconds']==case['stress_seconds']and v['response_progress']['verified'],'B06 strict readiness missing')
 else:require(v['lifetime'],'B06 normal retirement missing')
 if role=='component':require(v['whole_viewport_credit']is False,'B06 selected planes given viewport credit')
 co_ref=ref(P+case['id']+'-coordinator-01.json');co=checked(r,co_ref);require(co['validation']==vr,'B06 coordinator mismatch');sources=dict(co.get('sources',{}))
 for key in('source','retention_operator'):
  if co.get(key):sources[co[key]['path']]=co[key]['sha256']
 if role=='strict':sources['tools/run-broader-copy-strict-case.py']=r.json(spec['plan'])['sources']['tools/run-broader-copy-strict-case.py']
 require(sources and all(r.digest(n)==h for n,h in sources.items()),'B06 coordinator source changed');handoff=checked(r,ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'B06 raw evidence unretained')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers,coordinator=co_ref,coordinator_sources=sources,current_sources=v['sources'],publication_applicability=[])
