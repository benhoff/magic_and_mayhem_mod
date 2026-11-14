#!/usr/bin/env python3
"""Keep historical causes distinct from current strict and full-intro execution."""
import argparse,json,struct
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_acceptance import checked,native_phase
from broader_player_matrix import BINARY,P
from rendering_menu_acceptance import artifact
from rendering_pilot import require
import sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import media_v1

def startup(r,a,c,v):
 require(a['success']and a['configuration']['skip_intros']is False and '--skip-movies'not in a['command']and a['flow']['success'],'Full-intro Quick coupling missing')
 require(not a['configuration'].get('diagnostic_only',False)and not a['configuration'].get('injected_fault'),'Diagnostic/fault run cannot establish normal startup')
 media=json.loads(artifact(r,a,'native-media-report.json'));intros=[row for row in media if row.get('operation')==1 and str(row.get('asset','')).replace('\\','/').rsplit('/',1)[-1].lower()in('intro0.avi','intro1.avi')]
 require(len(intros)==2 and {str(row['asset']).replace('\\','/').rsplit('/',1)[-1].lower()for row in intros}=={'intro0.avi','intro1.avi'}and all(row.get('status')==3 and row.get('accepted')is True and not row.get('error')and min(row.get('video_frames',0),row.get('presented_video_frames',0))>0 for row in intros),'Both intros must naturally complete and present video in this Quick session')
 wire=artifact(r,a,'native-media.bin');require(media_v1.valid_header(wire[:media_v1.HEADER_SIZE],len(wire))and struct.unpack_from('<I',wire,media_v1.CANCELLED_REQUEST_ID_OFFSET)[0]==0,'Startup media cancellation or invalid retirement')
 first=c['first_presented_world'];require(first['verified']and 0<first['generation']<=int(a['resident']['completed'])and c['original_reference']['worlds']==int(a['resident']['completed'])and c['original_reference']['continuous_from_owned_initial_history']and not c['original_reference']['native_player_pixels_read']and not c['original_reference']['original_destination_capture_read'],'Incomplete first-World/continuous-history proof')
 require(v['response_progress']['verified']and v['output_history']['output']['world_presentations']>0 and int(a['resident']['completed'])>5 and not a['resident']['error'],'First-World stall or missing advancing source work')
 return dict(verified=True,movies=[{k:row[k]for k in('request','asset','status','video_frames','presented_video_frames')}for row in intros],producer_cancelled_request=0,first_presented_world=first,source_generations=int(a['resident']['completed']),response_progress=v['response_progress'],historical_cause_proved=False,universal_watchdog_repair_proved=False)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh failure relevance review required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));pre=checked(r,ref(P+'historical-relevance-preflight-01.json'));old=checked(r,ref('research/runtime/whole-player-validation-results-20261110/historical-impact-01.json'));strict=[];quick=[];sources={n:r.digest(n)for n in('tools/check-broader-failure-relevance.py','tools/native_media_acceptance.py','tools/rendering_player_acceptance.py','tools/broader_player_matrix.py')}
  for profile in('default','software'):
   for seconds in(10,30):
    name=P+f'bp11-strict-{profile}{seconds}-01-validation-01.json';v=checked(r,ref(name));require(v['functional_acceptance']['verified']and v['stress_seconds']==seconds and v['response_progress']['verified'],'Strict B05 phase missing');a=checked(r,v['acquisition']);require(native_phase(ROOT,r,a,BINARY)['current_native_source_candidate']and all(r.digest(n)==h for n,h in v['sources'].items()),'Strict execution is no longer current');strict.append(dict(validation=ref(name),functional_acceptance=v['functional_acceptance'],response_progress=v['response_progress']));sources.update(v['sources'])
   for width in(640,800):
    name=P+f'bp11-world-quick-{profile}{width}-01-validation-01.json';v=checked(r,ref(name));require(v['family']=='quick'and v['current_native_phase']['current_native_source_candidate'],'Wrong startup followup');a=checked(r,v['acquisition']);c=checked(r,v['comparison']);require(native_phase(ROOT,r,a,BINARY)['current_native_source_candidate']and all(r.digest(n)==h for n,h in v['sources'].items()),'Startup execution is no longer current');quick.append(dict(validation=ref(name),profile=v['profile'],size=v['actual_initial_size'],proof=startup(r,a,c,v)));sources.update(v['sources'])
  failures=[]
  for n in ['research/runtime/rendering-seventeenth-five-results-20261023/strict-campaign-01-journey.json','research/runtime/rendering-seventeenth-five-results-20261023/strict-campaign-02-journey.json']:
   a=r.json(n);require(a['success']is False,'Historical B05 negative changed');failures.append(dict(reference=ref(n),success=False,error=a.get('error'),source_hashes_preserved=True))
  q=dict(schema=1,kind='broader_current_historical_failure_relevance',success=True,original_game_runs=0,preflight=ref(P+'historical-relevance-preflight-01.json'),historical_startup=pre['historical_startup'],historical_WP10_impact=ref('research/runtime/whole-player-validation-results-20261110/historical-impact-01.json'),historical_B05_failures=failures,current_B05_strict=strict,current_full_intro_first_World=quick,B05=dict(current_same_contract_10_and_30_second_stress_verified=True,historical_cause='unclassified',historical_negative_retired=False,scope='Exact published strict two-phase readiness/callback/resume/Quit/resource policy on both current providers. No wizard-movement outcome or universal reliability claim.'),B06=dict(instrumented_historical_FPS=17.253776873910518,historical_19FPS_accepted=False,measurement_overhead='unresolved',fixed_FPS_acceptance_gate=False,historical_failure_retired=False),RN31=dict(current_finite_coupling_verified=True,historical_source_phase_preserved=True,historical_failure_retired=False,historical_cause_proved=False,current_failure_observed=False,current_relevance='The same full-intro-to-first-World coupling passes on both current providers/extents with complete owned history, independent first/continuing presented pixels and existing acknowledgement/input/menu/cleanup limits. Current native/runtime fingerprints are checked in full; old negative sources are never waived or promoted. This bounds current finite non-reproduction and does not diagnose or erase the historical watchdog/current-history refusal.'),current_scope_supported_only_by_fresh_positive_execution=True,historical_failures_retired=False,universal_repair_claimed=False,sources=sources,sources_stable=all(r.digest(n)==h for n,h in sources.items()))
  require(q['sources_stable'],'Failure-review source drift');h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,current_strict_cases=len(strict),current_startup_cases=len(quick),historical_failures_retired=False)))
if __name__=='__main__':raise SystemExit(main())
