#!/usr/bin/env python3
"""Conjoin the unchanged B06 final gates with exact retained hotspot reanalysis."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_hotspot_analysis_matrix import P,specs,requests,current
from broader_player_acceptance import checked
from broader_player_sources import python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def assess(reader,operation,boundary=None,failure=None):
 name='check-broader-copy-failure-relevance'if operation=='failure'else'check-broader-player-acceptance-copy'
 path=ROOT/'tools'/(name+'.py');s=importlib.util.spec_from_file_location('B06_retained_hotspot_'+operation,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
 # The original gate and its predicates remain exact. Only current-case lookup
 # reads explicit new retained-analysis reports for the two reviewed captures.
 m.current=current
 q=m.assess(reader)if operation=='failure'else m.assess(reader,boundary,failure)
 ref=lambda n:dict(record=n,sha256=reader.digest(n));controls=ref(P+'B06-hotspot-analysis-controls-01.json');c=checked(reader,controls)
 require(c['tests']==93 and c['files']==2 and c['original_game_runs']==c['new_native_executions']==c['new_pixel_comparisons']==0 and c['sources_stable']and all(reader.digest(n)==h for n,h in c['sources'].items()),'Current retained hotspot refusal controls missing')
 rows,_=requests(ROOT,reader);values=[current(ROOT,reader,s,rows[s['case']['id']])for s in specs(reader)if s['case']['id']in('bp27-world-hotspot-default640-01','bp27-world-hotspot-default800-01')]
 require(len(values)==2 and all(v['analysis_revalidation']for v in values),'Both exact retained hotspot revalidations required')
 binding=dict(verified=True,original_game_runs=0,new_native_executions=0,additional_reservations=0,acquisitions_unchanged=True,old_requests_and_negative_reports_preserved=True,Native_source_waivers=[],controls=controls,cases=[dict(case=v['spec']['case']['id'],review=v['analysis_revalidation'],validation=v['validation'],acquisition=v['acquisition'])for v in values],scope='The collector could record a pre-cursor World paint. Fresh analysis binds the first complete captured same-World/scaling swap before the original readiness deadline, using original caller/COPY ordering and clocks. Full independent COPY and viewport comparisons, all three original readiness generations, physical input and retirement remain required. The recorded partial paint and failed analysis remain evidence; no new game or broader recovery claim follows.')
 names=python_sources(ROOT,['tools/check-broader-hotspot-analysis-conjunction.py','tools/broader_player_hotspot_analysis_matrix.py'])
 sources=q['sources']|c['sources']
 for n in names:
  h=reader.digest(n);require(n not in sources or sources[n]==h,'Conflicting retained analysis reader source');sources[n]=h
 require(all(reader.digest(n)==h for n,h in sources.items()),'Retained-analysis conjunction source drift')
 q.update(current_hotspot_readiness_analysis_revalidation=binding,sources=sources,sources_stable=True)
 return q

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('operation',choices=['failure','acceptance']);p.add_argument('--report',required=True);p.add_argument('--boundary-review');p.add_argument('--failure-review');a=p.parse_args()
 require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh conjunction report required');require(a.operation=='failure'or(a.boundary_review and a.failure_review),'Acceptance requires both final reviews')
 with Reader(ROOT)as r:
  try:q=assess(r,a.operation,a.boundary_review,a.failure_review)
  except(ValueError,KeyError,OSError,TypeError,StopIteration)as error:q=dict(schema=1,kind='broader_retained_hotspot_final_conjunction_refusal',success=False,original_game_runs=0,error=str(error),supported_player_accepted=False,original_recovery_complete=False)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'),supported_player_accepted=q.get('supported_player_accepted'),original_recovery_complete=False)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
