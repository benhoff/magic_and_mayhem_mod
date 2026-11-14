#!/usr/bin/env python3
"""Pair and real-restore one admitted stopped menu, World, or component case."""
import argparse,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import seal,scan,safe,idle,MARKER
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--report',required=True);args=p.parse_args()
 require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh domain handoff required')
 with Reader(ROOT)as r:
  v=r.json(args.validation);require(v['success']and v['sources_stable']and all(r.digest(n)==h for n,h in v['sources'].items()),'Current admitted domain required')
  ar=v['acquisition'];require(r.digest(ar['record'])==ar['sha256'],'Changed acquired domain');a=r.json(ar['record'])
  require(a['success']and a['private_wine_stopped']and a['prefix_pool']['returned']and not a.get('forced_private_prefix_stop',False),'Domain producer/prefix still active')
  key=Path(ar['record']).stem;kind=v['kind'];family=v['family'];paths=[]
  if kind=='current_broader_menu_case_validation':
   directory={'menu':'menu-acceptance','intros':'enabled-intros-acceptance','skip':'movie-skip-acceptance'}[family]
   paths=[ROOT/'working'/directory/key,Path(a['session'])]
  else:
   require(kind in('current_broader_world_case_validation','current_broader_component_case_validation')and v['current_case_proved'],'Unknown/non-current domain handoff')
   cr=v['comparison'];require(r.digest(cr['record'])==cr['sha256'],'Changed comparison');c=r.json(cr['record'])
   require(c['success'],'Failed comparison needs its own negative review')
   if kind=='current_broader_world_case_validation':require(c['comparison_processes_stopped'],'World comparison still running')
   paths=[Path(a['temporary_root']),Path(a['session'])]
   locations={'interaction':'interaction-comparisons','hotspot':'interaction-comparisons','viewport':'viewport-cursor-comparisons','picking':'picking-closeout-comparisons','pause':'pause-comparisons','caller':'caller-state-comparisons'}
   comparison=Path(c['output_directory'])if c.get('output_directory')else ROOT/'working'/locations[family]/Path(cr['record']).stem if family in locations else None
   if comparison is not None and comparison.is_relative_to(ROOT/'working'):paths.append(comparison)
   if v.get('native_edge_policy'):
    raw_ref=v['native_edge_policy']['raw_original_comparison'];require(r.digest(raw_ref['record'])==raw_ref['sha256'],'Changed raw comparison handoff identity');raw=r.json(raw_ref['record']);require(raw['comparison_processes_stopped'],'Raw comparison still active')
    raw_directory=Path(raw['output_directory'])if raw.get('output_directory')else ROOT/'working'/locations[family]/Path(raw_ref['record']).stem if family in locations else None
    if raw_directory is not None and raw_directory.is_relative_to(ROOT/'working')and raw_directory not in paths:paths.append(raw_directory)
  require(len(set(paths))==len(paths)and len(paths)>=2,'Unexpected domain root overlap')
  rows=[]
  for path in paths:
   safe(ROOT,path);idle(path);require(not(path/MARKER).exists(),'Domain already handed off');members,identities,allocated=scan(path);rows.append(dict(path=str(path),members=members,allocated_bytes=allocated))
  source_bytes={n:(h,r.read_bytes(n))for n,h in v['sources'].items()}
  require(all(hashlib.sha256(raw).hexdigest()==h for h,raw in source_bytes.values()),'Domain source preimage differs')
  vr=dict(record=args.validation,sha256=r.digest(args.validation))
 plan=dict(schema=1,kind='admitted_broader_domain_handoff_plan',success=True,original_game_runs=0,validation=vr,paths=rows,scope='Only these stopped task-owned execution/comparison paths. Original pins, all raw bytes and source preimages transfer after dual verification and real restore. Shared inputs, builds, prefix pools, source checkouts and external RAM materializations stay held.')
 name=args.report.removesuffix('.json')+'-plan.json';ph=write_report(ROOT,name,(json.dumps(plan,indent=2)+'\n').encode())
 with configured(ROOT)as pair:
  with pair.batch():snapshot={n:dict(sha256=pair.bytes(raw),size=len(raw))for n,(h,raw)in source_bytes.items()}
  sources=pair.root('broader_domain_analysis_sources',key,dict(schema=1,sources=snapshot))
  results=[seal(ROOT,path,pair,retire=True,restore_budget=16*1024**3)for path in paths]
 q=dict(schema=1,kind='admitted_broader_domain_handoff',success=True,original_game_runs=0,validation=vr,plan=dict(record=name,sha256=ph),analysis_sources=sources,results=results,external_RAM_files_removed=False,acceptance_status_changed=False)
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,trees=len(results))),flush=True)
if __name__=='__main__':main()
