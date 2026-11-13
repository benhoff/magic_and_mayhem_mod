#!/usr/bin/env python3
import json,os,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_derived_world_launch as derived
import broader_secondary_subprocess as previous
class Reader:
 def __init__(self,*a,**k):pass
 def __enter__(self):return self
 def __exit__(self,*a):pass
 def digest(self,n):return 'hash'
class Launch(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);(self.root/'player').write_bytes(b'player');self.calls=[];owner=self
  self.original_popen=subprocess.Popen;self.addCleanup(setattr,subprocess,'Popen',self.original_popen)
  class Base:
   def __init__(self):
    self.root=owner.root;self.plan=dict(player_binary={'path':'player'});self.phase_name='phase';self.report='acquisition';self.popen=owner.actual
   def install(self):
    def call(command,*a,**kw):return self.popen(command,*a,**kw)
    subprocess.Popen=call
   def restore(self):subprocess.Popen=self.popen
  self.base=Base;self.binding=derived.derived_binding(previous.subprocess_binding(Base))()
  for target in [('broader_secondary_subprocess.Reader',Reader),('broader_secondary_subprocess.configuration',lambda *a,**k:dict(root=str(self.root),acquisition='acquisition',selection='selection')),('broader_derived_world_launch.Reader',Reader)]:
   p=patch(target[0],target[1]);p.start();self.addCleanup(p.stop)
  self.check=patch('broader_derived_world_launch.checked_configuration',return_value={});self.check.start();self.addCleanup(self.check.stop);self.binding.install()
 def actual(self,*a,**kw):self.calls.append((a,kw));return 'actual-process'
 def launch(self,env=None):return subprocess.Popen([str(self.root/'player')],env={}if env is None else env)
 def test_actual_boundary_preserves_old_startup_and_prepends_derived_startup(self):
  self.assertEqual(self.launch(),'actual-process');env=self.calls[0][1]['env'];parts=env['PYTHONPATH'].split(os.pathsep)
  self.assertEqual(parts[:2],[str(self.root/Path(derived.STARTUP).parent),str(self.root/Path(previous.STARTUP).parent)]);self.assertEqual(json.loads(env[derived.FLAG])['phase'],{'record':'phase','sha256':'hash'})
 def test_unrelated_process_arguments_and_environment_unchanged(self):
  env={'same':'value'};subprocess.Popen(['/tmp/unrelated'],env=env,cwd='/tmp');self.assertEqual(self.calls, [((['/tmp/unrelated'],),{'env':env,'cwd':'/tmp'})])
 def test_conflicting_selection_refuses_before_process_creation(self):
  with self.assertRaises(ValueError):self.launch({derived.FLAG:'foreign'})
  self.assertEqual(self.calls,[])
 def test_bad_compiled_input_refuses_before_process_creation(self):
  self.check.stop()
  with patch('broader_derived_world_launch.checked_configuration',side_effect=ValueError('changed')):
   with self.assertRaises(ValueError):self.launch()
  self.assertEqual(self.calls,[])
 def test_restore_returns_exact_original_process_factory(self):self.binding.restore();self.assertEqual(subprocess.Popen,self.actual)
 def test_parent_environment_is_not_modified(self):
  env={'PYTHONPATH':'existing'};saved=dict(env);self.launch(env);self.assertEqual(env,saved)
class Diagnostics(unittest.TestCase):
 def test_absent_startup_receipt_is_a_retained_setup_refusal(self):
  class Base:
   def __init__(self):self.root=ROOT;self.plan={'player_binary':{'path':'working/build/qt-shell/mnm-qt-shell'}};self.popen=lambda *a,**k:None
   def receipt(self):raise FileNotFoundError(2,'not created','/owned/missing-child')
  binding=derived.derived_binding(Base)()
  with self.assertRaisesRegex(ValueError,'startup receipt missing'):binding.receipt()
if __name__=='__main__':unittest.main()
