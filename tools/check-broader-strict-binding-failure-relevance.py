#!/usr/bin/env python3
"""Conjoin current finite followups while preserving every historical refusal."""
import argparse,hashlib,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_acceptance import checked,native_phase
from broader_player_strict_binding_final_matrix import P,BINARY,specs,requests,current
from broader_history_capture_contract import capture_selection
from broader_failure_evidence import checked_negative
from broader_player_sources import python_sources
from rendering_pilot import require
from broader_strict_binding_final_gate import prior_context
ROOT=Path(__file__).resolve().parents[1]
def module(name):
 p=ROOT/'tools'/(name+'.py');s=importlib.util.spec_from_file_location('B06_failure_'+name.replace('-','_'),p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def original_history(reader,review,actual):
 """A current same-context proof never rewrites the old acquisition or cause."""
 by_context={}
 for v in actual.values():
  s=v['spec'];c=s['case'];key=(s['family'],c['profile'],tuple(c['size']));by_context.setdefault(key,[]).append(v)
 records=[]
 for row in review['old_requests']:
  pr=json.loads(row['plan']);require(reader.digest(pr['record'])==pr['sha256'],'Historical request plan changed');phase=reader.json(pr['record']);case=next(c for c in phase['broader_cases']if c['id']==row['case_id']);family=phase.get('family')
  if family is None:
   found=[v for v in('loss','transition','strict','boundary')if '-'+v+'-'in row['case_id']];require(len(found)==1,'Unclassified historical finite family');family=found[0]
  key=(family,case['profile'],tuple(case['size']));require(key in by_context,'Historical request lacks a current declared-context followup');followups=by_context[key]
  if family=='strict':followups=[v for v in followups if v['spec']['case']['stress_seconds']==case['stress_seconds']]
  require(followups,'Historical strict duration lost');ref=None;a=None
  if(ROOT/row['report']).exists():
   ref=dict(record=row['report'],sha256=reader.digest(row['report']));require(ref['sha256']==row['acquisition_sha256'],'Historical acquisition payload changed');a=reader.json(row['report'])
  else:require(row['state']=='failed'and row['acquisition_sha256']is None,'Missing acquired/admitted historical report')
  custody=[]
  extra={'bp10-transition-default800-01':['negative-transition800-handoff-01.json'],'bp11-loss-default800-01':['zero-game-setup-review-01.json','bp11-loss-default800-01-reservation-review-01.json','bp11-loss-default800-01-coordinator-02.json']}
  paths=set((ROOT/P).glob(row['case_id']+'*custody*.json'))|{ROOT/P/n for n in extra.get(row['case_id'],[])}
  for path in sorted(paths):
   name=path.relative_to(ROOT).as_posix();value=reader.json(name)
   custody.append(dict(reference=dict(record=name,sha256=reader.digest(name)),success=value.get('success'),case_accepted=value.get('case_accepted'),historical_only=True))
  if row['state']!='admitted':require(custody,'Historical failed/unadmitted request has no retained custody: '+row['case_id'])
  records.append(dict(case=row['case_id'],state=row['state'],reservation=row['reserved_games'],plan=pr,acquisition=ref,recorded_original_game_runs=a.get('original_game_runs')if a else None,recorded_success=a.get('success')if a else None,recorded_error=a.get('error')if a else'Collector refused before creating acquisition',recorded_actual_size=(a.get('configuration',{}).get('actual_size')if a else None),custody=custody,current_same_declared_context=[v['validation']for v in followups],old_source_promoted=False,old_report_rewritten=False,reservation_refunded=False,historical_cause_newly_inferred=False))
 require(len(records)==71 and sum(v['reservation']for v in records)==103,'Historical 71/103 snapshot changed');return records

def hotspot(reader,ref,actual):
 negatives=[ref(P+n)for n in ['bp23-world-hotspot-default800-01-capacity-custody-01.json','bp24-world-hotspot-default800-01-capacity-custody-01.json','bp25-world-hotspot-default800-01-accessibility-custody-01.json','bp26-world-hotspot-default800-01-prefix-custody-01.json']]
 ns=[checked(reader,n)for n in negatives];require(all(v['case_accepted']is False and v['reservations_refunded']is False for v in ns),'Historical COPY/startup negatives changed');require([v['diagnosis']['missing_required_actual_COPY_observations']for v in ns[:2]]==[170,136],'Historical required COPY gaps changed');branch=ref(P+'bp23-world-hotspot-default800-01-queue-branch-analysis-02.json');b=checked(reader,branch);require(b['record_cap_excluded_for_every_required_gap']and b['readback_cap_excluded_for_every_required_gap']and len(b['missing_required'])==170 and not b['branch_counter_or_stack_directly_captured'],'Historical branch inference changed')
 values=[]
 for v in actual.values():
  if v['spec']['family']!='hotspot':continue
  c=checked(reader,v['value']['comparison']);p=c['proof'];h=p['copy_history'];d=p['broad_copy_diagnostic'];lo,hi=h['required_copy_interval'];require(p['verified']and h['verified']and h['required_compared_copies']>0 and h['complete_original_cursor_calls']>0 and not h['diagnostic_pixels_used_as_reference_inputs']and p['cleanup']['verified'],'Current COPY/cursor/lifetime proof missing');require(all(not(lo<=x['sequence']<=hi)for x in h['unobserved_outside_cursor_interval']),'Required missing COPY relabeled outside interval');require(d['worker_queue_limit_bytes']==512*1024**2 and d['worker_peak_bytes']<=512*1024**2 and d['preparation_workers']==8 and d['worker_admission_wait_limit_ms']==50,'Current COPY worker policy differs');a=v['acquired'];require('copy_diagnostic_scheduling'not in a['provider_adaptation'],'Abandoned host scheduling policy reached current execution');values.append(dict(validation=v['validation'],acquisition=v['acquisition'],comparison=v['value']['comparison'],required_COPYs=h['required_compared_copies'],required_interval=[lo,hi],unobserved_outside_required_interval=h['unobserved_outside_cursor_interval'],diagnostic=d))
 require(len(values)==4,'Current hotspot denominator changed');control=ref(P+'COPY-component-controls-03.json');q=checked(reader,control);require(q['total_required_COPYs']==1200 and q['physical_GUI_responses_during_COPYs']and q['optimized_and_ASAN_UBSAN_CPU_controls']and not q['host_scheduling_modified'],'Current Native COPY controls missing');return dict(historical_custody=negatives,historical_branch_analysis=branch,historical_branch_inferred_not_directly_recorded=True,historical_scheduled_startup_cause='Exact stalled callback/stack unrecorded',zero_game_prefix_refusal_retained=True,current_case_validations=values,positive_replacement_verified=True,scheduled_current_cases=0,host_priority_or_affinity_modified=False,Native_source_change='B06 bounded parallel CPU preparation,orderedwrites,fullwordverifiedhashsharingandboundedadmission',controls=control,diagnostic_worker_queue_limit_bytes=512*1024**2,copy_limit=8192,readback_limit_bytes=32*1024**3,historical_failure_retired=False,universal_repair_claimed=False)

def font(reader,ref,actual):
 from font_lifetime_observation import decode
 custody=ref(P+'bp22-world-font-default640-01-capacity-custody-01.json');n=checked(reader,custody);require(n['diagnosis']['data_rows']==131072 and n['diagnosis']['terminal_loss']==2 and n['diagnosis']['terminal_current_owners']==4 and not n['case_accepted']and not n['reservations_refunded'],'Historical font loss changed');preview=ref(P+'font-visible-accessibility-controls-08.json');p=checked(reader,preview);require(p['font_journey_freshness_wrappers_applied_to_both']and len(p['results'])==7 and all(v['identical']for v in p['results'])and p['player_binary_sha256']==BINARY,'Current font observer layout control missing');values=[]
 for v in actual.values():
  if v['spec']['family']!='font':continue
  a=v['acquired'];c=checked(reader,v['value']['comparison']);require(c['font_lifetimes_verified']and c['native_menu_fonts']['verified']and c['font_dc_lifetime']['dc']['complete_observed_chain']and c['font_dc_lifetime']['normal_native_retirement'],'Current complete font/DC closure missing');matches=[x for x in a['artifacts']if x['path'].endswith('/font-lifetime-0001.bin')];require(len(matches)==1,'Font trace ambiguity');t=matches[0];require(reader.digest(t['record'])==t['sha256'],'Font trace changed');decoded=decode(reader.read_bytes(t['record']));require(decoded['complete']and len(decoded['rows'])<=131072 and decoded['terminal'][7]==decoded['terminal'][10]==decoded['terminal'][11]==0,'Current font recorder loss or owners remain');values.append(dict(validation=v['validation'],acquisition=v['acquisition'],records=len(decoded['rows']),terminal_loss=0,current_owners=0,pending_calls=0))
 require(len(values)==4,'Current font denominator changed');return dict(custody=custody,preview_equality=preview,current_case_validations=values,positive_replacement_verified=True,diagnostic_row_limit=131072,historical_failure_retired=False,all_old_charges_retained=True)

def assess(r):
 ref=lambda n:dict(record=n,sha256=r.digest(n));decl=specs(r);rows,budget=requests(ROOT,r);require(len(rows)==196 and budget['reserved_additional_original_starts']==245,'Complete finite 196row/245charge cohort required');actual={s['case']['id']:current(ROOT,r,s,rows[s['case']['id']])for s in decl};plan=r.json(P+'plan-42.json');review=checked(r,plan['COPY_source_review']);history=original_history(r,review,actual);pre=checked(r,ref(P+'historical-relevance-preflight-01.json'));startup=module('check-broader-scheduled-failure-relevance').startup;strict=[];quick=[]
 for profile in('default','software'):
  for seconds in(10,30):
   v=prior_context(r,actual,f'bp29-strict-{profile}{seconds}-01');p=v['value'];require(p['functional_acceptance']['verified']and p['stress_seconds']==seconds and p['response_progress']['verified']and p['actual_context']['verified'],'Current strict domain incomplete');strict.append(dict(validation=v['validation'],functional_acceptance=p['functional_acceptance'],response_progress=p['response_progress']))
  for width in(640,800):
   v=prior_context(r,actual,f'bp29-world-quick-{profile}{width}-01');c=checked(r,v['value']['comparison']);quick.append(dict(validation=v['validation'],profile=v['value']['profile'],size=v['value']['actual_initial_size'],proof=startup(r,v['acquired'],c,v['value'])))
 faults=[v for v in actual.values()if v['spec']['role']=='fault'];require(len(faults)==8 and all(v['value']['expected_failed_epoch_normal_credit']is False and len(v['value']['epoch_progress'])==2 and v['value']['independent_pixels']['mismatching_pixels']==0 for v in faults),'Current failed/fresh proof weakened');controls=ref(P+'current-component-controls-06.json');cp=checked(r,controls);require(cp['parallel_optimized_and_sanitized_verified']and cp['gui']['deferred_unpresented_and_borrowed_requests']==2 and all(r.digest(n)==h for n,h in cp['sources'].items()),'Current recorder/GUI controls stale');callers=[]
 for v in actual.values():
  if v['spec']['family']!='caller':continue
  c=checked(r,v['value']['comparison']);from broader_strict_binding_final_gate import caller_binding;binding=caller_binding(r,v);callers.append(dict(validation=v['validation'],schema=binding))
 require(len(callers)==4,'Caller denominator changed');selected=[v for v in actual.values()if v['spec']['role']in('menu','component')];require(len(selected)==20,'Menu/component denominator changed');oldmenu=(ROOT/'tools/check-broader-relative-menu.py').read_bytes();newmenu=(ROOT/'tools/check-broader-copy-menu.py').read_bytes();require(newmenu==oldmenu.replace(b'build-current-05.json',b'build-current-06.json').replace(b'check-broader-relative-menu.py',b'check-broader-copy-menu.py'),'New menu identity reader changed other assertions');historic_B05=[]
 for name in ['research/runtime/rendering-seventeenth-five-results-20261023/strict-campaign-01-journey.json','research/runtime/rendering-seventeenth-five-results-20261023/strict-campaign-02-journey.json']:
  old=checked_negative(r,ref(name));historic_B05.append(dict(reference=ref(name),recorded_error=old.get('error'),source_hashes_preserved=True))
 WP10=ref('research/runtime/whole-player-validation-results-20261110/historical-impact-01.json');impact=checked(r,WP10);require(impact['historical_failures_retired']is False and impact['historical_root_cause_proved']is False,'Historical RN31cause orretirement changed');hp=hotspot(r,ref,actual);fp=font(r,ref,actual);sources={n:r.digest(n)for n in python_sources(ROOT,['tools/check-broader-strict-binding-failure-relevance.py','tools/check-broader-scheduled-failure-relevance.py'])}
 for v in actual.values():sources.update(v['current_sources']|v['coordinator_sources'])
 sources.update(cp['sources']);require(all(r.digest(n)==h for n,h in sources.items()),'Current failure relevance source drift');result=dict(schema=1,kind='broader_current_historical_failure_relevance',success=True,original_game_runs=0,preflight=ref(P+'historical-relevance-preflight-01.json'),historical_startup=pre['historical_startup'],historical_requests=history,historical_B05_failures=historic_B05,historical_WP10_impact=WP10,old_request_rows_preserved_exactly=71,old_reservations_preserved_exactly=103,current_recorder_and_GUI_fault_relevance=dict(current_cases=[v['validation']for v in faults],current_controls=controls,failed_epoch_normal_credit=False,historical_failures_retired=False),current_viewport_observation_failure_relevance=dict(current_cases=[v['validation']for v in actual.values()if v['spec']['role']=='boundary'],historical_live_failure_branch_proved=False,old_incomplete_capture_completed=False),current_caller_recorder_capacity_failure_relevance=dict(current_case_validations=[v['validation']for v in actual.values()if v['spec']['family']=='caller'],positive_replacement_verified=True,historical_failure_retired=False),current_nominal_analysis_failure_relevance=dict(current_caller_analyses=callers,additional_original_games=0,additional_reservations=0,historical_failure_retired=False),current_selected_preferences_identity_review=dict(current_case_validations=[v['validation']for v in selected],all_other_assertions_byte_identical=True,new_menu_build_identity_bound_to_B06=True,component_reader_unchanged=True),current_font_recorder_capacity_failure_relevance=fp,current_hotspot_COPY_observation_failure_relevance=hp,current_B05_strict=strict,current_full_intro_first_World=quick,B05=dict(current_same_contract_10_and_30_second_stress_verified=True,historical_cause='unclassified',historical_negative_retired=False),B06=dict(instrumented_historical_FPS=17.253776873910518,historical_19FPS_accepted=False,measurement_overhead='unresolved',fixed_FPS_acceptance_gate=False,historical_failure_retired=False),RN31=dict(current_finite_coupling_verified=True,historical_source_phase_preserved=True,historical_failure_retired=False,historical_cause_proved=False,current_failure_observed=False,current_relevance='Fresh full-intro/Quick coupling,continuousinitialhistory,everyrequiredpixel,source/menu/input progress andnormalretirement onbothactualproviders/extents. Historicalwatchdogcause anduniversalreliability remainunproved.'),current_scope_supported_only_by_fresh_positive_execution=True,historical_failures_retired=False,universal_repair_claimed=False,sources=sources,sources_stable=True,scope='Every old71request103charge remains exact,including acquired-but-unadmitted andzero-game refusals,withoriginalhashes andretainedcustody. All90B06contexts haveindependent currentproofs; matchingfinite followups establish currentnonreproduction only. They do not complete oldcaptures,refreshNativehashes,diagnoseuncapturedstacks,retirecounterexamples,grantOriginalrecovery,physicalreset,arbitraryduration/FPSoruniversalrepair.')
 from broader_strict_binding_final_gate import supplement
 result['current_runtime_failure_relevance']=supplement(r,ROOT,rows,list(actual.values()),plan)
 result['sources'].update({n:r.digest(n)for n in python_sources(ROOT,['tools/broader_runtime_caller_final_gate.py'])})
 require(all(r.digest(n)==h for n,h in result['sources'].items()),'Setup failure-gate source drift')
 return result

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh failure relevance record required')
 with Reader(ROOT)as r:
  try:q=assess(r)
  except(ValueError,KeyError,TypeError,OSError,StopIteration)as e:q=dict(schema=1,kind='broader_current_historical_failure_relevance',success=False,error=str(e),original_game_runs=0,historical_failures_retired=False,universal_repair_claimed=False)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
