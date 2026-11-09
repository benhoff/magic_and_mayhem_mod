#!/usr/bin/env python3
"""Admit only two finite font/DC rows from four complete source-bound cases."""
import argparse,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
LEAF='operation.text-font-dc-lifetime'
BID='NP.installed-font-dc-acceptance';SID='native.installed-font-dc-acceptance'

def analyze(reader,parent_record,pairs):
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
 behavior=next(b for b in register['behaviors']if b['id']==BID);scenario=next(s for s in register['scenarios']if s['id']==SID)
 sources={n:reader.digest(n)for n in required_sources(behavior)|{'tools/close-font-lifetime.py','tools/coverage_claims.py'}}
 cases=[];identities=set()
 with sqlite3.connect(ROOT/'working/font-dc-attempts.sqlite3')as db:
  attempts=[dict(zip(('id','report','kind','started_ns','plan_sha256'),r))for r in db.execute('SELECT * FROM attempts ORDER BY id')]
 reserved=0
 for index,row in enumerate(attempts,1):
  result=reader.json(row['report']);starts=result.get('original_game_runs')
  require(type(starts)is int and starts>=0 or starts is None and result.get('charged_original_games')==1,'Unknown start lacks conservative reservation')
  reserved+=starts if type(starts)is int else 1
  if result.get('input_plan'):
   ref=result['input_plan'];plan=reader.json(ref['record'])
   require(reader.digest(ref['record'])==ref['sha256']==row['plan_sha256']and index<=plan['maximum_launch_attempts']and reserved<=plan['maximum_actual_game_starts'],'Historical request/start cap or plan identity violated')
 for acquisition,comparison in pairs:
  aq=reader.json(acquisition);cq=reader.json(comparison)
  require(aq['success']and aq['sources_stable']and aq['private_wine_stopped']and aq['original_manifest_verified_before_after'],'Incomplete normal acquisition')
  require(cq['success']and cq['sources_stable']and cq['comparison_processes_stopped']and cq['artifact_retention_complete']and cq['original_manifest_verified_before_after'],'Incomplete normal independent comparison')
  require(cq['acquisition']==dict(record=acquisition,sha256=reader.digest(acquisition))and cq['font_lifetimes_verified']and cq['independent_output_verified']and cq['native_menu_fonts']['verified']and cq['native_menu_fonts']['normal_retirement'],'Missing font/DC/visible native output binding')
  require(aq['scenario']=='normal-font-dc'and aq['retirement']['state']=='completed'and aq['admitted_original_sessions']==aq['original_game_runs']==1,'Not one normally retired original session')
  require(aq['session_identity']not in identities,'Duplicate original session');identities.add(aq['session_identity'])
  software=aq['configuration']['software_flag'];extent=list(map(int,aq['configuration']['requested_size'].split('x')))
  require(('--software-rendering'in aq['command'])is software and cq['world_domain']['extents']==[extent],'Profile/actual extent relabelled')
  provider=aq['resident']['renderer'];require(not software or provider.lower().startswith(('llvmpipe','softpipe')),'Software renderer not observed')
  require(all(r['mismatching_pixels']==0 for r in cq['comparison']['comparisons']),'Native pixels differ')
  matches=[r for r in attempts if r['report']==acquisition];require(len(matches)==1 and matches[0]['id']==aq['font_attempt']and matches[0]['plan_sha256']==aq['input_plan']['sha256'],'Case missing charged request')
  plan=reader.json(aq['input_plan']['record']);amendment=reader.json(plan['budget_amendment']['record'])
  require(amendment['authorized']and (plan['maximum_launch_attempts'],plan['maximum_actual_game_starts'])==(amendment['maximum_launch_attempts'],amendment['maximum_actual_game_starts'])and matches[0]['id']<=plan['maximum_launch_attempts'],'Final batch exceeds case prospective allowance')
  for phase in (aq['sources'],cq['sources']):
   for name,h in phase.items():require(reader.digest(name)==h and (name not in sources or sources[name]==h),'Current font source phase drift: '+name);sources[name]=h
  cases.append(dict(software=software,actual_extent=extent,identity=aq['session_identity'],provider=provider,pair=dict(acquisition=dict(record=acquisition,sha256=reader.digest(acquisition)),comparison=dict(record=comparison,sha256=reader.digest(comparison))),font_dc=cq['font_dc_lifetime'],native_fonts=cq['native_menu_fonts'],pixels=sum(r['pixels']for r in cq['comparison']['comparisons']),presented_comparisons=len(cq['comparison']['comparisons'])))
 require(len(cases)==4 and {(c['software'],tuple(c['actual_extent']))for c in cases}=={(s,z)for s in(False,True)for z in((640,480),(800,600))},'Four distinct actual font contexts required')
 parent=reader.json(parent_record);require(parent['success']and len(parent['requirements'])==156,'Accepted parent dossier differs')
 rows=[];changed=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Font row already admitted')
   chosen=[c for c in cases if c['software']==(row['configuration']=='player-software')];require(len(chosen)==2,'Two actual sizes missing')
   row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],actual_world_extents=[c['actual_extent']],reason='Source-matched original SFT load/use/reload/teardown and explicit retained replacements; actual native Qt caption/registration/cache/release before and after reload; complete independent World/viewport output and separate real DC custody. Native outline/AA policy, undrawn yellow and original hidden menus remain explicit.')for c in chosen]);changed.append((row['configuration'],row['id']))
  rows.append(row)
 require(len(changed)==2 and sum(r['state']=='passed'for r in rows)==parent['passed_requirements']+2,'Closeout changed unrelated rows')
 require(all(reader.digest(n)==h for n,h in sources.items()),'Closeout source drift')
 drift={n:dict(accepted=h,current=reader.digest(n)if(ROOT/n).exists()else None)for n,h in parent['sources'].items()if not(ROOT/n).exists()or reader.digest(n)!=h}
 return dict(schema=1,kind='finite_font_dc_lifetime_closeout',success=True,task_complete=True,original_game_runs=0,current_combined_player_proved=False,parent=dict(record=parent_record,sha256=reader.digest(parent_record)),cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,newly_passed_requirements=2,charged_attempts=attempts,conservatively_reserved_original_starts=reserved,historical_failures_erased=False,sources=sources,analysis_sources=sources,sources_stable=True,retained_parent_source_drift=drift,parent_statuses_preserved_in_recorded_source_phases=True,claims=[dict(behavior=BID,contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={SID:scenario_contract(scenario)})],scope='Exactly two finite default/software font/DC operation rows at actual640/800. Native Qt SFT outlines/AA remain intentional policy; no original UI pixel identity, presented World SFT text, original reload leak-free behavior, arbitrary platform/caller/exceptional ownership or final rendering decision. Every historical source phase and failed request remains retained.')

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',required=True);p.add_argument('--case',action='append',nargs=2,required=True);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned immutable closeout required')
 try:
  with Reader(ROOT)as r:q=analyze(r,a.parent,a.case)
 except Exception as e:q=dict(schema=1,success=False,error=str(e))
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h,passed=q.get('passed_requirements'),pending=q.get('pending_requirements'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
