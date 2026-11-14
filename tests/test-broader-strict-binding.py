#!/usr/bin/env python3
import importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
s=importlib.util.spec_from_file_location('strict_binding',ROOT/'tools/run-broader-strict.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class StrictBinding(unittest.TestCase):
 def setUp(self):self.plan=dict(source_game='working/input',prefix_template='working/seed',claims_input='working/claims.json');self.case=dict(stress_seconds=10,size=[800,600],profile='player-default',provider='nvidia')
 def test_both_exact_durations_and_providers(self):
  for seconds in(10,30):
   for software in(False,True):
    c=self.case|dict(stress_seconds=seconds,profile='player-software'if software else'player-default',provider='llvmpipe'if software else'nvidia')
    a=m.strict_arguments('original.py',self.plan,c,'report.json')
    self.assertEqual(a,['original.py','--native-only','--functional-lifecycle','--stress-seconds',str(seconds),'--qt-build-type','Release','--source-game',str(ROOT/'working/input'),'--prefix-template',str(ROOT/'working/seed'),'--claims',str(ROOT/'working/claims.json'),'--report','report.json',*([]if software else['--hardware'])])
 def test_mismatched_provider_refused(self):
  with self.assertRaises(ValueError):m.strict_arguments('p',self.plan,self.case|dict(provider='llvmpipe'),'r')
 def test_changed_duration_refused(self):
  for x in(0,11,120):
   with self.assertRaises(ValueError):m.strict_arguments('p',self.plan,self.case|dict(stress_seconds=x),'r')
 def test_changed_extent_refused(self):
  with self.assertRaises(ValueError):m.strict_arguments('p',self.plan,self.case|dict(size=[640,480]),'r')
if __name__=='__main__':unittest.main()
