#!/usr/bin/env python3
"""Keep acquisition discovery disjoint without changing native case contracts."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_discovery_paths import rebind
PREFIX='research/runtime/example/bp29/'
def phase():
 a=PREFIX+'case-acquisition.json';c=PREFIX+'case-comparison-01.json';p=PREFIX+'case-preparation-01.json'
 return dict(collector_prefix=PREFIX,bounds=dict(maximum_requests=6,maximum_original_games=5,stop_on_first_failure=True),broader_cases=[dict(id='case',report=a,comparison=c,prepare_progression=p,reserved_original_games=1,profile='player-software',size=[800,600],command=['python3','tools/collector.py','--report',a,'--software'],comparison_command=['python3','tools/comparer.py','--acquisition',a,'--report',c],historical_primary=dict(acquisition=dict(record='research/runtime/older/original.json',sha256='old')))],counter_bindings={'/root/old.sqlite3':'working/broader-player/bp40/example/attempts.sqlite3'})
class Namespace(unittest.TestCase):
 def test_source_immutable(self):
  p=phase();before=copy.deepcopy(p);rebind(p);self.assertEqual(p,before)
 def test_all_prospective_outputs_and_commands(self):
  p=phase();q=rebind(p);c=q['broader_cases'][0];self.assertEqual(q['collector_prefix'],PREFIX+'continuation-43/');self.assertEqual(c['report'],q['collector_prefix']+'case-acquisition.json');self.assertEqual(c['command'][3],c['report']);self.assertEqual(c['comparison_command'][3],c['report']);self.assertEqual(c['comparison_command'][5],c['comparison']);self.assertEqual(c['prepare_progression'],q['collector_prefix']+'case-preparation-01.json')
 def test_case_semantics_and_bounds_exact(self):
  p=phase();q=rebind(p);self.assertEqual(q['bounds'],p['bounds']);self.assertEqual({k:q['broader_cases'][0][k]for k in ['id','profile','size','reserved_original_games','historical_primary']},{k:p['broader_cases'][0][k]for k in ['id','profile','size','reserved_original_games','historical_primary']})
 def test_fresh_counter_namespace(self):self.assertEqual(rebind(phase())['counter_bindings'],{'/root/old.sqlite3':'working/broader-player/bp43/example/attempts.sqlite3'})
 def test_unsafe_prefix(self):
  p=phase();p['collector_prefix']='research/runtime/../old/'
  with self.assertRaises(ValueError):rebind(p)
 def test_unbound_output(self):
  p=phase();p['broader_cases'][0]['comparison']='research/runtime/other/report.json'
  with self.assertRaises(ValueError):rebind(p)
 def test_nested_output(self):
  p=phase();p['broader_cases'][0]['report']=PREFIX+'nested/report.json'
  with self.assertRaises(ValueError):rebind(p)
 def test_duplicate_acquisition(self):
  p=phase();p['broader_cases'].append(copy.deepcopy(p['broader_cases'][0]))
  with self.assertRaises(ValueError):rebind(p)
 def test_foreign_counter(self):
  p=phase();p['counter_bindings']['/root/old.sqlite3']='working/unreviewed.sqlite3'
  with self.assertRaises(ValueError):rebind(p)
 def test_double_start_context_refused(self):
  p=phase();p['broader_cases'][0]['reserved_original_games']=2
  with self.assertRaises(ValueError):rebind(p)
class OriginalAdmission(unittest.TestCase):
 def test_prior_unreviewed_success_still_refuses(self):
  from rendering_interaction_budget import admit
  prior=dict(report='retained',repair_of=None,result=dict(success=True,original_game_runs=1),proved=False,comparison_failed=False,preparation_fixed=False)
  with self.assertRaisesRegex(ValueError,'unreviewed pilot'):admit([prior],None,2,2)
 def test_empty_fresh_collection_uses_unchanged_guard(self):
  from rendering_interaction_budget import admit
  self.assertEqual(admit([],None,2,2),dict(requests_before=0,original_games_before=0,repair_ancestors=[],resolved_ancestors=[]))
if __name__=='__main__':unittest.main()
