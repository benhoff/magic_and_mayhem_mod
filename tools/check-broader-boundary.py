#!/usr/bin/env python3
"""Require current provider identity and complete fullscreen/input/retirement proof."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_acceptance import checked,native_phase
from broader_world_policy import assess as edge_assess
from journeys_gate import lifetime
from pixels_history_gate import Join
from native_render_driver import require_nvidia
from rendering_pilot import require
from required_boundaries_gate import fullscreen_case
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def assess(r,acquisition,comparison):
 ref=lambda n:dict(record=n,sha256=r.digest(n));pair=dict(acquisition=ref(acquisition),comparison=ref(comparison));a=checked(r,pair['acquisition']);c=checked(r,pair['comparison']);p=checked(r,a['broader_phase']);case=next(v for v in p['broader_cases']if v['report']==acquisition)
 require(p['family']=='boundary'and c['acquisition']==pair['acquisition']and a['actual_provider_verified']and a['sources_stable']and c['sources_stable']and c['comparison_processes_stopped']and c['original_manifest_verified_before_after'],'Incomplete boundary execution')
 require(case['provider']in('nvidia','mesa'),'Unknown fullscreen provider')
 software=case['provider']=='mesa';require(case['profile']==('player-software'if software else'player-default')and a['configuration']['software_flag']is software,'Wrong boundary provider/profile')
 if software:require(a['resident']['renderer'].lower().startswith(('llvmpipe','softpipe')),'Actual software fullscreen provider missing')
 else:require_nvidia([a['resident']['renderer']])
 phase=native_phase(ROOT,r,a,p['player_binary']['sha256']);require(phase['current_native_source_candidate'],'Historical boundary native sources')
 require(c['proof']['verified']and c['proof']['fullscreen_transitions']==12 and len(c['proof']['rebuilds'])==2 and c['comparison']['mismatching_pixels']==0,'Incomplete fullscreen proof')
 detailed=fullscreen_case(r,pair['acquisition'],pair['comparison'],case['profile'])
 g=checked(r,p['broader_plan']);fixture=checked(r,g['owned_input']);initial=fixture['input_fingerprints'];actual=a['installation_inputs'];allowed=set(fixture['permitted_mutations']);require(actual.keys()==initial.keys()and all(actual[k]==v for k,v in initial.items()if k not in allowed),'Boundary source installation changed')
 storage=c['proof']['storage'];require({tuple(v[:2])for v in storage['extents']}=={(640,480),(800,600)}and len(storage['formats'])==1,'Missing boundary storage context')
 output=Join(r).pair(pair,case['profile']);retired=lifetime(r,pair['acquisition'],case['profile'],dict(case=output))
 path=ROOT/'tools/check-broader-player-case.py';spec=importlib.util.spec_from_file_location('broader_boundary_progress',path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);progress=m.epoch_progress(dict(resident=a['resident']),c)
 sources=a['sources']|c['sources']|{n:r.digest(n)for n in('tools/check-broader-boundary.py','tools/compare-broader-boundary.py')};edge=None
 if c.get('native_edge_policy'):edge=edge_assess(r,c);sources.update(edge['oracle_sources'])
 require(all(r.digest(n)==h for n,h in sources.items()),'Boundary analysis sources changed')
 q=dict(schema=1,kind='current_broader_world_case_validation',success=True,sources_stable=True,original_game_runs=0,actual_acquisition_original_games=1,acquisition=pair['acquisition'],comparison=pair['comparison'],family='boundary',profile=case['profile'],provider=case['provider'],actual_driver=a['resident']['renderer'],actual_initial_size=[800,600],current_native_phase=phase,output_history=output,lifetime=retired,semantics=c['proof'],required_boundary_details=detailed,input_binding=dict(verified=True,fixture=g['owned_input']),response_progress=progress,sources=sources,current_case_proved=True,scope='Actual current explicitly observed NVIDIA or software Qt/X11/OpenGL fullscreen and restoration, both extents in one original process, independent complete presented output, original input/progress and passive boundary metadata. Retained original supplier and physical-platform exclusions unchanged.')
 if edge:q['native_edge_policy']=edge
 return q
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('acquisition','comparison','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args()
 with Reader(ROOT)as r:
  try:q=assess(r,args.acquisition,args.comparison)
  except(ValueError,KeyError,OSError,TypeError)as e:q=dict(schema=1,kind='current_broader_boundary_validation',success=False,original_game_runs=0,error=str(e))
 require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh boundary validation required');h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h)));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
