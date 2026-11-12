#!/usr/bin/env python3
"""Run two actual low Worlds from an owned verified input, after the six starts."""
import argparse,copy,hashlib,importlib.util,json,sqlite3,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/whole-player-validation-results-20261110/'
N='research/runtime/rendering-normal-results-20261031/'


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--software',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
 label='software-low-02'if a.software else'default-low-02'
 require(a.report==N+'wp10-'+label+'-acquisition.json'and not(ROOT/a.report).exists(),'Fresh corrected low fixture case required')
 with Reader(ROOT)as r:
  plan=r.json(P+'plan-03.json');fixture=r.json(P+'low-fixture-03.json');require(plan['success']and fixture['success'],'Verified prospective corrected fixture required')
  require(all(r.digest(n)==h for n,h in plan['sources'].items()),'Corrected fixture controls changed')
  require((ROOT/'working/game-nocd').resolve()==Path(fixture['owned_game']),'Owned low source was not selected')
  for name in fixture['changed_files']:
   require(hashlib.sha256((ROOT/'working/game-nocd'/name).read_bytes()).hexdigest()==fixture['after_inputs'][name]['sha256'],'Selected low preference bytes changed')
  names=native_runtime_source_paths(ROOT)|{'tools/run-current-player-low-fixture.py'}
  before={n:r.digest(n)for n in sorted(names)}
  plan_ref=dict(record=P+'plan-03.json',sha256=r.digest(P+'plan-03.json'));fixture_ref=dict(record=P+'low-fixture-03.json',sha256=r.digest(P+'low-fixture-03.json'))
  build_ref=dict(record=P+'build-02.json',sha256=r.digest(P+'build-02.json'))
 db=sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3');db.execute('BEGIN IMMEDIATE')
 prior=list(db.execute('SELECT report,state FROM requests'))
 require(6<=len(prior)<8 and all(state=='compared'for _,state in prior),'Eight-start total cap or unreviewed predecessor')
 require(not any(('software-low-02'if a.software else'default-low-02')in name for name,_ in prior),'Corrected low profile already attempted')
 db.execute('INSERT INTO requests VALUES(?,?,?,?,?,NULL,NULL)',(a.report,'quick-low-fixture',int(a.software),time.time_ns(),'requested'));db.commit()
 spec=importlib.util.spec_from_file_location('corrected_low_collector',ROOT/'tools/run-rendering-acceptance-pilot.py');collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
 original_writer=collector.write_report;checkpointed=False
 def augmented(q):
  stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in before.items())
  q.setdefault('sources',{}).update(before);q.update(native_runtime_sources=sorted(native_runtime_source_paths(ROOT)),current_player_plan=plan_ref,current_player_build=build_ref,current_player_scenario='quick-low-fixture',current_player_profile='player-software'if a.software else'player-default',runtime_sources_stable=stable)
  q['configuration'].update(source_game_fixture=fixture_ref,requested_world_extent=[640,480],preferences_import_not_required=True)
  q['success']=q['success']and stable;q['sources_stable']=q.get('sources_stable',True)and stable
  return q
 def writer(root,path,raw):
  nonlocal checkpointed
  if not checkpointed and path.startswith(a.report.removesuffix('.json')+'-raw-'):
   frame=sys._getframe(1)
   require(Path(frame.f_code.co_filename)==ROOT/'tools/run-rendering-acceptance-pilot.py'and'report'in frame.f_locals,'Pinned collector retention checkpoint site changed')
   q=augmented(copy.deepcopy(frame.f_locals['report']));q.update(original_execution_succeeded=q['success'],success=False,retention_state='pending')
   original_writer(ROOT,P+label+'-pre-retention-metadata.json',(json.dumps(q,indent=2)+'\n').encode());checkpointed=True
  if path==a.report:
   q=augmented(json.loads(raw));q['retention_checkpoint']=dict(record=P+label+'-pre-retention-metadata.json')
   sha=original_writer(root,path,(json.dumps(q,indent=2)+'\n').encode())
   db.execute('UPDATE requests SET state=?,result_sha256=? WHERE report=?',('acquired'if q['success']else'failed',sha,path));db.commit();return sha
  return original_writer(root,path,raw)
 collector.write_report=writer
 sys.argv=[str(ROOT/'tools/run-rendering-acceptance-pilot.py'),'--scenario','normal-quick','--report',a.report]+(['--software']if a.software else[])
 try:return collector.main()
 finally:db.close()

if __name__=='__main__':raise SystemExit(main())
