#!/usr/bin/env python3
"""Conjoin the complete current finite player matrix without retiring old phases."""
import argparse,hashlib,json
from pathlib import Path
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from coverage_evidence import Reader,write_report
from broader_player_acceptance import checked,parents,BID,SID
from broader_player_capture_matrix import specs,requests,current,replacements,P,BINARY
from broader_player_join import join_rows,expected_cells
from broader_player_sources import python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def assess(r,boundary_name,failure_name):
 ref=lambda n:dict(record=n,sha256=r.digest(n));declared=specs(r);rows,budget=requests(ROOT,r);global_plan=r.json(P+'plan-11.json');old=checked(r,global_plan['source_continuation_review']);historical={v['case_id']:v for v in old['requests']};wanted={v['case']['id']for v in declared};setup=set(global_plan['preserved_setup_refusals'])
 require(set(rows)==wanted|set(historical)|setup|{r.json(global_plan['guard_stopped_review']['record'])['failed_case_id'],r.json(global_plan['capture_refusal_review']['record'])['failed_case_id']},'Unknown, missing or extra original start row')
 for key,record in historical.items():require(rows[key]==record,'Historical source-phase request was rewritten')
 require(len(declared)==89 and sum(v['case']['reserved_original_games']for v in declared)==97,'Changed current actual-start denominator')
 values=[current(ROOT,r,s,rows[s['case']['id']])for s in declared]
 require(len({v['acquisition']['record']for v in values})==89,'Shared acquisition falsely credited as distinct execution')
 inventory=checked(r,ref(P+'inventory-04.json'));require(len(inventory['cases'])==94,'Historical primary inventory shrank');mapping=replacements(r,inventory,values)
 reg=r.json('research/runtime/coverage/register.json');accepted=parents(r,reg);closure=r.json('docs/native-rendering-closure.json');journey_keys,operation_keys=expected_cells(closure)
 overlap=checked(r,ref(P+'overlap-controls-01.json'));hardware_overlap=checked(r,ref(P+'hardware-overlap-controls-01.json'))
 require(overlap['success']and overlap['sources_stable']and hardware_overlap['success']and hardware_overlap['sources_stable']and hardware_overlap['actual_hardware']['verified']and hardware_overlap['approved_scope_amendment_unchanged'],'Missing current controlled-overlap basis')
 require(all(r.digest(n)==h for n,h in(overlap['sources']|hardware_overlap['sources']).items()),'Controlled-overlap native/compiler source drift')
 controlled=dict(verified=True,approved_amendment='ALIAS09.controlled-opaque-overlap-v1',retained_original_and_Mesa_proof_unchanged=True,current_software_control=ref(P+'overlap-controls-01.json'),current_NVIDIA_control=ref(P+'hardware-overlap-controls-01.json'),natural_player_call_proved=False)
 joined={}
 for eid,expected,journey in[('JG10.journeys02',journey_keys,True),('OG09.merged-operations01',operation_keys,False)]:
  parent=accepted[eid];joined[eid]=join_rows(parent['proof']['linked_requirements'],expected,mapping,parent['admission'].get('proof',parent['reference']),journey=journey,controlled_overlap=controlled)
 require(len(joined['JG10.journeys02'])==94 and len(joined['OG09.merged-operations01'])==42,'Current conjunction lost a leaf')
 amendment=accepted['OG09.merged-operations01']['proof']['scope_amendments'];require(len(amendment)==1 and amendment[0]['id']=='ALIAS09.controlled-opaque-overlap-v1','Approved amendment changed')
 boundary=checked(r,ref(boundary_name));failure=checked(r,ref(failure_name));require(boundary['technical_conditions_waived']is False and boundary['parent_requirement_rows_preserved']==156 and boundary['source_phase_waivers_for_native_files']is False and len(boundary['required_boundary_review'])==8,'Missing actual-provider required-boundary review')
 require(failure['historical_failures_retired']is False and failure['universal_repair_claimed']is False and failure['B05']['current_same_contract_10_and_30_second_stress_verified']and failure['RN31']['current_finite_coupling_verified']and failure['B06']['fixed_FPS_acceptance_gate']is False,'Historical failure relevance missing or weakened')
 strict=[v for v in values if v['spec']['role']=='strict'];require(all(v['value']['actual_context']['verified']and v['value']['actual_context']['actual_native_frame_size']==[800,600]for v in strict),'Missing actual strict native extent');require({(v['value']['profile'],v['value']['stress_seconds'])for v in strict}=={(p,s)for p in('player-default','player-software')for s in(10,30)},'Missing exact strict stress context')
 faults=[v for v in values if v['spec']['role']=='fault'];require(len(faults)==8 and {(v['spec']['family'],v['value']['profile'],tuple(v['value']['size']))for v in faults}=={(f,p,s)for f in('loss','transition')for p in('player-default','player-software')for s in((640,480),(800,600))},'Declared failed/fresh domain shrank')
 sources={}
 def extend(values):
  for name,sha in values.items():require(name not in sources or sources[name]==sha,'Conflicting current source phases: '+name);sources[name]=sha
 for v in values:extend(v['value']['sources'])
 for v in(boundary,failure,overlap,hardware_overlap):extend(v['sources'])
 b=next(v for v in reg['behaviors']if v['id']==BID);scenario=next(v for v in reg['scenarios']if v['id']==SID)
 names=python_sources(ROOT,['tools/check-broader-player-acceptance-capture.py'])|required_sources(b)|set(scenario['tests'])
 extend({n:r.digest(n)for n in sorted(names)});require(all(r.digest(n)==h for n,h in sources.items()),'Current combined proof source drift')
 require(hashlib.sha256((ROOT/'working/build/qt-shell/mnm-qt-shell').read_bytes()).hexdigest()==BINARY,'Current linked executable changed')
 world_values=[v for v in values if v['spec']['role']in('world','boundary')];world_pixels=sum(v['value']['output_history']['output']['pixels']for v in world_values);fault_pixels=sum(v['value']['independent_pixels']['pixels_compared']for v in faults)
 movie_values=[v for v in values if v['spec']['family']=='intros'];require(len(movie_values)==4,'Enabled-movie domain missing')
 cases=[dict(id=v['spec']['case']['id'],role=v['spec']['role'],family=v['spec']['family'],profile=v['spec']['case']['profile'],size=v['spec']['case']['size'],acquisition=v['acquisition'],validation=v['validation'],actual_drivers=v['drivers'],exact_current_native_source_candidate=v['phase']['current_native_source_candidate'])for v in values]
 source_facts=[dict(historical_primary=item['acquisition'],historical_roles=item['roles'],current_validation=mapping[(item['acquisition']['record'],item['acquisition']['sha256'])]['validation'],semantic_binding=mapping[(item['acquisition']['record'],item['acquisition']['sha256'])]['semantic_binding'],historical_execution_sources_promoted=False)for item in inventory['cases']]
 return dict(schema=1,kind='broader_current_public_player_acceptance',success=True,current_whole_player_proved=True,supported_player_accepted=True,original_recovery_complete=False,historical_failures_retired=False,physical_driver_reset_proved=False,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_source_original_game_starts=97,current_cases=len(cases),player_binary_sha256=BINARY,current_native_sources_verified_without_waiver=True,journeys=14,journey_leaves_per_profile=47,journey_cells=94,operations_per_profile=21,operation_cells=42,linked_journey_requirements=joined['JG10.journeys02'],linked_operation_requirements=joined['OG09.merged-operations01'],parent_admissions={eid:v['reference']for eid,v in accepted.items()},all_historical_primary_attachments=source_facts,scope_amendments=amendment,scope_amendments_sha256=fingerprint(amendment),controlled_overlap=controlled,natural_player_overlap_domains=0,declared_failed_fresh_cases=8,failed_epoch_normal_credit=False,strict_10_30_second_cases=4,NVIDIA_fullscreen_supplements=1,current_software_fullscreen_reused=True,complete_World_history_cases=57,selected_plane_only_cases=8,semantic_menu_media_cases=12,independently_compared_World_and_fault_pixels=world_pixels+fault_pixels,enabled_movie_validations=[v['validation']for v in movie_values],native_edge_policy_cases=[dict(validation=v['validation'],policy=v['value']['native_edge_policy'])for v in values if v['value'].get('native_edge_policy')],all_required_output_pixels_compared=True,ignored_pixels=0,original_padding_equivalence=False,actual_cases=cases,cumulative_start_accounting=budget,required_boundary_impact=ref(boundary_name),historical_failure_relevance=ref(failure_name),supported_input_domain=ref('docs/native-rendering-support-domain.json'),fixed_FPS_threshold=None,original_supplier_boundaries_preserved=True,arbitrary_maps_saves_durations_supported=False,sources=sources,sources_stable=True,claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(scenario)})],scope='Current finite native-player acceptance across all14authored journeys/47leaves and21operation requirements per profile, all declared failed/fresh domains, strict10/30second readiness and actual NVIDIA/software presentation. Every historical row/attachment, exact original supplier/input/map/save/duration boundary and approved controlled-overlap scope remains explicit. Full required presented/history pixels, original callbacks/state, deadlines and resource retirement are conjoined at the exact native/runtime/compiler/player source identities. Native finite-edge retention is separately certified with every pixel compared and raw-original negatives preserved; original padding equivalence is not granted. Historical failures and causal limits remain separate. No arbitrary provider, original recovery, natural alias reachability, physical Windows/driver/reset or unbounded reliability/performance claim.')
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('report','boundary-review','failure-review'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh combined current acceptance record required')
 with Reader(ROOT)as r:
  try:q=assess(r,args.boundary_review,args.failure_review)
  except(ValueError,KeyError,OSError,TypeError,StopIteration)as error:q=dict(schema=1,kind='broader_current_public_player_acceptance',success=False,current_whole_player_proved=False,supported_player_accepted=False,original_recovery_complete=False,original_game_runs=0,error=str(error))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'),current_whole_player_proved=q['current_whole_player_proved'],supported_player_accepted=q['supported_player_accepted'],original_recovery_complete=False)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
