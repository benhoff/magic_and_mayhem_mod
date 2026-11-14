#!/usr/bin/env python3
"""Freeze the complete new B06 source cohort after current no-game controls."""
import argparse,copy,hashlib,json,re,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from broader_player_scheduled_matrix import specs as prior_specs
from broader_player_sources import python_sources
from broader_copy_source_budget import bound,predecessors
from broader_history_capture_contract import POLICY
from broader_delegated_plan import bind_arguments,capture_backing
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser();p.add_argument('--attempt',required=True);a=p.parse_args();rn=P+'COPY-preparation-attempt-'+a.attempt+'.json';require(a.attempt.isdecimal()and not(ROOT/rn).exists(),'Fresh COPY preparation result required');result=dict(schema=1,kind='broader_COPY_source_phase_preparation',success=False,original_game_runs=0)
 try:
  with Reader(ROOT)as r:
   ref=lambda n:dict(record=n,sha256=r.digest(n));reports={}
   def add(n,q):
    raw=(json.dumps(q,indent=2)+'\n').encode();require(n not in reports and not(ROOT/n).exists(),'Never overwrite prospective COPY report: '+n);reports[n]=raw;return dict(record=n,sha256=hashlib.sha256(raw).hexdigest())
   class Draft:
    def digest(self,n):return hashlib.sha256(reports[n]).hexdigest()if n in reports else r.digest(n)
    def json(self,n):return json.loads(reports[n])if n in reports else r.json(n)
   draft=Draft()
   with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
    db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns,case_id')]
   require(len(rows)==70 and sum(v['reserved_games']for v in rows)==102,'Historical 70/102 request snapshot changed');old=prior_specs(r);require(len(old)==90 and sum(s['case']['reserved_original_games']for s in old)==98,'Historical positive denominator changed');ids={s['case']['id']:re.sub(r'^bp[0-9]+-','bp26-',s['case']['id'])for s in old};require(len(set(ids.values()))==90 and not set(ids.values())&{v['case_id']for v in rows},'COPY identities reused')
   br=ref(P+'build-current-06.json');b=r.json(br['record']);require(b['success']and b['sources_stable']and all(r.digest(n)==h for n,h in(b['compiler_sources']|b['runtime_sources']).items()),'B06 input changed');binding=ref(P+'player-build-only-binding-07.json');decl=r.json(binding['record']);require(decl['full_build_sources']==b['compiler_sources']and decl['player_build']['compiler_sources']|decl['configuration_sources']==b['compiler_sources'],'B06 build declaration lost input');staging=ref(P+'COPY-player-staging-06.json');require(r.json(staging['record'])['success'],'B06 staging missing');controls=[ref(P+n+'.json')for n in ['copy-pipeline-native-GPU-GUI-controls-04','copy-pipeline-sanitized-01','native-controls-07']]
   for cr in controls:
    c=r.json(cr['record']);require(c['success']and c.get('sources_stable',True)and all(r.digest(n)==h for n,h in c['sources'].items()),'COPY controls stale')
   reviewer=sorted({'tools/prepare-broader-copy-phase.py','tools/broader_copy_source_budget.py','tools/broader_player_copy_matrix.py','tests/test-broader-copy-budget.py'}|{str(v.relative_to(ROOT))for v in(ROOT/'tools').glob('run-broader-copy-*-case.py')}|{'tools/check-broader-copy-player-case.py','tools/check-broader-copy-hardware-case.py','tools/check-broader-relative-component.py','tools/check-broader-copy-menu.py'})
   t=subprocess.run(['python3','tests/test-broader-copy-budget.py'],capture_output=True,text=True,check=True);test=add(P+'COPY-budget-controls-01.json',dict(schema=1,kind='broader_COPY_budget_controls',success=True,original_game_runs=0,tests=11,sources={n:r.digest(n)for n in reviewer},output=t.stdout+t.stderr))
   review=dict(schema=1,kind='broader_COPY_source_cohort_review',success=True,original_game_runs=0,old_reserved_starts=102,fresh_positive_cases=90,fresh_positive_original_starts=98,new_additional_ceiling=200,new_combined_ceiling=208,old_source_current_credit=False,reservations_refunded=False,automatic_repair_starts=0,agent_set_limit=True,user_numeric_approval_claimed=False,game_response_deadlines_changed=False,reviewer_sources={n:r.digest(n)for n in reviewer},current_controls=controls,current_build=br,corrected_build_binding=binding,staging=staging,old_requests=rows,new_case_reservations={ids[s['case']['id']]:s['case']['reserved_original_games']for s in old},previous_catalog=ref(P+'scheduled-source-catalog-01.json'),negative_custodies=[ref(P+n)for n in ['bp23-world-hotspot-default800-01-capacity-custody-01.json','bp24-world-hotspot-default800-01-capacity-custody-01.json','bp25-world-hotspot-default800-01-accessibility-custody-01.json']],Native_controls_before_Original=True,host_priority_or_affinity_policy=False,reason='Retain every70earlier request102conservative charge and all35historical B05positive contexts without current Native credit. New exact B06 changes only COPY diagnostic CPU preparation,hash sharing after whole-vector equality and bounded50ms admission; all8192COPY/32GiBreadback/512MiB/256job caps,oneFIFOwriter,expected/actualpixelandcanonicalhashassertions remain exact. Current optimized,ASAN/UBSAN,both actualNVIDIA/softwareGPUwrong-output controls,1200COPY physicalGUIcontrols and6recovery controls pass. Full90newcontexts98positiveactualstarts require200additional/208combinedwithWP10eight,zero spare/refund/automaticretry. Use fresh bp26source-specific localcounter namespaces without touching old counters or limits; all oldnegativecauses remain preserved,including scheduledATSPIfailure without an exact stalledstack. First hotspotNVIDIA800 must complete every requiredCOPY,cursor/viewport/lifetime andpairedrestore before remaining89. No old Nativehashwaiver,Originalrecoverypromotion,physicalreset,unboundedmaps/durations orFPSclaim.')
   rr=add(P+'COPY-source-review-01.json',review);g=copy.deepcopy(r.json(P+'plan-25.json'));g.update(kind='broader_current_player_COPY_source_plan',COPY_source_review=rr,previous_plan=ref(P+'plan-25.json'),current_build=br,current_player_binary_sha256=b['player']['sha256'],corrected_build_binding=binding,maximum_additional_original_games=200,maximum_combined_original_games=208,maximum_combined_WP10_and_new_original_games=208,minimum_final_additional_reservations=200,retained_predecessor_reservations=102,retained_current_cases=0,remaining_positive_cases=90,remaining_positive_original_starts=98,request_driver='tools/run-broader-copy-source-case.py',scope=review['reason']);gr=add(P+'plan-26.json',g);require(bound(draft,g)==200 and predecessors(draft,rows,g)==102,'COPY finite guard refused')
   names={n:P+'COPY-26-phase-%02d.json'%i for i,n in enumerate(sorted({s['plan']for s in old}),1)}
   aliases={P+'build-current-05.json':br,P+'player-build-only-binding-06.json':binding,P+'player-build-only-binding-05.json':binding,P+'native-controls-06.json':controls[-1],P+'plan-25.json':gr}
   def adapt(v):
    if isinstance(v,dict):
     if v.get('record')in aliases:return copy.deepcopy(aliases[v['record']])
     return {adapt(k):adapt(x)for k,x in v.items()}
    if isinstance(v,list):return [adapt(x)for x in v]
    if isinstance(v,str):
     if v in names:return names[v]
     for oldid,newid in ids.items():v=v.replace(oldid,newid)
     return v
    return v
   bindings=[];phases=[];source_bytes={};closure_cache={}
   for n in names:
    orig=copy.deepcopy(r.json(n));specs=[s for s in old if s['plan']==n];orig.setdefault('family',specs[0]['family']);wanted={s['case']['id']for s in specs};phase=adapt(orig);phase['broader_cases']=[adapt(c)for c in orig['broader_cases']if c['id']in wanted];phase.update(broader_plan=gr,request_driver='tools/run-broader-copy-source-case.py',supersedes_prospective_plan=ref(n),current_controls=controls[-1],compiler_sources={k:r.digest(k)for k in orig['compiler_sources']},runtime_sources=b['runtime_sources'],player_binary=dict(b['player'],path='working/build/qt-shell/mnm-qt-shell'),current_player_build=br)
    for key in('current_player_build_binding','corrected_build_binding'):
     if key in phase:phase[key]=binding
    if 'oracle'in phase:
     phase['oracle']['native_sources']={k:r.digest(k)for k in orig['oracle']['native_sources']};phase['oracle']['binaries']=[dict(b[k],path='working/build/qt-shell/mnm-acceptance-world-history'if k=='world-history'else'working/original-reference')for k in ['world-history','original-reference']]
    for key in('oracle_sources','clip_oracle_sources'):
     if key in phase:phase[key]={k:r.digest(k)for k in orig[key]}
    phase['historical_source_preimages']=orig.get('source_preimages');phase['source_preimages']=[];phase['counter_bindings']={k:re.sub(r'working/broader-player/bp[0-9]+/','working/broader-player/bp26/',v)for k,v in orig.get('counter_bindings',{}).items()};require(all(not(ROOT/v).exists()for v in phase['counter_bindings'].values()),'New COPY counter exists')
    if 'execution_paths'in phase:phase['execution_paths']={k:re.sub(r'working/broader-player/bp[0-9]+/','working/broader-player/bp26/',v)if isinstance(v,str)else v for k,v in phase['execution_paths'].items()}
    phase['capture_storage']=capture_backing(orig['family'],orig.get('capture_storage',{}));source_names=set(orig['sources'])|set(reviewer)|{'apps/qt-shell/CMakeLists.txt'}
    for c in phase['broader_cases']:
     if c['command'][1]=='tools/run-broader-hotspot-scheduled-world.py':c['command'][1]='tools/run-broader-hotspot-visible-world.py'
     if orig['family']=='hotspot':c['command'][1]='tools/run-broader-hotspot-visible-world.py'
     if 'controller_arguments'in c:
      c['controller_arguments'],proof=bind_arguments(c['controller_arguments'],n,names[n],orig['family'])
      if proof is not None:c['delegated_plan_identity']=proof
    if specs[0]['role']=='world':phase['command_history_policy']=dict(POLICY)
    if orig['family']=='font':
     alias=phase['collector_prefix']+'qualified-font-pilot-B06-01.json';oldalias=orig['positive_pilot_policy']['qualified_comparison_alias'];phase['positive_pilot_policy'].update(first_case='bp26-world-font-default640-01',first_validation=P+'bp26-world-font-default640-01-validation-01.json',qualified_comparison_alias=alias,binding_report=P+'bp26-font-pilot-reference-01.json')
     for c in phase['broader_cases']:c['controller_arguments']=[alias if x==oldalias else x for x in c['controller_arguments']]
    if orig['family']=='strict':phase['actual_context_analysis_plan']=names[n].removesuffix('.json')+'-strict-extent.json'
    for key in list(phase):
     if key.startswith('copy_diagnostic_scheduling')or key.startswith('hotspot_scheduling'):phase.pop(key)
    seed=tuple(sorted(n for n in source_names if n.endswith('.py')));source_names|=closure_cache.setdefault(seed,python_sources(ROOT,seed))if seed not in closure_cache else closure_cache[seed];phase['sources']={n:r.digest(n)for n in sorted(source_names)};source_bytes.update({n:(ROOT/n).read_bytes()for n in source_names});pr=add(names[n],phase);phases.append(pr)
    for s in specs:
     c=next(v for v in phase['broader_cases']if v['id']==ids[s['case']['id']]);require(all(c[k]==s['case'][k]for k in ['profile','provider','size','reserved_original_games']),'COPY finite context changed');bindings.append(dict(previous_case_id=s['case']['id'],case_id=c['id'],plan=pr,role=s['role'],family=s['family'],retained=False))
   analysis={}
   for stem in ['native-edge','world-native-edge','boundary-native-edge']:
    n=P+stem+'-analysis-plan-23.json';v=copy.deepcopy(r.json(n));require(all(r.digest(k)==h for k,h in v['sources'].items()),'Policy oracle source drift');v.update(global_plan=gr,previous_analysis_plan=ref(n));analysis[stem]=add(P+stem+'-analysis-plan-26.json',v)
   for pr in [x for x in phases if draft.json(x['record'])['family']=='strict']:
    n=P+'strict-extent-analysis-plan-16.json';v=copy.deepcopy(r.json(n));v.update(strict_phase=pr,previous_analysis_plan=ref(n));v['sources']={k:r.digest(k)for k in v['sources']};analysis[pr['record']]=add(draft.json(pr['record'])['actual_context_analysis_plan'],v)
   catalog=dict(schema=1,kind='broader_COPY_source_catalog',success=True,original_game_runs=0,global_plan=gr,review=rr,current_build=br,corrected_build_binding=binding,retained_current_cases=0,total_positive_cases=90,total_positive_original_starts=98,case_bindings=bindings,analysis_plans=analysis,prospective_phases=phases,first_pilot='bp26-world-hotspot-default800-01',old_source_current_credit=False,global_player_accepted=False,previous_catalog=review['previous_catalog'])
   require(len(bindings)==90 and len({v['case_id']for v in bindings})==90,'COPY cohort lost cases')
   for pr in phases:
    phase=draft.json(pr['record']);require(all(r.digest(k)==h for k,h in(phase['sources']|phase['compiler_sources']|phase['runtime_sources']).items()),'COPY sources changed')
    for c in phase['broader_cases']:require(pr['record']in c['command']and c['report']in c['command'],'COPY acquisition command unbound')
   with configured(ROOT)as pair:
    with pair.batch():objects={n:dict(sha256=pair.bytes(body),size=len(body))for n,body in source_bytes.items()}
    catalog['source_preimages']=pair.root('broader_COPY_new_source_cohort',ROOT,dict(schema=1,sources=objects))
   cr=add(P+'COPY-source-catalog-01.json',catalog)
   for n,raw in reports.items():require(write_report(ROOT,n,raw)==hashlib.sha256(raw).hexdigest(),'COPY report changed')
   result.update(success=True,catalog=cr,global_plan=gr,review=rr,first_pilot=catalog['first_pilot'],fresh_cases=90,fresh_positive_original_starts=98)
 except Exception as error:
  import traceback
  traceback.print_exc();result['error']=repr(error)
 op=rn.removesuffix('.json')+'-operator.bin';result['operator']=dict(record=op,sha256=write_report(ROOT,op,Path(__file__).read_bytes()));h=write_report(ROOT,rn,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))));return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
