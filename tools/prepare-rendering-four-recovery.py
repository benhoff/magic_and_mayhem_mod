#!/usr/bin/env python3
"""Freeze two missing Low fault cases after the proven private-display repair."""
import hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import required_sources
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/rendering-recovery-four-results-20261108/'
LEGACY='research/runtime/rendering-transition-recovery-results-20261107/'
REPORTS={LEGACY+'four09-default640-01-acquisition.json':dict(software=False,size=[640,480]),
         LEGACY+'four09-software640-01-acquisition.json':dict(software=True,size=[640,480])}
def main():
 with Reader(ROOT) as r:
  old=LEGACY+'closeout08-input-plan-02.json';q=r.json(old)
  require(all(r.digest(n)==h for n,h in q['compiler_sources'].items()),'Retained406-source native player changed')
  startup='research/runtime/native-startup-display-results-20261108/'
  controls=r.json(startup+'after-controls-01.json');observed=r.json(startup+'startup-postmortem-01.json')
  require(controls['success'] and controls['sources_stable'] and [v['authenticated'] for v in controls['cases']]==[True,True,False]
          and all(r.digest(n)==h for n,h in controls['sources'].items()),'Current disk/RAM/wrong-cookie proof required')
  require(observed['startup_verified'] and not observed['normal_quit_verified'] and observed['original_game_runs']==1,'Wrong startup boundary')
  runtime={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT))}
  added=sorted(set(runtime)-set(q['runtime_sources']))
  require(all(runtime.get(n)==h for n,h in q['runtime_sources'].items()) and added==['reconstruction/rendering/dc_image_wrappers.hpp','reconstruction/rendering/font_screen_installation.hpp'],'Unexpected native/runtime catalog delta')
  q['conservative_source_catalog_review']=dict(previous_paths=1021,current_paths=1023,added_unlinked_portable_models=added,existing_sources_unchanged=True,reason='CR08 added two standalone original caller models; no existing guest/native source changed and all406linked player compiler inputs remain exact. Capture the complete current conservative catalog without pretending the historical catalog contained these files.')
  for b in [q['player_binary'],q['fault_binary'],*q['oracle']['binaries']]:
   require(r.digest(b['record'])==b['sha256']==hashlib.sha256((ROOT/b['path']).read_bytes()).hexdigest(),'Exact retained executable unavailable')
  b=next(v for v in r.json('research/runtime/coverage/register.json')['behaviors'] if v['id']=='NP.transition-recovery-acceptance')
  sources={n:r.digest(n) for n in sorted(required_sources(b)|set(controls['sources'])|{'tools/prepare-rendering-four-recovery.py'})}
  require(not (ROOT/'working/transition-four09-attempts.sqlite3').exists(),'New bounded collection state already exists')
  q.update(previous_plan=dict(record=old,sha256=r.digest(old)),sources=sources,runtime_sources=runtime,
   bounds=dict(maximum_requests=2,maximum_original_games=4,primary_cases=2,primary_original_starts=4,repair_cases=0,stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
   execution_paths=dict(schema=1,output_root='working/recovery-four09/acceptance',budget='working/transition-four09-attempts.sqlite3',reports=REPORTS),
   domains=[dict(software=v,size=[640,480]) for v in (False,True)],prior_requests=[],
   historical_stopped_collections=[dict(name='RT07',requests=11,original_game_runs=9),dict(name='RT08',requests=2,original_game_runs=1),dict(name='SX08',requests=1,original_game_runs=1)],
   current_authority_control=dict(record=startup+'after-controls-01.json',sha256=r.digest(startup+'after-controls-01.json')),
   startup_observation=dict(record=startup+'startup-postmortem-01.json',sha256=r.digest(startup+'startup-postmortem-01.json')),
   parent_acceptance=dict(record='research/runtime/rendering-defeat-results-20261108/acceptance-01.json',sha256=r.digest('research/runtime/rendering-defeat-results-20261108/acceptance-01.json')),
   owned_base=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
   review_reason='User requested four easiest remaining operation rows. Choose transition-recovery and failure-cleanup in both profiles: two High domains already have raw independently compared fault/restart evidence. Only two missing Low domains are collected after the reproduced namespace repair. Existing private dbus-daemon, movies-enabled physical entry, declared drained-Mini INVALID, physical public restart and ordinary Quit are reused. Old request databases remain stopped. No retry, High recollection, new fixture, native raster/guest change, physical loss or final decision credit.',
   retained_high_scope='Historical1019-path guest phase remains pinned. It is retained input to new raw assessment, not current1023-path conservative source catalog execution or a native-source freshness waiver.',
   row_candidates=['operation.transition-recovery','operation.failure-cleanup'],new_native_capture_code=False,new_native_rendering_code=False,
   selection_review=[dict(requirements=['transition-recovery','failure-cleanup'],rank=1,reason='Shared complete High fault/restart/raw owner/output evidence; two bounded Low domains missing.'),dict(requirements=['palette-cycling'],rank=2,reason='Original palette kernels/providers exist; real refresh/selection/delivery/retirement through required transitions still missing.'),dict(requirements=['copy-stretched','copy-alias-overlap'],rank=3,reason='Supplied controls exist; retained natural route census lacks required stretch/self-alias witnesses.'),dict(requirements=['text-font-dc-lifetime','flags-and-return-values'],rank=4,reason='Recent original coordinator/wrapper comparisons do not supply installed font/DC lifetime or complete production caller state.'),dict(requirements=['surface-loss-restore'],rank=5,reason='Actual provider loss/Restore or a precise compatibility boundary remains separate from declared producer failure.')])
  note=(ROOT/'working/four09-parent-applicability.log').read_bytes();q['targeted_applicability']=dict(record=P+'parent-applicability.log.bin',sha256=write_report(ROOT,P+'parent-applicability.log.bin',note))
  print(write_report(ROOT,P+'input-plan-01.json',(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
