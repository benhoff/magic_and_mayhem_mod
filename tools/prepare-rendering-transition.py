#!/usr/bin/env python3
"""Freeze a finite recovery budget and the exact built native/collector phase."""
import argparse,hashlib,importlib.util,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/rendering-transition-recovery-results-20261107/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,required=True);p.add_argument('--report',default=PREFIX+'input-plan-01.json');p.add_argument('--previous-plan');p.add_argument('--prior-request',action='append',default=[]);p.add_argument('--review-reason');p.add_argument('--current-master',action='store_true');a=p.parse_args()
 require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned plan required')
 spec=importlib.util.spec_from_file_location('transition_build',ROOT/'tools/test-native-campaign.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 with Reader(ROOT) as r:
  parent_name=PREFIX+'inputs-01.json';parent=r.json(parent_name);require(parent['success'],'Unverified retained inputs')
  historical=r.json(parent['parent']['record']);names=m.linked_compiler_sources(a.build,ROOT)
  require(names==set(historical['compiler_sources'])|{'apps/qt-shell/resident_capture_path.hpp'},'Unexpected linked compiler universe')
  source=a.build/'mnm-qt-shell';require(source.is_file() and all(source.stat().st_mtime_ns>=(ROOT/n).stat().st_mtime_ns for n in names),'Player predates current linked source')
  compiler={n:r.digest(n) for n in sorted(names)};runtime={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT))}
  if a.current_master:
   require(a.previous_plan and a.review_reason,'Current master phase needs exact review')
   head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
   require(all(hashlib.sha256(subprocess.check_output(['git','show',head+':'+n],cwd=ROOT)).hexdigest()==h for n,h in runtime.items()),'Owned native runtime changes outside reviewed master')
  else:require(runtime==historical['runtime_sources'],'Runtime DLL source phase changed')
  cache=(a.build/'CMakeCache.txt').read_text();require('CMAKE_BUILD_TYPE:STRING=Release' in cache and 'MNM_EXPERIMENTAL_MINI_MENUS:BOOL=OFF' in cache,'Wrong native build')
  binary=ROOT/'working/build/transition-player/mnm-qt-shell';binary.parent.mkdir(parents=True,exist_ok=True);raw=source.read_bytes();binary.write_bytes(raw);binary.chmod(0o755);rec=a.report.removesuffix('.json')+'-player.bin';h=write_report(ROOT,rec,raw);player=dict(path=str(binary.relative_to(ROOT)),record=rec,sha256=h)
  controls_name=PREFIX+'fault-controls-01.json';controls=r.json(controls_name);require(controls['success'] and controls['sources_stable'] and all(r.digest(n)==h for n,h in controls['sources'].items()),'Fault controls changed')
  artifact=next(v for v in controls['artifacts'] if v['path']=='optimized');raw=r.read_bytes(artifact['record']);require(hashlib.sha256(raw).hexdigest()==artifact['sha256'],'Fault binary changed');fault=binary.parent/'transition-fault';fault.write_bytes(raw);fault.chmod(0o755);fault_binary=dict(path=str(fault.relative_to(ROOT)),record=artifact['record'],sha256=artifact['sha256'])
  reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']=='NP.transition-recovery-acceptance');s=next(v for v in reg['scenarios'] if v['id']=='native.transition-recovery-acceptance')
  sources={n:r.digest(n) for n in sorted(required_sources(b)|{'tools/prepare-rendering-transition.py'})}
  logs=[]
  for n in ('working/epoch-player-configure-01.log','working/epoch-player-build-01.log','working/epoch-player-build-02.log','working/epoch-player-build-03.log','working/epoch-player-build-04.log','working/transition-parent-applicability-01.log','working/transition-inputs-01.log','working/transition-inputs-02.log',*(['working/epoch-player-build-05.log'] if a.current_master else [])):
   record=a.report.removesuffix('.json')+'-'+Path(n).name+'.bin';logs.append(dict(path=n,record=record,sha256=write_report(ROOT,record,(ROOT/n).read_bytes())))
  q=dict(schema=1,kind='finite_public_transition_recovery_plan',success=True,compiler_sources=compiler,runtime_sources=runtime,player_binary=player,fault_binary=fault_binary,oracle=parent['oracle'],sources=sources,
   parent=dict(record=parent_name,sha256=r.digest(parent_name)),logs=logs,bounds=dict(maximum_requests=6,maximum_original_games=10,primary_cases=4,primary_original_starts=8,repair_cases=1,stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),domains=[dict(software=v,size=z) for v in (False,True) for z in ([800,600],[640,480])],
   input_policy=dict(entry='ordinary movies enabled, physical intro skip, NewGame/Adept/Enter',mini_dwell_seconds=3,minimum_samples=30,terminal_stability_ns=600000000,terminal_minimum_samples=8,fault='existing mnm_ring_fail INVALID, active drained real Mini transport3; only state/reason',restart='physical public Ctrl+Shift+R',fresh_exit='ordinary Campaign Quit/Yes/report/Main Quit',compiled_test_driver=False,original_pixels_as_native_inputs=False,old_epoch_normal_credit=False,deadline_seconds=600),
   claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})],original_game_runs=0,new_native_capture_code=True,new_native_rendering_code=False,
   limits='One declared drained-Mini fault and fresh-generation recovery only; no pending CPU, physical context loss, arbitrary races/providers, in-place/save recovery, nominal failed-epoch credit or final renderer signoff.')
  if a.previous_plan:
   require(a.review_reason and a.prior_request,'Preparation phase needs exact failure review')
   previous=r.json(a.previous_plan)
   if a.current_master:
    q.update(reviewed_master=head,previous_native_phase_historical_only=True,new_native_build=True)
    q['bounds'].update(maximum_requests=9,zero_game_preparations=4,repair_cases=1)
    from check_inputs import paths
    shared=paths(ROOT);require(shared.get('assets')==(ROOT/'working/assets/native-cd-audio-20261012').resolve(),'Shared audio input missing verified local catalog');q['verified_shared_inputs']={k:str(v) for k,v in shared.items()}
   else:require(previous['player_binary']['sha256']==player['sha256'] and previous['compiler_sources']==compiler and previous['runtime_sources']==runtime,'Preparation changed native phase')
   q.update(previous_plan=dict(record=a.previous_plan,sha256=r.digest(a.previous_plan)),prior_requests=[dict(record=n,sha256=r.digest(n)) for n in a.prior_request],review_reason=a.review_reason)
  print(write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
