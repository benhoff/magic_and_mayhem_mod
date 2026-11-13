#!/usr/bin/env python3
"""Review actual NVIDIA presentation against the retained finite supplier boundary."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from broader_player_acceptance import checked,native_phase
from broader_player_picking_target_final_matrix import BINARY,P,specs,current
import importlib.util,sqlite3
from broader_player_sources import python_sources

from rendering_pilot import require
from broader_picking_target_final_gate import prior_context
ROOT=Path(__file__).resolve().parents[1]
def retained_fullscreen(r,record,profile):
 # The original pure checker keeps every output/input/storage/lifetime assertion.
 # Substitute only its source-policy dependency in this isolated module instance.
 path=ROOT/'tools/required_boundaries_gate.py';spec=importlib.util.spec_from_file_location('publication_fullscreen_facts',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 a=checked(r,record['acquisition']);v=record['value']
 require(all(n in v['sources']and v['sources'][n]==h for n,h in a['sources'].items()),'Acquisition source omitted from exact reviewed validation')
 def reviewed(reader,sources):
  require(sources==a['sources'],'Foreign acquisition source map')
  actual={n:record['current_sources'][n]for n in sources};require(all(reader.digest(n)==h for n,h in actual.items()),'Current fullscreen source changed')
  return actual,record['publication_applicability']
 core.review_source_phase=reviewed
 return core.fullscreen_case(r,record['acquisition'],v['comparison'],profile)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh actual-provider impact review required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));parent_ref=ref('research/runtime/required-boundaries-results-20261110/closeout-admission-02.json');parent=checked(r,parent_ref);require(parent['passed_requirements']==156 and parent['pending_requirements']==0 and len(parent['requirements'])==156,'Changed finite boundary basis');proof=checked(r,parent['proof']);require(proof['gate_ready'],'Retained finite boundary did not close')
  with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
   db.row_factory=sqlite3.Row;rows={v['case_id']:dict(v)for v in db.execute('select *from requests')}
  contexts={s['case']['id']:current(ROOT,r,s,rows[s['case']['id']])for s in specs(r)if s['role']=='boundary'}
  soft_record=prior_context(r,contexts,'bp29-boundary-software800-01');native_record=prior_context(r,contexts,'bp29-boundary-default800-01')
  sr=soft_record['validation'];sv=checked(r,sr);require(sv['provider']=='mesa'and sv['profile']=='player-software'and sv['family']=='boundary'and sv['required_boundary_details']['storage']['every_world_checked']and all(r.digest(n)==h for n,h in soft_record['current_sources'].items()),'Current software boundary proof missing');software=dict(acquisition=sv['acquisition'],comparison=sv['comparison']);a=checked(r,software['acquisition']);phase=native_phase(ROOT,r,a,BINARY);require(phase['current_native_source_candidate'],'Software fullscreen native source changed');soft=retained_fullscreen(r,soft_record,'player-software')
  nr=native_record['validation'];nv=checked(r,nr);require(nv['provider']=='nvidia'and nv['family']=='boundary'and nv['current_native_phase']['current_native_source_candidate']and nv['required_boundary_details']['storage']['every_world_checked'],'Current NVIDIA boundary proof missing');require(all(r.digest(n)==h for n,h in native_record['current_sources'].items()),'NVIDIA boundary sources changed')
  na=checked(r,nv['acquisition']);require(native_phase(ROOT,r,na,BINARY)['current_native_source_candidate'],'Current NVIDIA native phase changed');hw=checked(r,ref(P+'hardware-overlap-controls-02.json'));require(hw['actual_hardware']['verified']and hw['approved_scope_amendment_unchanged']and hw['sources_stable']and all(r.digest(n)==h for n,h in hw['sources'].items()),'Current hardware controlled mechanism missing')
  sources=dict(native_record['current_sources'])|soft_record['current_sources']|{n:r.digest(n)for n in('tools/check-broader-picking-target-boundary-impact.py','tools/required_boundaries_gate.py','tools/broader_player_acceptance.py','tools/broader_player_runtime_inputs_matrix.py','docs/native-rendering-support-domain.json','docs/native-rendering-required-boundaries.json')}
  sources.update({n:r.digest(n)for n in python_sources(ROOT,['tools/check-broader-picking-target-boundary-impact.py'])})
  decisions={v['id']:v for v in parent['boundary_scope_decisions']};require(len(decisions)==8 and all(v['player_scope_state']=='resolved'and v['original_recovery_state']=='unresolved'for v in decisions.values()),'Changed required technical boundary decisions')
  scopes={
   'SC.alternate-ownership':'Current software and NVIDIA fullscreen observations check disabled alternate storage and active backing at every original World; zero-padding/disabled-alternate input remains exact. No historical alternate owner is relabeled recovered.',
   'SC.padded-fullscreen':'Both actual providers have twelve physical toolbar/F9 transitions, exact affine input/hover-picking, complete independent displayed pixels,800/640/800recreation, restored window geometry and normal retirement. Original exclusive/padded DirectDraw remains outside the exact native presentation configuration.',
   'SC.natural-callers':'All finite original caller/input identities and declared native ownership remain bounded. The approved original/Mesa8x6opaque-overlap mechanism remains unchanged; actual NVIDIA backend/replay controls supplement it. Natural alias reachability, live COM alias discovery and unknown original domains remain open.',
   'SC.prior-history':'Current matrix histories are continuous from owned initial source state and compare every required viewport pixel. Failed epochs retain only their declared compared prefixes; fresh generations have separate state. The old eight-frame counterexample remains negative/historical.',
   'SC.map-save-domain':'Preserve exact demonstrated Quick selectors and Celtic1to2routes, actual serializer/save round trips and observed durations. All47journey leaves and21operations per profile, both results and actual later-map progression remain mandatory in the separate current matrix. Installed assets grant no arbitrary map/save/duration support.',
   'SC.screen-provider-lifetime':'Retain the actual current-source original Main/Realm/World/Report/coordinator/resource observations and their external supplier boundaries. The isolated original guest has no host graphics devices; the changed native host GL provider is tested by current complete presented pixels, original callbacks and owner retirement in the same finite journey families. Unresolved Realm indirect tails, external suppliers and Windows exception dispatch remain original-recovery gaps.',
   'SC.rgb555-live':'The supported live World/native primary remains RGB565 with current every-World format/active-backing checks on both providers. Retained original-getter and explicit offscreen RGB555 controls retain their own narrower provider identity. No live RGB555 sprite/effect support or global format1unreachability is inferred.',
   'SC.physical-compatibility':'Name actual Linux/Wine PE32/Qt X11/OpenGL software and NVIDIA-via-Zink/Vulkan execution, with authenticated private native displays and isolated original graphics suppliers. No physical scanout, device reset, Win98/ME/Windows SEH or original hardware-driver equivalence is asserted.'}
  require(set(scopes)==set(decisions),'Boundary review lost a required decision')
  q=dict(schema=1,kind='broader_current_provider_boundary_impact',success=True,original_game_runs=0,retained_finite_master=parent_ref,parent_requirement_rows_sha256=fingerprint(parent['requirements']),parent_requirement_rows_preserved=156,parent_scope_amendments_sha256=fingerprint(parent['scope_amendments']),technical_conditions_waived=False,software_fullscreen=dict(validation=sr,acquisition=software['acquisition'],comparison=software['comparison'],native_phase=phase,proof=soft),nvidia_fullscreen=nr,nvidia_controlled_overlap=ref(P+'hardware-overlap-controls-02.json'),required_boundary_review=scopes,review_kind='retained-original-supplier-boundaries-plus-current-provider-output-input-and-lifetime',source_phase_waivers_for_native_files=False,current_matrix_must_still_pass=True,publication_applicability=ref(P+'completed-journal-publication-applicability-01.json'),reviewed_python_publication_only=False,native_files_waived=[],original_recovery_complete=False,historical_failures_retired=False,sources=sources,sources_stable=all(r.digest(n)==h for n,h in sources.items()))
  require(q['sources_stable'],'Boundary review source drift');h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,boundaries=len(scopes))))
if __name__=='__main__':raise SystemExit(main())
