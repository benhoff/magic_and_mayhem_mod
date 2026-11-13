import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_uncovered_final_gate import same_context
class ContextTests(unittest.TestCase):
 def setUp(self):self.original=dict(role='world',family='caller',case=dict(id='bp29-case',profile='player-default',provider='nvidia',size=[800,600],reserved_original_games=1));self.value=dict(spec=copy.deepcopy(self.original));self.value['spec']['case']['id']='bp31-case'
 def test_rename_only_allowed(self):self.assertIs(same_context(self.value,self.original),self.value)
 def test_profile_refuses(self):
  self.value['spec']['case']['profile']='player-software'
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_provider_refuses(self):
  self.value['spec']['case']['provider']='mesa'
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_extent_refuses(self):
  self.value['spec']['case']['size']=[640,480]
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_charge_refuses(self):
  self.value['spec']['case']['reserved_original_games']=2
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_role_refuses(self):
  self.value['spec']['role']='fault'
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_family_refuses(self):
  self.value['spec']['family']='palette'
  with self.assertRaises(ValueError):same_context(self.value,self.original)
 def test_duration_refuses(self):
  self.original['role']=self.value['spec']['role']='strict';self.original['case']['stress_seconds']=30;self.value['spec']['case']['stress_seconds']=10
  with self.assertRaises(ValueError):same_context(self.value,self.original)
if __name__=='__main__':unittest.main()
