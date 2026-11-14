#!/usr/bin/env python3
"""Freeze the publication-only continuation after complete no-game preflight."""
import argparse,copy,hashlib,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_bookkeeping import atomic_replace
from evidence_objects import configured
from broader_player_sources import python_sources
from broader_player_viewport_inputs_matrix import specs as previous_specs
from broader_publication_source_budget import bound,predecessors
from broader_player_publication_matrix import specs,current
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'

class Proposed:
 def __init__(self,reader):self.reader=reader;self.reports={};self.physical={}
 def add(self,name,value):
  body=(json.dumps(value,indent=2)+'\n').encode();require(name not in self.reports,'Duplicate proposed report');self.reports[name]=body;return dict(record=name,sha256=hashlib.sha256(body).hexdigest())
 def digest(self,n):
  body=self.reports.get(n,self.physical.get(n));return hashlib.sha256(body).hexdigest()if body is not None else self.reader.digest(n)
 def json(self,n):
  body=self.reports.get(n,self.physical.get(n));return json.loads(body)if body is not None else self.reader.json(n)
 def read_bytes(self,n):return self.reports[n]if n in self.reports else self.physical[n]if n in self.physical else self.reader.read_bytes(n)

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--attempt',required=True);args=parser.parse_args();require(args.attempt.isdecimal(),'Numeric attempt required')
 report=P+'publication-preparation-attempt-'+args.attempt+'.json';require(not(ROOT/report).exists(),'Fresh preflight result required')
 result=dict(schema=1,kind='broader_publication_phase_preparation',success=False,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0)
 try:
  with Reader(ROOT)as r:
   ref=lambda n:dict(record=n,sha256=r.digest(n));draft=Proposed(r)
   with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
    db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns,case_id')]
   require(len(rows)==37 and sum(v['reserved_games']for v in rows)==69,'Stopped request set changed')
   by_id={v['case_id']:v for v in rows};declared=previous_specs(r);retained=[s for s in declared if s['case']['id']in by_id and by_id[s['case']['id']]['state']=='admitted'];remaining=[s for s in declared if s not in retained]
   require(len(retained)==11 and len(remaining)==79 and all(s['role']not in('fault','boundary')for s in remaining),'Unexpected continuation domain')
   failure='bp16-strict-default30-01';require(by_id[failure]['state']=='failed'and by_id[failure]['reserved_games']==1,'Missing preserved strict refusal')
   require(all(s['case']['id']not in by_id or s['case']['id']==failure for s in remaining),'Other requested case needs its own review')
   ids={s['case']['id']:s['case']['id'].replace('bp16-','bp17-',1)for s in remaining};require(len(set(ids.values()))==79,'Duplicate future identities')
   build=ref(P+'build-current-05.json');b=r.json(build['record']);require(b['success']and b['sources_stable']and all(r.digest(n)==h for n,h in(b['compiler_sources']|b['runtime_sources']).items()),'Native/compiler/runtime changed')
   require(hashlib.sha256((ROOT/'working/build/qt-shell/mnm-qt-shell').read_bytes()).hexdigest()==b['player']['sha256'],'Current player changed')
   control_names=['native-controls-06','isolated-tooling-controls-10','parallel-command-controls-03','parallel-viewport-controls-01','viewport-command-history-controls-01','viewport-legacy-history-controls-02','drain-admission-controls-02','input-reference-controls-01']
   controls=[ref(P+n+'.json')for n in control_names]
   for cr in controls:
    q=r.json(cr['record']);require(q['success']and q.get('sources_stable',True)and all(r.digest(n)==h for n,h in q.get('sources',{}).items()),'Current controls changed: '+cr['record'])
   app=ref(P+'completed-journal-publication-applicability-01.json');ap=r.json(app['record']);require(ap['success']and ap['retained_current_cases']==11 and ap['native_files_waived']==[],'Completed publication review missing')
   reviewer=['tools/prepare-broader-publication-phase.py','tools/broader_publication_source_budget.py','tools/run-broader-publication-source-case.py','tools/run-broader-publication-domain-case.py','tools/run-broader-publication-strict-case.py','tests/test-broader-publication-source-budget.py','tools/owned-menu-game-runner.py','tools/owned_session_journal.py']
   review=dict(schema=1,kind='broader_journal_publication_continuation_review',success=True,original_game_runs=0,old_reserved_starts=69,retained_current_cases=11,remaining_positive_cases=79,remaining_positive_original_starts=79,new_additional_ceiling=148,new_combined_ceiling=156,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,manually_reviewed_replacement_starts=1,changed_compiler_files=[],changed_runtime_files=[],game_response_deadlines_changed=False,reviewer_sources={n:r.digest(n)for n in reviewer},current_build=build,current_controls=controls,old_requests=rows,new_case_ids=sorted(ids.values()),publication_applicability=app,failed_case_custody=ref(P+failure+'-journal-custody-01.json'),parent_global_plan=ref(P+'plan-16.json'),first_pilot=ids[failure],reason='One manually reviewed replacement after the strict observer encountered JSON parsing before session assignment. The publisher formerly exposed an empty/partial final-name journal while json.dump was in progress; the exact live read/stack remains uncaptured. Atomic exclusive publication preserves complete JSON bytes, namespace/admission assertions and launch order, with228isolated controls. Preserve all37old rows/69charges including the failed one; keep eleven complete B05Native proofs, with exact publication-only retained-data applicability for only three Python-dependent records. Every Native/compiler/runtime file and player stays exact. The remaining79positive cases need148additional/156combined reservations, with no spare, automatic retry or refund. First qualify the strict30replacement, then expand under unchanged actions, durations, deadlines, providers, independent comparisons and retention. This is an agent-set finite bound, not a user numeric approval or Original-recovery promotion.')
   rr=draft.add(P+'publication-source-review-01.json',review);g=copy.deepcopy(r.json(P+'plan-16.json'));g['historical_viewport_source_review']=g.pop('viewport_source_review')
   g.update(kind='broader_current_player_publication_plan',previous_plan=review['parent_global_plan'],journal_publication_review=rr,publication_applicability=app,current_build=build,current_player_binary_sha256=b['player']['sha256'],maximum_additional_original_games=148,maximum_combined_original_games=156,maximum_combined_WP10_and_new_original_games=156,minimum_final_additional_reservations=148,retained_predecessor_reservations=69,retained_current_cases=11,remaining_positive_cases=79,remaining_positive_original_starts=79,maximum_manually_reviewed_additional_repairs=1,request_driver='tools/run-broader-publication-source-case.py',scope=review['reason'])
   gr=draft.add(P+'plan-17.json',g)
   require(bound(draft,g)==148 and predecessors(draft,rows,g)==69,'Finite publication guard refused')
   old_names=sorted({s['plan']for s in remaining});names={n:n.removesuffix('.json')+'-publication-17.json'for n in old_names};require(len(names)==21,'Changed prospective phase count')
   physical_map={};ref_map={}
   for old_path,label in [('working/broader-player/strict-claims-16.json','strict'),('working/broader-player/bp16/world-caller/comparison-claims.json','caller')]:
    q=json.loads((ROOT/old_path).read_bytes());require([n for n,h in q['sources'].items()if r.digest(n)!=h]==['tools/owned-menu-game-runner.py'],'Unexpected claims-input change');q['sources']={n:r.digest(n)for n in (set(q['sources'])|{'tools/owned_session_journal.py'}|python_sources(ROOT,[n for n in set(q['sources'])|{'tools/owned_session_journal.py'}if n.endswith('.py')]))};new_path='working/broader-player/publication-17-'+label+'-claims.json';raw=(json.dumps(q,indent=2)+'\n').encode();draft.physical[new_path]=raw;physical_map[old_path]=new_path;name=P+'publication-'+label+'-claims-17.bin';draft.reports[name]=raw;newref=dict(record=name,sha256=hashlib.sha256(raw).hexdigest())
    old_record=P+'strict-source-declaration-16.json'if label=='strict'else'research/runtime/caller-state-installed-results-20261108/bp16/world-caller/comparison-claims-source-inputs-02.bin';ref_map[old_record]=newref
   def adapt(v):
    if isinstance(v,dict):
     if v.get('record')in ref_map:return copy.deepcopy(ref_map[v['record']])|{k:adapt(x)for k,x in v.items()if k not in('record','sha256')}
     return {k:adapt(x)for k,x in v.items()}
    if isinstance(v,list):return [adapt(x)for x in v]
    if isinstance(v,str):
     if v in names:return names[v]
     if v in physical_map:return physical_map[v]
     for old_id,new_id in ids.items():v=v.replace(old_id,new_id)
     return v
    return v
   bindings=[dict(case_id=s['case']['id'],plan=ref(s['plan']),role=s['role'],family=s['family'],retained=True)for s in retained];phase_refs=[];source_names=set()
   for old_name in old_names:
    original=r.json(old_name);phase=adapt(original);old_cases={s['case']['id']for s in remaining if s['plan']==old_name};phase['broader_cases']=[adapt(c)for c in original['broader_cases']if c['id']in old_cases]
    phase.update(broader_plan=gr,request_driver='tools/run-broader-publication-source-case.py',supersedes_prospective_plan=ref(old_name),publication_applicability=app,publication_controls=ref(P+'isolated-tooling-controls-10.json'))
    phase['sources']={n:r.digest(n)for n in (set(original['sources'])|set(reviewer)|{'tools/owned_session_journal.py'}|python_sources(ROOT,[n for n in set(original['sources'])|set(reviewer)|{'tools/owned_session_journal.py'}if n.endswith('.py')]))};source_names.update(phase['sources'])
    for n,h in original['compiler_sources'].items():require(r.digest(n)==h,'Compiler source changed')
    for n,h in original['runtime_sources'].items():require(r.digest(n)==h,'Runtime source changed')
    if 'maximum_new_original_starts'in phase:phase['maximum_new_original_starts']=len(phase['broader_cases'])
    if 'actual_context_analysis_plan'in phase:phase['actual_context_analysis_plan']=P+'strict-extent-analysis-plan-17.json'
    pr=draft.add(names[old_name],phase);phase_refs.append(pr)
    for s in remaining:
     if s['plan']==old_name:
      c=next(v for v in phase['broader_cases']if v['id']==ids[s['case']['id']]);require(all(c[k]==s['case'][k]for k in('profile','provider','size','reserved_original_games'))and c['reserved_original_games']==1,'Changed finite context');bindings.append(dict(previous_case_id=s['case']['id'],case_id=c['id'],plan=pr,role=s['role'],family=s['family'],retained=False))
   analysis={}
   for stem in('native-edge','world-native-edge','boundary-native-edge'):
    n=P+stem+'-analysis-plan-16.json';q=copy.deepcopy(r.json(n));require(all(r.digest(k)==h for k,h in q['sources'].items()),'Oracle source changed');q.update(global_plan=gr,previous_analysis_plan=ref(n));analysis[stem]=draft.add(P+stem+'-analysis-plan-17.json',q)
   strict=next(s for s in remaining if s['role']=='strict');old_strict=r.json(strict['plan'])['actual_context_analysis_plan'];q=copy.deepcopy(r.json(old_strict));q.update(strict_phase=next(v for v in phase_refs if v['record']==names[strict['plan']]),previous_analysis_plan=ref(old_strict));q['sources']={n:r.digest(n)for n in q['sources']};analysis['strict-extent']=draft.add(P+'strict-extent-analysis-plan-17.json',q)
   catalog=dict(schema=1,kind='broader_publication_source_catalog',success=True,original_game_runs=0,global_plan=gr,review=rr,publication_applicability=app,current_build=build,retained_current_cases=11,remaining_positive_cases=79,total_positive_cases=90,total_positive_original_starts=98,native_source_waivers=[],case_bindings=bindings,analysis_plans=analysis,prospective_phases=phase_refs,first_pilot=ids[failure],source_preimages_pending=True,global_player_accepted=False)
   cn=P+'publication-source-catalog-01.json';draft.add(cn,catalog)
   planned=specs(draft);require(len(planned)==90 and len({v['case']['id']for v in planned})==90,'Lost a current case')
   for s in planned:
    cid=s['case']['id'];phase=draft.json(s['plan'])
    if cid in by_id and by_id[cid]['state']=='admitted':current(ROOT,draft,s,by_id[cid]);continue
    require(phase['broader_plan']==gr and phase['request_driver']=='tools/run-broader-publication-source-case.py','Wrong request binding')
    require(all(draft.digest(n)==h for n,h in phase['sources'].items()),'Prospective source drift')
    require(s['plan']in s['case']['command']and s['case']['report']in s['case']['command'],'Foreign collector plan/report')
   def references(v):
    if isinstance(v,dict):
     if 'record'in v and 'sha256'in v:require(draft.digest(v['record'])==v['sha256'],'Changed child reference: '+v['record'])
     for x in v.values():references(x)
    elif isinstance(v,list):
     for x in v:references(x)
   for pr in phase_refs:references(draft.json(pr['record']))
   require(not(ROOT/cn).exists()and not(ROOT/P/'plan-17.json').exists(),'Publication phase already exists')
   source_bytes={n:(ROOT/n).read_bytes()for n in source_names}
  with configured(ROOT)as pair:
   with pair.batch():sr={n:dict(sha256=pair.bytes(body),size=len(body))for n,body in source_bytes.items()};inputs={n:dict(sha256=pair.bytes(body),size=len(body))for n,body in draft.physical.items()}
   source_root=pair.root('broader_publication_phase_sources',ROOT,dict(schema=1,sources=sr,authored_inputs=inputs))
  catalog.pop('source_preimages_pending');catalog['source_preimages']=source_root;draft.reports[cn]=(json.dumps(catalog,indent=2)+'\n').encode()
  for n,body in draft.physical.items():
   path=ROOT/n
   if path.exists():require(path.read_bytes()==body,'Authored input changed')
   else:atomic_replace(path,body,None)
  for n,body in draft.reports.items():require(write_report(ROOT,n,body)==hashlib.sha256(body).hexdigest(),'Report payload changed')
  result.update(success=True,catalog=dict(record=cn,sha256=hashlib.sha256(draft.reports[cn]).hexdigest()),global_plan=gr,review=rr,retained_current_cases=11,remaining_positive_cases=79,source_preimages=source_root,all_child_refs_and_sources_verified=True,first_pilot=ids[failure])
 except Exception as error:
  import traceback
  traceback.print_exc();result['error']=repr(error)
 op=report.removesuffix('.json')+'-operator.bin';result['operator']=dict(record=op,sha256=write_report(ROOT,op,Path(__file__).read_bytes()));h=write_report(ROOT,report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))),flush=True);return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
