#!/usr/bin/env python3
import sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_repair_binding import arguments
class RepairBinding(unittest.TestCase):
 def setUp(self):self.argv=[str(ROOT/'tools/run-rendering-transition.py'),'--plan','new-plan','--report','new-acquisition.json']
 def test_only_existing_ancestry_arguments_added(self):self.assertEqual(arguments(ROOT,self.argv,'failed-acquisition.json','Storage guard stopped collector'),self.argv+['--repair-of','failed-acquisition.json','--repair-reason','Storage guard stopped collector'])
 def test_other_collector_refused(self):
  with self.assertRaises(ValueError):arguments(ROOT,['other.py'],'failed-acquisition.json','reason')
 def test_profile_or_extent_change_refused(self):
  for option in('--low','--software'):
   with self.assertRaises(ValueError):arguments(ROOT,self.argv+[option],'failed-acquisition.json','reason')
 def test_missing_or_duplicate_ancestry_refused(self):
  for argv,failed,reason in[(self.argv+['--repair-of','other'],'failed-acquisition.json','reason'),(self.argv,'other','reason'),(self.argv,'failed-acquisition.json','')]:
   with self.assertRaises(ValueError):arguments(ROOT,argv,failed,reason)
if __name__=='__main__':unittest.main()
