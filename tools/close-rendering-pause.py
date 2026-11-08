#!/usr/bin/env python3
"""Close only the two frozen pause rows after four raw-wire/output proofs."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint,required_sources,behavior_contract,scenario_contract
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from rendering_pause_assessment import assess
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-pause-results-20261107/'
BID='NP.pause-mini-acceptance';SID='native.pause-mini-acceptance';LEAF='journey.pause-preferences.pause'

def close_rows(plan,parent,cases):
 validate(plan);require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan)) and len(parent['requirements'])==156,'Frozen acceptance inventory changed')
 require(len(cases)==4 and {(v['software'],tuple(v['size'])) for v in cases}=={(f,s) for f in (False,True) for s in ((800,600),(640,480))}
  and len({v['identity'] for v in cases})==4,'Four unique actual pause domains required')
 for v in cases:
  require(v['verified'] is True and len(v['dwells'])==2 and v['cleanup']['verified'] is True and v['pixels']>0 and v['mismatching_pixels']==0,'Incomplete pause case')
  for d in v['dwells']:
   require(d['verified'] is True and d['samples']>=30 and d['duration_ns']>=2_000_000_000 and d['compared_resumed_paints']>0
    and not d['all_simulation_animation_pause_claimed'] and not d['music_playback_frozen_claimed'],'Incomplete or widened pause interval')
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Pause row already accepted')
   row.update(state='passed',admitted_execution_evidence=[dict(execution=v['pair'],actual_world_extents=[v['size']],provider=v['provider'],reason='Two real Campaign Mini dwells: original ready owner/ACK and frozen World returns, released/blocked gameplay input, acknowledged live media/music ownership, original Cancel callbacks, fresh independently compared resumed output, positive gameplay input and normal retirement.') for v in cases if ('player-software' if v['software'] else 'player-default')==row['configuration']])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==parent['passed_requirements']+2,'Pause closeout exceeded two rows')
 return rows

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',required=True);p.add_argument('--comparison',action='append',required=True);p.add_argument('--report',required=True);args=p.parse_args()
 require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned closeout required');q=dict(schema=1,success=False)
 try:
  with Reader(ROOT) as r:
   reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID)
   sources={n:r.digest(n) for n in required_sources(b)};cases=[];execution_sources={}
   for name in args.comparison:
    c=r.json(name);require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped'] and c['original_manifest_verified_before_after'],'Incomplete comparison')
    a=r.json(c['acquisition']['record']);require(r.digest(c['acquisition']['record'])==c['acquisition']['sha256'],'Changed acquisition')
    require(all(r.digest(n)==h for n,h in c['sources'].items()),'Changed capture/oracle/analysis source phase')
    require(all(n not in execution_sources or execution_sources[n]==h for n,h in c['sources'].items()),'Conflicting current execution source phases')
    execution_sources.update(c['sources'])
    observed=acquisition([artifact(r,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')],a['session_identity'])
    case_assess=assess
    if a.get('control_variant')=='source640-calibration-v1':
     from rendering_pause_low_assessment import assess as case_assess
    proof=case_assess(r,a,c,observed);require(proof==c['proof'],'Retained/raw pause proof differs')
    cases.append(dict(**proof['domain'],identity=a['session_identity'],pair=dict(acquisition=c['acquisition'],comparison=dict(record=name,sha256=r.digest(name))),
     verified=proof['verified'],dwells=proof['dwells'],cleanup=proof['cleanup'],pixels=c['comparison']['pixels_compared'],mismatching_pixels=c['comparison']['mismatching_pixels'],provider=a['resident']['renderer']))
   parent=r.json(args.parent);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
   sources=execution_sources|sources
   require(all(r.digest(n)==h for n,h in sources.items()),'Closeout sources changed')
   q=dict(schema=1,kind='public_campaign_mini_pause_closeout',success=True,sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},analysis_sources=sources,analysis_sources_stable=True,
    claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
    parent=dict(record=args.parent,sha256=r.digest(args.parent)),cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
    passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,actual_cases=4,pause_intervals=8,
    checked_pixels=sum(v['pixels'] for v in cases),mismatching_pixels=0,new_native_capture_policy=True,new_native_rendering_code=False,original_game_runs=14,primary_acceptance_original_game_runs=4,historical_successful_original_game_runs=2,failed_control_original_game_runs=8,new_analysis_original_game_runs=0,historical_failures_retired=False,
    source_phases=dict(inherited=dict(record=args.parent,sha256=r.digest(args.parent),source_pointer='/sources'),current_pause='/analysis_sources'),
    scope='Only the pending Mini pause row per profile, actual800/640. Preserves the frozen156-row inventory, all other accepted/pending decisions, exact historical sources and failures. No all-animation/simulation clock or music-playback freeze claim, arbitrary map/provider, resize, fault recovery or final native-rendering signoff.')
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
