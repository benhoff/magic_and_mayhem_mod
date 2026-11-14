#!/usr/bin/env python3
import copy,json,os,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_secondary_subprocess as s
class Config(unittest.TestCase):
 def setUp(self):
  self.report='research/runtime/control-acquisition.json';self.phase=dict(family='interaction',broader_cases=[dict(id='control',report=self.report)],subprocess_scene_binding=dict(startup=s.STARTUP,helper=s.HELPER,source_waivers=[],sources={'source':'exact'}))
 def config(self):return s.configuration(ROOT,self.phase,self.report,lambda n:'exact')
 def test_exact_target(self):self.assertEqual(self.config()['selection'],s.P+'control-subprocess-scene-selection-01.json')
 def test_source_drift_refuses(self):
  with self.assertRaises(ValueError):s.configuration(ROOT,self.phase,self.report,lambda n:'changed')
 def test_other_family_refuses(self):
  self.phase['family']='pause'
  with self.assertRaises(ValueError):self.config()
 def test_unknown_case_refuses(self):
  self.phase['broader_cases']=[]
  with self.assertRaises(ValueError):self.config()
 def test_unsafe_report_refuses(self):
  with self.assertRaises(ValueError):s.configuration(ROOT,self.phase,'../outside',lambda n:'exact')
 def test_waiver_refuses(self):
  self.phase['subprocess_scene_binding']['source_waivers']=['base']
  with self.assertRaises(ValueError):self.config()
 def test_other_startup_refuses(self):
  self.phase['subprocess_scene_binding']['startup']='other'
  with self.assertRaises(ValueError):self.config()
 def test_other_helper_refuses(self):
  self.phase['subprocess_scene_binding']['helper']='other'
  with self.assertRaises(ValueError):self.config()
 def test_other_preparer_refuses_before_reader(self):
  with patch.object(sys,'argv',['/tmp/foreign.py']),patch.object(s,'Reader')as reader:
   with self.assertRaises(ValueError):s.startup(ROOT,dict())
   reader.assert_not_called()
class Startup(unittest.TestCase):
 def run_python(self,raw):
  env=dict(os.environ,PYTHONPATH=str(ROOT/Path(s.STARTUP).parent)+os.pathsep+str(ROOT/'tools'));env[s.FLAG]=raw
  return subprocess.run([sys.executable,'-c','print("ordinary-child")'],env=env,capture_output=True)
 def test_ordinary_child_is_unchanged(self):
  p=self.run_python(json.dumps(dict(root=str(ROOT))));self.assertEqual(p.returncode,0);self.assertEqual(p.stdout,b'ordinary-child\n')
 def test_malformed_startup_fails_closed(self):
  p=self.run_python('{invalid');self.assertEqual(p.returncode,78);self.assertNotIn(b'ordinary-child',p.stdout);self.assertIn(b'startup refused',p.stderr)
 def test_startup_without_flag_is_inert(self):
  env=dict(os.environ,PYTHONPATH=str(ROOT/Path(s.STARTUP).parent));env.pop(s.FLAG,None)
  p=subprocess.run([sys.executable,'-c','print("unchanged")'],env=env,capture_output=True);self.assertEqual(p.returncode,0);self.assertEqual(p.stdout,b'unchanged\n')
if __name__=='__main__':unittest.main()
