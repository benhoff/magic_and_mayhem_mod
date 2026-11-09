#!/usr/bin/env python3
"""Close exactly two finite palette-refresh rows from four current native cases."""
import argparse,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
from rendering_pilot import require
from scene_effects import module
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/palette-cycling-results-20261108/'
PARENT='research/runtime/scene-master-merge-results-20261108/acceptance-02.json'
BID='NP.palette-cycling-closeout';SCENARIO='native.palette-cycling-closeout'
OBSERVER='NP.palette-production-observation';CONTROLS_SCENARIO='native.palette-production-observation-controls'
CONTROLS=PREFIX+'controls-12.json'

def analyze(reader,pairs):
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behavior=next(b for b in reg['behaviors']if b['id']==BID);builds={b['id']:b for b in reg['builds']};scenario=next(s for s in reg['scenarios']if s['id']==SCENARIO)
 observer=next(b for b in reg['behaviors']if b['id']==OBSERVER);control_scenario=next(s for s in reg['scenarios']if s['id']==CONTROLS_SCENARIO)
 controls=reader.json(CONTROLS);require(controls['success']and controls['sources_stable']and controls['exit_code']==0 and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current passive observer controls required')
 # These are the complete scientific Python modules actually used below;
 # storage readers verify immutable payloads independently of this closure.
 names=set(required_sources(behavior))|set(required_sources(observer))|{'tools/close-palette-cycling.py','tools/analyze-palette-cycle.py','tools/scene_effects.py','tools/rendering_menu_acceptance.py','tools/close-rendering-world-exit.py','tools/inspect-canvas-producers.py','tools/rendering_acceptance.py','tools/rendering_capture.py','tools/coverage_claims.py'}
 sources={n:reader.digest(n)for n in sorted(names)};packet=module('analyze-palette-cycle');cases=[]
 for acquisition,comparison,numerical in pairs:
  case=packet.analyze(reader,acquisition,comparison,numerical);require(case['success'],'Incomplete fresh palette output analysis')
  acquired=reader.json(acquisition);provider=acquired['resident']['renderer']
  require(('--software-rendering'in acquired['command'])is case['software'],'Palette software profile relabelled')
  if case['software']:require(provider.lower().startswith(('llvmpipe','softpipe')),'Palette software provider not observed')
  case['provider']=provider;cases.append(case)
  from palette_source_phase import execution_sources
  effective,_=execution_sources(reader,acquired,reader.json(comparison).get('palette_reader_source_review'),dict(record=acquisition,sha256=reader.digest(acquisition)))
  for phase in(effective,reader.json(comparison)['sources'],reader.json(numerical)['sources']):
   for n,h in phase.items():require(n not in sources or sources[n]==h,'Conflicting current palette source phase');sources[n]=h
 require(len(cases)==4 and {(c['software'],tuple(c['actual_extent']))for c in cases}=={(s,z)for s in(False,True)for z in((640,480),(800,600))}and len({c['identity']for c in cases})==4,'Missing/duplicate required palette profile/size')
 parent=reader.json(PARENT);require(parent['success']and parent['passed_requirements']==108 and parent['pending_requirements']==48,'Accepted108-row parent differs')
 rows=[];changed=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']=='operation.palette-cycling':
   require(row['state']=='pending','Palette row already accepted')
   selected=[c for c in cases if c['software']==(row['configuration']=='player-software')];require(len(selected)==2,'Missing actual palette contexts')
   row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],numerical=c['numerical'],actual_world_extents=[c['actual_extent']],reason=c['scope'])for c in selected]);changed.append((row['configuration'],row['id']))
  rows.append(row)
 require(len(changed)==2 and sum(r['state']=='passed'for r in rows)==110,'Palette closeout changed more than its two rows')
 with sqlite3.connect(ROOT/'working/palette-cycling/attempts.sqlite3')as db:charged=[dict(zip(('id','report','kind','started_ns','plan_sha256'),r))for r in db.execute('select * from attempts order by id')]
 require(len(charged)<=9,'Palette task exceeded its prospective attempt bound')
 actual_starts=sum(reader.json(row['report']).get('original_game_runs',0)for row in charged);require(actual_starts<=7,'Palette task exceeded actual-game bound')
 for case,(an,cn,nn)in zip(cases,pairs):
  aq=reader.json(an);matches=[row for row in charged if row['report']==an];require(len(matches)==1 and matches[0]['id']==aq['palette_attempt']and matches[0]['plan_sha256']==aq['input_plan']['sha256'],'Actual case lacks its charged source-plan binding')
 require(all(reader.digest(n)==h for n,h in sources.items()),'Palette closeout current source drift')
 drift={n:dict(accepted=h,current=reader.digest(n))for n,h in parent['sources'].items()if reader.digest(n)!=h}
 return dict(schema=1,kind='finite_actual_world_palette_refresh_closeout',success=True,task_complete=True,
  original_game_runs=0,new_native_engine_executions=0,new_original_oracle_executions=0,current_combined_player_proved=False,
  parent=dict(record=PARENT,sha256=reader.digest(PARENT)),cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
  closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{'operation.palette-cycling'}),passed_requirements=110,pending_requirements=46,newly_passed_requirements=2,
  completed_palette_rows=2,remaining_palette_rows=0,actual_contexts=4,checked_refresh_worlds=12,native_colour_samples=12,
  charged_attempts=charged,charged_actual_game_starts=actual_starts,historical_failures_erased=False,observer_controls=dict(record=CONTROLS,sha256=reader.digest(CONTROLS)),sources=sources,analysis_sources=sources,sources_stable=True,
  retained_parent_source_drift=drift,parent_statuses_preserved_in_recorded_source_phases=True,
  claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={sc['id']:scenario_contract(sc)})for b,sc in((behavior,scenario),(observer,control_scenario))],
  scope='Close only the two finite default/software palette-refresh operation rows. Actual original World-entry RGB/globals/coefficient/FP/profile inputs independently reproduce initial construction and existing-node WORD/DWORD rewrites, exact selected revisions join every owned World request and establish visible delivery in three advancing visible post-startup Worlds, changed fixed-index/shade production colours reach current complete native output, and original/native resources retire. All108accepted parent rows retain their recorded source phases; every source drift and failure is preserved. Initial unpresented constructor colours are comparison baselines, not prior native frames. Arbitrary periodic effects, broader exceptional/platform domains, live replacement of palette builders and both final player decisions remain separate.')

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--case',action='append',nargs=3,metavar=('ACQUISITION','COMPARISON','NUMERICAL'),required=True);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned palette closeout required');q=dict(schema=1,success=False)
 try:
  with Reader(ROOT)as r:q=analyze(r,a.case)
 except Exception as e:q['error']=str(e)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h,passed=q.get('passed_requirements'),pending=q.get('pending_requirements'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
