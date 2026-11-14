#!/usr/bin/env python3
"""Add the exact inherited input reference to unstarted declarations only."""
import copy,hashlib,json,sqlite3,subprocess,sys
from contextlib import closing
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_worker_matrix import continued_specs,P
from broader_input_plan_inheritance import INCOMPLETE,PARENT,resolve
from broader_drained_source_budget import bound
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];CAT=P+'input-complete-continuation-catalog-01.json';GLOBAL=P+'plan-15.json';ACTIVE='bp13-transition-default800-02'
def main():
 require(not(ROOT/CAT).exists()and not(ROOT/GLOBAL).exists(),'Never overwrite completion metadata')
 control=subprocess.run([sys.executable,'-B','tests/test-continuation-input-binding.py'],cwd=ROOT,capture_output=True);require(control.returncode==0,control.stderr.decode())
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));fixture,inheritance=resolve(r,INCOMPLETE);g=copy.deepcopy(r.json(INCOMPLETE['record']));require(bound(r,g)==130,'Continuation bound changed');g.update(owned_input=fixture,previous_input_incomplete_plan=INCOMPLETE,input_metadata_completion=inheritance)
  with closing(sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3'))as db:
   db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests')]
  ids={v['case_id']for v in rows};require(ACTIVE in ids and len(rows)==22 and sum(v['reserved_games']for v in rows)==44,'Review exact in-flight predecessor set');future=[s for s in continued_specs(r)if s['case']['id']not in ids];require(len(future)==84,'Only84unstarted positive contexts may be rebound')
  names=['tools/complete-broader-drain-input-plan.py','tools/broader_input_plan_inheritance.py','tools/check-broader-inherited-fault.py','tests/test-continuation-input-binding.py'];sources={n:r.digest(n)for n in names};cn=P+'input-reference-controls-01.json';cv=dict(schema=1,kind='broader_input_reference_inheritance_controls',success=True,original_game_runs=0,tests=7,sources=sources,sources_stable=True,stdout=control.stdout.decode(),stderr=control.stderr.decode(),input_resolution=inheritance,global_player_accepted=False);ch=write_report(ROOT,cn,(json.dumps(cv,indent=2)+'\n').encode())
  gh=write_report(ROOT,GLOBAL,(json.dumps(g,indent=2)+'\n').encode());gr=dict(record=GLOBAL,sha256=gh);phase_map={};bindings=[]
  for old_name in sorted({s['plan']for s in future}):
   phase=copy.deepcopy(r.json(old_name));new_name=old_name.removesuffix('.json')+'-input-complete-01.json';phase_map[old_name]=new_name;phase['broader_plan']=gr;phase['supersedes_unrequested_plan']=ref(old_name);phase['input_reference_completed_without_new_game']=True
   for case in phase['broader_cases']:
    for field in ['command','comparison_command']:
     if field in case:case[field]=[new_name if x==old_name else x for x in case[field]]
   if 'actual_context_analysis_plan'in phase:phase['actual_context_analysis_plan']=P+'strict-extent-analysis-plan-15.json'
   h=write_report(ROOT,new_name,(json.dumps(phase,indent=2)+'\n').encode());nr=dict(record=new_name,sha256=h)
   bindings.extend(dict(case_id=s['case']['id'],previous_plan=ref(old_name),plan=nr)for s in future if s['plan']==old_name)
  analyses={}
  for stem in ['native-edge','world-native-edge','boundary-native-edge']:
   old_name=P+stem+'-analysis-plan-14.json';v=copy.deepcopy(r.json(old_name));v['global_plan']=gr;v['previous_analysis_plan']=ref(old_name);new_name=P+stem+'-analysis-plan-15.json';h=write_report(ROOT,new_name,(json.dumps(v,indent=2)+'\n').encode());analyses[stem]=dict(record=new_name,sha256=h)
  n=P+'strict-extent-analysis-plan-14.json';v=copy.deepcopy(r.json(n));old_phase=v['strict_phase']['record'];new_phase=phase_map[old_phase];v['strict_phase']=dict(record=new_phase,sha256=next(x['plan']['sha256']for x in bindings if x['plan']['record']==new_phase));v['previous_analysis_plan']=ref(n);n=P+'strict-extent-analysis-plan-15.json';h=write_report(ROOT,n,(json.dumps(v,indent=2)+'\n').encode());analyses['strict-extent']=dict(record=n,sha256=h)
  q=dict(schema=1,kind='broader_input_complete_continuation_catalog',success=True,original_game_runs=0,previous_catalog=ref(P+'drain-continuation-catalog-01.json'),global_plan=gr,case_bindings=bindings,unstarted_cases=84,pending_acquisition_preserved=ACTIVE,source_case_or_input_changed=False,native_source_or_binary_changed=False,ceiling_changed=False,original_reports_changed=False,controls=dict(record=cn,sha256=ch),analyses=analyses,source_review=sources,scope='The retained plan14omitted its inherited owned_input reference. Plan15binds exactly the hash-verified plan13fixture. Only84unstarted declarations change their global metadata and analysis-plan references; all actions, sources, tested binaries, output/input assertions, budgets and suppliers stay exact. The in-flight replacement keeps its original plan14identity and requires separately recorded pinned-parent retained-data validation. No new game, acceptance or source waiver follows.')
  require(len(bindings)==84,'Missing unstarted binding');h=write_report(ROOT,CAT,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,catalog_sha256=h,global_plan_sha256=gh,controls_sha256=ch,original_game_runs=0,unstarted_cases=84)),flush=True)
if __name__=='__main__':main()
