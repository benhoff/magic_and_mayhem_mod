#!/usr/bin/env python3
"""Retain the boundary comparison tree omitted by the older domain path map."""
import argparse,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import safe,idle,scan,seal,MARKER
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh boundary comparison handoff required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));v=r.json(a.validation);require(v['success']and v['sources_stable']and v['family']=='boundary'and v['current_case_proved']and all(r.digest(n)==h for n,h in v['sources'].items()),'Current boundary validation required');ar=v['acquisition'];require(r.digest(ar['record'])==ar['sha256'],'Acquisition changed');ac=r.json(ar['record']);require(ac['success']and ac['private_wine_stopped']and ac['prefix_pool']['returned'],'Actual producer still active');cr=v['comparison'];require(r.digest(cr['record'])==cr['sha256'],'Comparison changed');c=r.json(cr['record']);require(c['success']and c['comparison_processes_stopped']and c['sources_stable']and c['original_manifest_verified_before_after'],'Incomplete comparison');refs=[cr]
  if c.get('native_edge_policy'):refs.append(c['native_edge_policy']['raw_original_comparison'])
  plans=[]
  for rr in refs:
   require(r.digest(rr['record'])==rr['sha256'],'Raw comparison changed');q=r.json(rr['record']);require(q['comparison_processes_stopped'],'Comparison consumer still active');path=Path(q['output_directory'])if q.get('output_directory')else ROOT/'working/viewport-cursor-comparisons'/Path(rr['record']).stem;safe(ROOT,path);idle(path);require(not(path/MARKER).exists(),'Comparison already handed off');members,identity,allocated=scan(path);require(sum(m.get('size',0)for m in members)<4*1024**3,'Boundary comparison real restore exceeds4GiBbudget');plans.append(dict(comparison=rr,path=str(path),members=members,identity=identity,allocated_bytes=allocated))
  vr=ref(a.validation)
 plan=dict(schema=1,kind='admitted_boundary_comparison_workspace_handoff_plan',success=True,original_game_runs=0,validation=vr,paths=plans,scope='Only the exact completed task-owned boundary comparison trees. Preserve every file, original pin, metadata, raw/positive phase and tested executable through two verified stores and a real restore before replacing with a pinned historical-path stub. No source checkout, capture, shared input or active consumer is removed.')
 name=a.report.removesuffix('.json')+'-plan.json';ph=write_report(ROOT,name,(json.dumps(plan,indent=2)+'\n').encode())
 with configured(ROOT)as pair:results=[seal(ROOT,Path(v['path']),pair,retire=True,restore_budget=4*1024**3)for v in plans]
 q=dict(schema=1,kind='admitted_boundary_comparison_workspace_handoff',success=True,original_game_runs=0,validation=vr,plan=dict(record=name,sha256=ph),results=results,acceptance_status_changed=False,source=dict(path=str(Path(__file__).relative_to(ROOT)),sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()));print(write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
