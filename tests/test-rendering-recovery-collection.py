"""A frozen follow-up cannot silently reuse a stopped recovery budget."""
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('recovery_collection',ROOT/'tools/run-rendering-recovery-closeout.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Collection(unittest.TestCase):
 def plan(self):
  return dict(execution_paths=dict(schema=1,output_root='working/recovery-four09/acceptance',budget='working/transition-four09-attempts.sqlite3',reports={'a-acquisition.json':dict(software=False,size=[640,480])}))
 def test_legacy_collection_keeps_its_original_state(self):
  out,budget=m.collection_paths({},'legacy-acquisition.json',False)
  self.assertEqual(budget,ROOT/'working/transition-closeout08-attempts.sqlite3')
  self.assertEqual(out,ROOT/'working/recovery-closeout08/acceptance/legacy-acquisition')
 def test_new_collection_has_independent_budget_and_output(self):
  out,budget=m.collection_paths(self.plan(),'a-acquisition.json',False)
  self.assertEqual(out,ROOT/'working/recovery-four09/acceptance/a-acquisition')
  self.assertEqual(budget,ROOT/'working/transition-four09-attempts.sqlite3')
 def test_configuration_and_report_must_match_frozen_admission(self):
  for name,software in [('a-acquisition.json',True),('unplanned-acquisition.json',False)]:
   with self.assertRaises(ValueError):m.collection_paths(self.plan(),name,software)
 def test_stopped_budgets_remain_closed(self):
  for path in ['working/transition-closeout08-attempts.sqlite3','working/transition-attempts.sqlite3']:
   p=self.plan();p['execution_paths']['budget']=path
   with self.assertRaises(ValueError):m.collection_paths(p,'a-acquisition.json',False)
 def test_absolute_or_escaping_state_is_refused(self):
  for key in ['budget','output_root']:
   for path in ['/tmp/foreign','working/../../foreign','original/foreign']:
    p=self.plan();p['execution_paths'][key]=path
    with self.assertRaises(ValueError):m.collection_paths(p,'a-acquisition.json',False)
if __name__=='__main__':unittest.main()
