#!/usr/bin/env python3
"""Join bounded sampler/input controls and actual diagnostic player observations."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import traceback
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, required_sources, scenario_contract
ROOT=Path(__file__).resolve().parents[1]

def digest(name):return hashlib.sha256((ROOT/name).read_bytes()).hexdigest()

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['controls','admission-controls','input-controls','negative-intro','baseline-input','baseline-comparison','preimages','observer-failure','fault','full-intro','comparison','report']:p.add_argument('--'+n,required=True)
 a=p.parse_args()
 if not a.report.startswith('research/runtime/rendering-watchdog-results-20261101/') or (ROOT/a.report).exists():p.error('New immutable owned report required')
 result=dict(schema=1,kind='parent_debugger_and_input_wait_validation',success=False,
  historical_failure=dict(id='RN31.G.first-world-watchdog',resolution='unresolved'),nominal_requirements_passed=0,
  original_behavior_recovered=False,producer_deadlines_or_ownership_changed=False,
  scope='Current scoped diagnostic sampler and acceptance-input wait only. Old negative source domains remain immutable and explicitly separate. Finite original/history output agreement does not retire the historical watchdog or the separate Main transition failure, admit nominal requirements or recover general original input/camera behavior.')
 sources={'tools/review-rendering-stall-sampler.py':digest('tools/review-rendering-stall-sampler.py')};records=[]
 with Reader(ROOT) as reader:
  def get(name):
   value=reader.json(name);records.append(dict(record=name,sha256=reader.digest(name)));return value
  def artifact(acquired,suffix):
   entries=[r for r in acquired['artifacts'] if r['path'].endswith(suffix)];assert len(entries)==1,(suffix,len(entries))
   row=entries[0];raw=reader.read_bytes(row['record']);assert hashlib.sha256(raw).hexdigest()==row['sha256'];return raw
  def merge(entries):
   for n,sha in entries.items():
    assert digest(n)==sha,('Current source differs',n)
    assert n not in sources or sources[n]==sha,n
    sources[n]=sha
  try:
   controls=get(a.controls);admission=get(a.admission_controls);inputs=get(a.input_controls)
   assert controls['success'] and controls['original_artifacts_consumed'] is False and controls['live_game_validated'] is False
   assert admission['success'] and admission['engine_executed'] is False
   assert inputs['success'] and inputs['original_artifacts_consumed'] is False
   for v in [controls,admission,inputs]:merge(v['sources'])
   executed=[r['result'] for r in inputs['checks'] if 'result' in r]
   assert len(executed)==2 and all(r['poll_after_old_hold_window_observed_before_release'] and r['missing_poll_refused_without_retry'] and r['wait_deadline_ms']==3000 for r in executed)
   preimages=get(a.preimages);baseline=get(a.baseline_input);failed_comparison=get(a.baseline_comparison)
   old_driver=preimages['baseline_driver'];assert reader.digest(old_driver['record'])==old_driver['sha256']==baseline['sources']['apps/qt-shell/live_quick_battle_app_test.cpp']
   old_body=reader.read_text(old_driver['record']);assert 'QTimer::singleShot(250,' in old_body and 'QTimer::singleShot(700,' in old_body
   assert old_driver['sha256']!=digest('apps/qt-shell/live_quick_battle_app_test.cpp')
   assert baseline['success'] and baseline['retirement']['state']=='completed'
   assert not failed_comparison['success'] and failed_comparison['error']=='Incomplete original Right down/release transitions'
   assert failed_comparison['sources']['tools/rendering_pilot.py']==digest('tools/rendering_pilot.py')
   old_input=artifact(baseline,'/render-input.bin.acceptance');assert old_input[:8]==b'MNMIAK01'
   old_rows=list(struct.iter_unpack('<12I',old_input[64:]));published=baseline['flow']['inputs'];assert len(old_rows)==2 and len(published)==4
   assert all(not(published[0]['sequence']<=r[1]<published[1]['sequence']) for r in old_rows)
   assert published[2]['sequence']<=old_rows[0][1]<published[3]['sequence'] and old_rows[0][3]&0x8000 and old_rows[1][3]&0x8000==0
   assert baseline['native_admission']['native_only']['sources']['runtime/render/input_polling.h']==digest('runtime/render/input_polling.h')
   result['input_driver_repair']=dict(baseline_missing_first_poll_receipt=True,old_nominal_hold_ms=450,
    exact_old_poll_timing_not_recorded=True,transient_observer_contention_not_excluded=True,
    delayed_peer_control_observed=True,original_input_admission_unchanged=True,original_input_hook_unchanged=True,
    current_wait_limit_ms=3000,scope='The old elapsed-time pulse did not guarantee an observed original poll. Controlled delayed polling demonstrates the failure mechanism; exact baseline poll timing/contention is not recovered.')
   negative=get(a.negative_intro)
   negative_flow=json.loads(artifact(negative,'/flow.json'))
   assert negative['success'] is False and negative['kind']=='public_native_watchdog_diagnostic_acquisition'
   assert negative['configuration']['skip_intros'] is False and 'injected_fault' not in negative['configuration']
   assert negative['stall_sampler_valid'] and negative['stall_sampler']['sample'] is None and negative['stall_sampler']['peak_pending_seconds']<1.5
   assert negative_flow['stage']==1 and negative_flow['world'] is False
   assert negative_flow['error']=='Menu adapter timed out; using original menus. Requests will not be retried.'
   assert negative['sources']['apps/qt-shell/live_quick_battle_app_test.cpp']==old_driver['sha256']
   raw=artifact(negative,'/events.bin');assert raw[:16]==b'MNMMENU1'+struct.pack('<II',1,64)
   events=list(struct.iter_unpack('<16I',raw[16:]));assert len([r for r in events if r[1]==2 and r[3]==3 and r[9]==2])==len([r for r in events if r[1]==3 and r[3]==3 and r[9]==2])==1
   assert len(negative['experiments'])==1
   temporary=Path(negative['temporary_root'])
   assert temporary.is_absolute() and temporary.parts[-3:-1]==('working','pilot')
   recorded_root=temporary.parents[2]
   generation=Path(negative['experiments'][0]).relative_to(recorded_root)
   assert generation.parts[:-1]==('working','experiments','menu-observer') and generation.name.startswith('run-')
   menu_path=str(generation/'channel.bin')
   menu_entries=[r for r in negative['artifacts'] if r['path']==menu_path];assert len(menu_entries)==1
   menu=reader.read_bytes(menu_entries[0]['record']);assert hashlib.sha256(menu).hexdigest()==menu_entries[0]['sha256']
   assert menu[:16]==b'MNMMCMD5'+struct.pack('<II',5,73728)
   lane=struct.unpack_from('<8I',menu,128);assert lane[4]==1
   media=artifact(negative,'/native-media.bin');assert struct.unpack_from('<I',media,48)[0]==0 and struct.unpack_from('<II',media,512)==(3,3)
   result['negative_intro']=dict(failure_id='RW03.G.intro-main-transition',resolution='unresolved',
    v3_peak_pending_seconds=negative['stall_sampler']['peak_pending_seconds'],v3_sample_requested=False,
    original_main_action_entered_and_returned=True,terminal_menu_ack=lane[4],terminal_menu_status=lane[5],
    terminal_snapshot_does_not_order_retirement=True,final_movie='Intro1.avi',final_movie_status='COMPLETE',producer_cancel=False,
    current_source_equivalence_claimed=False,causal_attribution='Unknown Main transition/lease cause; no five-second V3 no-ACK stall was observed. Later finite passes do not retire this failure.')
   failed_observer=get(a.observer_failure);assert failed_observer['success'] is False and failed_observer['usable_timeline'] is False
   result['auxiliary_observer']=dict(usable_timeline=False,proof_uses_observer_timeline=False,record=a.observer_failure,sha256=reader.digest(a.observer_failure))
   fault=get(a.fault);live=get(a.full_intro);comparison=get(a.comparison)
   for acquisition in [fault,live]:
    assert acquisition['kind']=='public_native_watchdog_diagnostic_acquisition' and acquisition['configuration']['diagnostic_only'] is True
    assert acquisition['private_wine_stopped'] and acquisition['original_manifest_verified_before_after'] and acquisition['player_binary_stable'] and acquisition['compiler_closure_stable']
    assert acquisition['stall_sampler_valid'] and acquisition['stall_sampler']['player_binary']['sha256']==acquisition['player_build']['binary']['sha256']
    assert acquisition['stall_sampler']['deadline_or_ownership_changed'] is False
    assert reader.digest(acquisition['player_build']['binary']['record'])==acquisition['player_build']['binary']['sha256']
    merge(acquisition['sources'])
   assert fault['success'] is False and fault['host_fault_recorded'] and fault['configuration']['skip_intros']
   assert fault['configuration']['injected_fault']['kind']=='gui-stall-after-first-world-paint'
   sampled=fault['stall_sampler']['sample'];assert sampled and 1.5<=sampled['pending_seconds']<4.5
   log=artifact(fault,'/public-launch.log').decode(errors='replace')
   assert log.count('MNM_WATCHDOG_STACK_BEGIN')==log.count('MNM_WATCHDOG_STACK_END')==1 and 'nanosleep' in log and 'WatchdogHostFault::schedule' in log
   states=[]
   for row in fault['artifacts']:
    if row['path'].endswith('/lifecycle.log'):
     raw=reader.read_bytes(row['record']);assert hashlib.sha256(raw).hexdigest()==row['sha256'];states.extend(line for line in raw.decode().splitlines() if line.startswith('command_watchdog_state '))
   assert len(states)==1;values=[int(v,16) for v in states[0].split()[1:]];elapsed=(values[1]-values[0])&0xffffffff
   assert elapsed>=5000 and values[2]==values[3]==sampled['acknowledged']
   ring=struct.unpack_from('<16I',artifact(fault,'/render-commands.bin'));assert ring[6:8]==(3,4) and ring[5]!=ring[9]
   for n in ['runtime/render/command_queue.h','runtime/render/command_scheduler.h']:
    old=subprocess.check_output(['git','show','d0bf7a551:'+n],cwd=ROOT);assert hashlib.sha256(old).hexdigest()==digest(n)
   assert live['success'] and live['configuration']['skip_intros'] is False and 'injected_fault' not in live['configuration'] and live['retirement']['state']=='completed'
   assert live['flow']['original_input_wait'] is True and live['flow']['original_input_rows']==4 and live['flow']['original_input_wait_limit_ms']==3000
   assert comparison['success'] and comparison['kind']=='public_native_watchdog_diagnostic_comparison'
   assert comparison['acquisition']==dict(record=a.full_intro,sha256=reader.digest(a.full_intro)) and comparison['player_build_binding']['verified'] and comparison['original_manifest_verified_before_after']
   assert comparison['input_acknowledgements']['verified'] and comparison['input_acknowledgements']['right_downs']==comparison['input_acknowledgements']['releases']==2
   assert comparison['comparison']['mismatching_pixels']==comparison['comparison']['remaining_surfaces']==0
   merge(live['native_admission']['native_only']['sources']);merge(comparison['sources'])
   media=json.loads(artifact(live,'/native-media-report.json'));movies=[r for r in media if r['operation']==1]
   assert [Path(r['asset']).name for r in movies]==['Intro0.avi','Intro1.avi']
   assert all(r['status']==3 and r['accepted'] and r['video_frames']>0 and r['presented_video_frames']==r['video_frames'] for r in movies)
   assert struct.unpack_from('<I',artifact(live,'/native-media.bin'),48)[0]==0
   ring=struct.unpack_from('<16I',artifact(live,'/render-commands.bin'));assert ring[6:8]==(2,0) and ring[5]==ring[9]
   register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());builds={b['id']:b for b in register['builds']};claims=[]
   for bid,sid in [('NP.rendering-stall-sampler','RW03.parent-sampler'),('NP.acceptance-input-receipt-wait','RW03.original-input-wait')]:
    behavior=next(b for b in register['behaviors'] if b['id']==bid);scenario=next(s for s in register['scenarios'] if s['id']==sid)
    for n in required_sources(behavior):sources.setdefault(n,digest(n))
    claims.append(dict(behavior=bid,contract_sha256=behavior_contract(behavior,builds),scenarios={sid:scenario_contract(scenario)}))
   result.update(success=True,claims=claims,fault=dict(pending_ack_stack_sample=sampled,guard_elapsed_ms=elapsed,terminal_ring='FAILED/INTERRUPTED',historical_cause_reproduced=False),
    full_intro=dict(movies=[dict(asset=Path(r['asset']).name,frames=r['video_frames'],status=r['status']) for r in movies],sampler=live['stall_sampler'],
     input_acknowledgements=comparison['input_acknowledgements'],world_swaps=comparison['observations']['world_swaps'],reference=comparison['original_reference'],
     pixels_compared=comparison['comparison']['pixels_compared'],mismatching_pixels=0,producer_cancel=False,ring_drained=True,normal_original_quit=True))
  except Exception as error:result.update(error=repr(error),traceback=traceback.format_exc())
  result.update(sources=sources,records=records)
  sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],record=a.report,sha256=sha,error=result.get('error')),indent=2),flush=True)
  return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
