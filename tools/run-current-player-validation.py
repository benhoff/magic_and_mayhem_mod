#!/usr/bin/env python3
"""Run one bounded current-build public-player case; require reviewed predecessors."""
import argparse,hashlib,importlib.util,json,sqlite3,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/whole-player-validation-results-20261110/'
NORMAL='research/runtime/rendering-normal-results-20261031/'

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--scenario',choices=('quick','storage'),required=True);p.add_argument('--software',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(NORMAL+'wp10-')and a.report.endswith('-acquisition.json')and not(ROOT/a.report).exists(),'Fresh owned current-player acquisition required')
 with Reader(ROOT)as r:
  plan=r.json(P+'plan-01.json');built=r.json(P+'build-02.json');require(plan['success']is built['success']is True,'Successful owned plan/build required')
  require(all(r.digest(n)==h for n,h in plan['sources'].items()),'Prospective source/control plan changed')
  names=native_runtime_source_paths(ROOT)|{'tools/run-current-player-validation.py'}
  before={n:r.digest(n)for n in sorted(names)};plan_ref=dict(record=P+'plan-01.json',sha256=r.digest(P+'plan-01.json'))
  build_ref=dict(record=P+'build-02.json',sha256=r.digest(P+'build-02.json'))
 db=sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3');db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY,scenario TEXT,software INTEGER,started_ns INTEGER,state TEXT,result_sha256 TEXT,comparison TEXT)')
 db.execute('BEGIN IMMEDIATE');prior=list(db.execute('SELECT report,state FROM requests'))
 require(len(prior)<4 and all(state=='compared'for _,state in prior),'Uncompared/failed predecessor or four-start budget exhausted')
 db.execute('INSERT INTO requests VALUES(?,?,?,?,?,NULL,NULL)',(a.report,a.scenario,int(a.software),time.time_ns(),'requested'));db.commit()
 spec=importlib.util.spec_from_file_location('current_player_collector',ROOT/'tools/run-rendering-acceptance-pilot.py');collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
 original_writer=collector.write_report
 def writer(root,path,raw):
  if path==a.report:
   q=json.loads(raw);stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in before.items())
   q.setdefault('sources',{}).update(before);q.update(native_runtime_sources=sorted(native_runtime_source_paths(ROOT)),current_player_plan=plan_ref,current_player_build=build_ref,
    current_player_scenario=a.scenario,current_player_profile='player-software'if a.software else'player-default',runtime_sources_stable=stable)
   q['success']=q['success']and stable;q['sources_stable']=q.get('sources_stable',True)and stable
   raw=(json.dumps(q,indent=2)+'\n').encode()
   sha=original_writer(root,path,raw)
   db.execute('UPDATE requests SET state=?,result_sha256=? WHERE report=?',('acquired'if q['success']else'failed',sha,path));db.commit();return sha
  return original_writer(root,path,raw)
 collector.write_report=writer
 argv=['--scenario','normal-quick'if a.scenario=='quick'else'normal-storage','--report',a.report]
 if a.scenario=='quick':argv.append('--gameplay-witness')
 if a.software:argv.append('--software')
 sys.argv=[str(ROOT/'tools/run-rendering-acceptance-pilot.py'),*argv]
 try:return collector.main()
 finally:db.close()
if __name__=='__main__':raise SystemExit(main())
