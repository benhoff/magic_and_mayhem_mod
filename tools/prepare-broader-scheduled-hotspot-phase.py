#!/usr/bin/env python3
"""Prepare only data/source declarations after the retained font recorder-cap loss."""
import argparse,copy,hashlib,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from broader_player_sources import python_sources
from broader_player_hotspot_matrix import specs as previous_specs
from broader_player_scheduled_matrix import specs,current
from broader_scheduled_source_budget import bound,predecessors
from rendering_pilot import require
import importlib.util
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
sp=importlib.util.spec_from_file_location('publication_planning_overlay',ROOT/'tools/prepare-broader-publication-phase.py');previous=importlib.util.module_from_spec(sp);sp.loader.exec_module(previous);Proposed=previous.Proposed

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--attempt',required=True);a=p.parse_args();require(a.attempt.isdecimal(),'Numeric attempt required');rn=P+'scheduled-preparation-attempt-'+a.attempt+'.json';require(not(ROOT/rn).exists(),'Fresh planning result required');result=dict(schema=1,kind='broader_copy_scheduling_phase_preparation',success=False,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0)
 try:
  with Reader(ROOT)as r:
   ref=lambda n:dict(record=n,sha256=r.digest(n));draft=Proposed(r)
   with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
    db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns,case_id')]
   require(len(rows)==69 and sum(v['reserved_games']for v in rows)==101,'Stopped request set changed');byid={v['case_id']:v for v in rows};old=previous_specs(r);retained=[s for s in old if s['case']['id']in byid and byid[s['case']['id']]['state']=='admitted'];remaining=[s for s in old if s not in retained];require(len(retained)==35 and len(remaining)==55 and all(s['role']in('world','menu','component')for s in remaining),'Changed positive cohort')
   failed='bp24-world-hotspot-default800-01';require(byid[failed]['state']=='acquired'and byid[failed]['reserved_games']==1,'Missing acquired hotspot worker-reservation negative');require(all(s['case']['id']not in byid or s['case']['id']==failed for s in remaining),'Another requested context needs review');ids={s['case']['id']:s['case']['id'].replace('bp24-','bp25-',1)for s in remaining};require(len(set(ids.values()))==55,'Reused case identity')
   build=ref(P+'build-current-05.json');b=r.json(build['record']);require(all(r.digest(n)==h for n,h in(b['compiler_sources']|b['runtime_sources']).items()),'Native/compiler/runtime changed');binding=ref(P+'player-build-only-binding-06.json');decl=r.json(binding['record']);require(decl['full_build_sources']==b['compiler_sources']and decl['player_build']['compiler_sources']|decl['configuration_sources']==b['compiler_sources'],'Build configuration omitted')
   controls=[ref(P+n+'.json')for n in('native-controls-06','isolated-tooling-controls-13','drain-admission-controls-02','binding-continuation-controls-01','input-reference-controls-01','history-continuation-controls-01','namespace-continuation-controls-01','arguments-continuation-controls-01','visible-accessibility-controls-06','visible-continuation-controls-01','isolated-tooling-controls-15','font-visible-accessibility-controls-07','font-observer-continuation-controls-01','final-negative-reader-controls-01','hotspot-visible-accessibility-controls-02','hotspot-observer-continuation-controls-01','copy-queue-scheduling-native-controls-03','scheduled-hotspot-continuation-controls-01')]
   for cr in controls:
    q=r.json(cr['record']);require(q['success']and q.get('sources_stable',True)and all(r.digest(n)==h for n,h in q.get('sources',{}).items()),'Current control changed: '+cr['record'])
   app=ref(P+'completed-journal-publication-applicability-01.json');reviewers=['tools/prepare-broader-scheduled-hotspot-phase.py','tools/broader_scheduled_source_budget.py','tools/run-broader-scheduled-source-case.py','tools/run-broader-scheduled-domain-case.py','tools/run-broader-scheduled-selected-domain-case.py','tools/broader_player_scheduled_matrix.py','tools/broader_hotspot_visible_launch.py','tools/broader_copy_diagnostic_scheduler.py','tools/broader_hotspot_scheduled_launch.py','tools/run-broader-hotspot-scheduled-world.py','tools/probe-broader-copy-queue.cpp','tests/test-broader-copy-scheduler.py','tests/test-broader-scheduled-budget.py','tests/test-broader-hotspot-visible-launch.py','tests/test-broader-scheduled-source-scope.py']
   review=dict(schema=1,kind='broader_copy_scheduling_continuation_review',success=True,original_game_runs=0,old_reserved_starts=101,retained_current_cases=35,remaining_positive_cases=55,remaining_positive_original_starts=55,new_additional_ceiling=156,new_combined_ceiling=164,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,manually_reviewed_replacement_starts=9,changed_compiler_files=[],changed_runtime_files=[],game_response_deadlines_changed=False,reviewer_sources={n:r.digest(n)for n in reviewers},current_build=build,corrected_build_binding=binding,current_controls=controls,old_requests=rows,new_case_ids=sorted(ids.values()),publication_applicability=app,failed_case_custody=ref(P+failed+'-capacity-custody-01.json'),hotspot_failure_custody=ref(P+failed+'-capacity-custody-01.json'),hotspot_actions_or_caps_changed=False,parent_global_plan=ref(P+'plan-24.json'),first_pilot=ids[failed],reason='The phase24faster-observer hotspot800NVIDIA pilot exitsnormallywithmatchingfullviewportpixels,but136requiredactualCOPYoutputsaremissingamong4982commands/4328diagnosticsbelowthe8192cap. Preserveall69priorrows101charges,bothhotspotnegativesand35completecontexts. Retainedcommandmetadataandfixedsourceconditionsinferboundedworkerreservationloss,notNativepixelcorruption. The exactchild-onlyhostadapterpasses12scope/refusaltestsandNative-only600COPY800x600pressureonbothactualNVIDIAandsoftware,withall1200COPYdestinations,capturesandretirementcompleteunderunchangedcaps. PinonlytheexactNativechildownerandinheritedstandardworkerstooneallowedCPUandlowermainthreadnice19aftertheexistingconstructor marker;allcontrolleractions/waits,response deadlines,Native/compiler/runtimebytesandqueue/cumulativecapsremainunchanged. Newphase25declares55remainingpositivesunder156additional/164combinedwithWP10eight,onenewmanuallyreviewedreplacement,zerospare/refund/automaticretry. Requirecompletehotspot800NVIDIApilotandpairedretentionbefore54othercontexts;retainexistinghotspot6game/9requestcountersandancestry. Originalrecoveryanduniversalperformance/schedulingclaimsremainpending.')
   rr=draft.add(P+'scheduled-source-review-01.json',review);g=copy.deepcopy(r.json(P+'plan-24.json'));g['previous_hotspot_observation_review']=g.pop('hotspot_observation_review');g.update(kind='broader_current_player_copy_scheduling_plan',previous_plan=review['parent_global_plan'],copy_scheduling_review=rr,current_build=build,corrected_build_binding=binding,maximum_additional_original_games=156,maximum_combined_original_games=164,maximum_combined_WP10_and_new_original_games=164,minimum_final_additional_reservations=156,retained_predecessor_reservations=101,retained_current_cases=35,remaining_positive_cases=55,remaining_positive_original_starts=55,maximum_manually_reviewed_additional_repairs=9,request_driver='tools/run-broader-scheduled-source-case.py',scope=review['reason']);gr=draft.add(P+'plan-25.json',g);require(bound(draft,g)==156 and predecessors(draft,rows,g)==101,'Exact finite guard refused')
   oldnames=sorted({s['plan']for s in remaining});names={n:n.removesuffix('.json')+'-hotspot-scheduled-25.json'for n in oldnames}
   def adapt(v):
    if isinstance(v,dict):return {k:adapt(x)for k,x in v.items()}
    if isinstance(v,list):return [adapt(x)for x in v]
    if isinstance(v,str):
     if v in names:return names[v]
     for oldid,newid in ids.items():v=v.replace(oldid,newid)
    return v
   bindings=[dict(case_id=s['case']['id'],plan=ref(s['plan']),role=s['role'],family=s['family'],retained=True)for s in retained];phases=[];source_names=set()
   for oldname in oldnames:
    orig=r.json(oldname);phase=adapt(orig);wanted={s['case']['id']for s in remaining if s['plan']==oldname};phase['broader_cases']=[adapt(c)for c in orig['broader_cases']if c['id']in wanted];phase.update(broader_plan=gr,request_driver='tools/run-broader-scheduled-source-case.py',supersedes_prospective_plan=ref(oldname),corrected_build_binding=binding)
    if orig['family']not in('menu','intros','skip','clipping','stretch'):phase['command_history_policy']=dict(flag='MNM_ACCEPTANCE_COMMAND_HISTORY',value='1',version=4,policy='command-history-lossless-v1',row_limit=65536,raw_limit_bytes=8*1024**3,stored_limit_bytes=1024**3,queue_limit_bytes=256*1024**2,queue_limit_rows=4096,compression_workers=8,native_limits_changed=False)
    if 'current_player_build_binding'in phase:phase['current_player_build_binding']=binding
    seeds=set(orig['sources'])|set(reviewers)|{'apps/qt-shell/CMakeLists.txt'};allnames=seeds|python_sources(ROOT,[n for n in seeds if n.endswith('.py')])
    phase['counter_bindings']=dict(orig['counter_bindings'])
    from broader_delegated_plan import bind_arguments,capture_backing
    phase['capture_storage']=capture_backing(orig['family'],orig.get('capture_storage',{}))
    for case in phase['broader_cases']:
     if 'controller_arguments'in case:
      case['controller_arguments'],argument_binding=bind_arguments(case['controller_arguments'],oldname,names[oldname],orig['family'])
      if argument_binding is not None:case['delegated_plan_identity']=argument_binding
    if orig['family']=='hotspot':
     scheduling=r.json(P+'copy-queue-scheduling-native-controls-03.json')
     require(scheduling['success']and scheduling['parent_scheduling_unchanged'],'Current actual scheduling control missing')
     phase['hotspot_copy_scheduling']=copy.deepcopy(scheduling['policy'])
     for case in phase['broader_cases']:
      require(case['command'][1]=='tools/run-broader-hotspot-visible-world.py','Unexpected hotspot collector');case['command'][1]='tools/run-broader-hotspot-scheduled-world.py'
      if case['id']==ids[failed]:
       args=case['controller_arguments'];require(args.count('--repair-of')==args.count('--repair-reason')==1,'Missing exact preceding repair ancestry')
       args[args.index('--repair-of')+1]=byid[failed]['report'];args[args.index('--repair-reason')+1]='Source-bound child scheduler after retained diagnostic worker reservation gaps; same actions,Nativebytes,capsanddeadlines'
     phase['hotspot_scheduling_repair_of']=ref(byid[failed]['report'])
     phase['positive_pilot_policy']=dict(first_case=ids[failed],first_validation=P+ids[failed]+'-validation-01.json',all_other_cases_need_positive_pilot=True,external_qualification_required=True,request_accounting_stops_on_incomplete=True)
    phase['copy_scheduling_review']=rr
    phase['sources']={n:r.digest(n)for n in allnames};require('apps/qt-shell/CMakeLists.txt'in phase['sources'],'Configuration source missing from actual prelaunch gate');source_names.update(allnames)
    pr=draft.add(names[oldname],phase);phases.append(pr)
    for s in remaining:
     if s['plan']==oldname:
      c=next(v for v in phase['broader_cases']if v['id']==ids[s['case']['id']]);require(all(c[k]==s['case'][k]for k in('profile','provider','size','reserved_original_games'))and c['reserved_original_games']==1,'Changed finite context');bindings.append(dict(previous_case_id=s['case']['id'],case_id=c['id'],plan=pr,role=s['role'],family=s['family'],retained=False))
   analysis={}
   for stem in('native-edge','world-native-edge','boundary-native-edge'):
    n=P+stem+'-analysis-plan-24.json';v=copy.deepcopy(r.json(n));require(all(r.digest(n)==h for n,h in v['sources'].items()),'Oracle source changed');v.update(global_plan=gr,previous_analysis_plan=ref(n));analysis[stem]=draft.add(P+stem+'-analysis-plan-25.json',v)
   catalog=dict(previous_catalog=ref(P+'hotspot-source-catalog-01.json'),schema=1,kind='broader_copy_scheduling_source_catalog',success=True,original_game_runs=0,global_plan=gr,review=rr,publication_applicability=app,current_build=build,corrected_build_binding=binding,retained_current_cases=35,remaining_positive_cases=55,total_positive_cases=90,total_positive_original_starts=98,native_source_waivers=[],case_bindings=bindings,analysis_plans=analysis,prospective_phases=phases,first_pilot=ids[failed],global_player_accepted=False);cn=P+'scheduled-source-catalog-01.json';draft.add(cn,catalog)
   planned=specs(draft);require(len(planned)==90,'Positive denominator changed')
   for s in planned:
    cid=s['case']['id'];phase=draft.json(s['plan'])
    if cid in byid and byid[cid]['state']=='admitted':current(ROOT,draft,s,byid[cid]);continue
    require(phase['broader_plan']==gr and phase['request_driver']=='tools/run-broader-scheduled-source-case.py'and all(draft.digest(n)==h for n,h in phase['sources'].items()),'Prospective source/driver mismatch');require(s['plan']in s['case']['command']and s['case']['report']in s['case']['command'],'Wrong collector plan/report')
    args=s['case'].get('controller_arguments',[])
    if '--plan'in args:require(args.count('--plan')==1 and args[args.index('--plan')+1]==s['plan'],'Delegated collector would read another plan')
    if s['family']in('caller','picking'):require(phase['capture_storage']['enabled']is False and phase['capture_storage']['paired_real_restore_required'],'Controller RAM ownership missing')
    if s['family']=='attachment':
     d=draft.json(phase['current_player_build_binding']['record']);require(set(d['player_build']['compiler_sources'])==set(decl['player_build']['compiler_sources'])and d['player_build']['binary']==decl['player_build']['binary'],'Actual attachment build declaration differs')
   def refs(v):
    if isinstance(v,dict):
     if 'record'in v and 'sha256'in v:require(draft.digest(v['record'])==v['sha256'],'Child reference changed: '+v['record'])
     for x in v.values():refs(x)
    elif isinstance(v,list):
     for x in v:refs(x)
   for pr in phases:refs(draft.json(pr['record']))
   require(not(ROOT/cn).exists()and not(ROOT/P/'plan-25.json').exists(),'Binding phase already exists');source_bytes={n:(ROOT/n).read_bytes()for n in source_names}
  with configured(ROOT)as pair:
   with pair.batch():objects={n:dict(sha256=pair.bytes(body),size=len(body))for n,body in source_bytes.items()}
   sr=pair.root('broader_copy_scheduling_phase_sources',ROOT,dict(schema=1,sources=objects))
  catalog['source_preimages']=sr;draft.reports[cn]=(json.dumps(catalog,indent=2)+'\n').encode()
  for n,body in draft.reports.items():require(write_report(ROOT,n,body)==hashlib.sha256(body).hexdigest(),'Report changed')
  result.update(success=True,catalog=dict(record=cn,sha256=hashlib.sha256(draft.reports[cn]).hexdigest()),global_plan=gr,review=rr,retained_current_cases=35,remaining_positive_cases=55,first_pilot=ids[failed],source_preimages=sr,all_child_refs_and_sources_verified=True)
 except Exception as error:
  import traceback
  traceback.print_exc();result['error']=repr(error)
 op=rn.removesuffix('.json')+'-operator.bin';result['operator']=dict(record=op,sha256=write_report(ROOT,op,Path(__file__).read_bytes()));h=write_report(ROOT,rn,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))),flush=True);return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
