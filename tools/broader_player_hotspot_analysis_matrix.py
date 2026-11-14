"""Exact retained hotspot reanalysis beside the unchanged acquisition readers."""
import json
from pathlib import Path
from broader_player_copy_matrix import P,BINARY,requests,replacements,replacement
from broader_player_copy_inputs_matrix import specs
from broader_player_acceptance import checked,native_phase
from native_render_driver import require_nvidia
from rendering_pilot import require

def analysis_binding(root,r,spec,row):
 """Resolve an exact retained-data revalidation without rewriting request history."""
 name=P+spec['case']['id']+'-analysis-revalidation-01.json'
 if not(Path(root)/name).exists():return None
 ref=lambda n:dict(record=n,sha256=r.digest(n))
 b=checked(r,ref(name))
 require(spec['family']=='hotspot'and spec['role']=='world'
     and spec['case']['id']in('bp27-world-hotspot-default640-01','bp27-world-hotspot-default800-01')
     and b['kind']=='broader_retained_hotspot_analysis_revalidation'
     and b['case']==spec['case']['id']and b['original_game_runs']==b['new_native_executions']==0
     and b['new_pixel_comparisons']==1 and b['Native_ELF_sha256']==BINARY,
     'Unknown retained hotspot analysis revalidation')
 require(b['request_identity']=={k:row[k]for k in('case_id','report','plan','reserved_games','started_ns','acquisition_sha256')}
     and b['ledger_validation']==row['validation']and b['acquisition']==ref(spec['case']['report'])
     and b['plan']==ref(spec['plan'])and b['source_phase_waivers']==[],
     'Retained analysis request/acquisition identity changed')
 old=checked(r,b['original_coordinator'])if b['original_coordinator_success']else r.json(b['original_coordinator']['record'])
 require(r.digest(b['original_coordinator']['record'])==b['original_coordinator']['sha256']
     and old['success']is b['original_coordinator_success']
     and b['original_coordinator']==ref(P+spec['case']['id']+'-coordinator-01.json'),
     'Original analysis coordinator changed')
 if b['original_validation']is not None:
  require(b['original_validation']==ref(row['validation'])and old['validation']==b['original_validation'],
      'Original admitted validation changed')
 require(b['raw_capture_and_Native_inputs_unchanged']is True
     and b['historical_failed_comparison_preserved']is True
     and b['sources']and all(r.digest(n)==h for n,h in b['sources'].items()),
     'Retained analysis review is stale or weakens custody')
 for key in('current_validation','current_coordinator','current_handoff'):
  checked(r,b[key])
 return dict(reference=ref(name),value=b)


def current(root,r,spec,row):
 ref=lambda n:dict(record=n,sha256=r.digest(n));case=spec['case'];revalidation=analysis_binding(root,r,spec,row);binding=revalidation['value']if revalidation else None;require(row['state']=='admitted'and row['report']==case['report']and json.loads(row['plan'])==ref(spec['plan']),'Missing/foreign B06 admission: '+case['id']);ar=ref(case['report']);require(row['acquisition_sha256']==ar['sha256'],'B06 acquisition changed');a=checked(r,ar);vr=binding['current_validation']if binding else ref(row['validation']);v=checked(r,vr);require(v['acquisition']==ar and v['sources_stable']and v['profile']==case['profile']and all(r.digest(n)==h for n,h in v['sources'].items()),'B06 mixed/stale validation');phase=native_phase(root,r,a,BINARY);require(phase['current_native_source_candidate'],'Noncurrent B06 Native source candidate');role=spec['role'];drivers=[e['resident']['renderer']for e in a['proof']['epochs']]if role=='fault'else[a['resident']['renderer']]
 if case['provider']=='nvidia':require_nvidia(drivers)
 else:require(drivers and all(d.lower().startswith(('llvmpipe','softpipe'))for d in drivers),'B06 software provider missing')
 require(a.get('original_game_runs',1)==case['reserved_original_games'],'B06 actual start count differs')
 if role=='fault':require(v['expected_failed_epoch_normal_credit']is False and v['independent_pixels']['mismatching_pixels']==0 and len(v['epoch_progress'])==2,'B06 failed/fresh boundary weakened')
 elif role=='strict':require(v['functional_acceptance']['verified']and v['stress_seconds']==case['stress_seconds']and v['response_progress']['verified'],'B06 strict readiness missing')
 else:require(v['lifetime'],'B06 normal retirement missing')
 if role=='component':require(v['whole_viewport_credit']is False,'B06 selected planes given viewport credit')
 co_ref=binding['current_coordinator']if binding else ref(P+case['id']+'-coordinator-01.json');co=checked(r,co_ref);require(co['validation']==vr,'B06 coordinator mismatch');sources=dict(co.get('sources',{}))|(binding['sources']if binding else {})
 for key in('source','retention_operator'):
  if co.get(key):sources[co[key]['path']]=co[key]['sha256']
 if role=='strict':sources['tools/run-broader-copy-strict-case.py']=r.json(spec['plan'])['sources']['tools/run-broader-copy-strict-case.py']
 require(sources and all(r.digest(n)==h for n,h in sources.items()),'B06 coordinator source changed');handoff=checked(r,binding['current_handoff']if binding else ref(P+case['id']+'-handoff-01.json'));require(handoff['validation']==vr and handoff['results']and all(x['real_restore_verified']and x['replicas_verified']==2 for x in handoff['results']),'B06 raw evidence unretained')
 return dict(spec=spec,validation=vr,acquisition=ar,value=v,acquired=a,phase=phase,drivers=drivers,coordinator=co_ref,coordinator_sources=sources,current_sources=v['sources'],publication_applicability=[],analysis_revalidation=revalidation['reference']if revalidation else None)
