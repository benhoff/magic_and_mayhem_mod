import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_picking_repair_matrix import verify_case,PILOT
class Controls(unittest.TestCase):
 def setUp(self):
  self.old=dict(id=PILOT,controller_arguments=['--plan','old'],command=['python3','tool','old'],profile='default',size=[800,600],reserved_original_games=1)
  self.new=copy.deepcopy(self.old);self.new['controller_arguments']=['--plan','new','--repair-of','failed','--repair-reason','reason'];self.new['command'][-1]='new'
 def check(self):return verify_case(self.old,self.new,'old','new','failed','reason')
 def test_exact_ancestry(self):self.assertTrue(self.check())
 def test_missing_ancestry(self):
  self.new['controller_arguments']=self.new['controller_arguments'][:2]
  with self.assertRaises(ValueError):self.check()
 def test_wrong_failure(self):
  self.new['controller_arguments'][3]='other'
  with self.assertRaises(ValueError):self.check()
 def test_changed_extent(self):
  self.new['size']=[640,480]
  with self.assertRaises(ValueError):self.check()
 def test_changed_charge(self):
  self.new['reserved_original_games']=2
  with self.assertRaises(ValueError):self.check()
 def test_changed_case(self):
  self.new['id']='other'
  with self.assertRaises(ValueError):self.check()
 def test_software_gets_no_repair(self):
  self.old['id']=self.new['id']='software';self.new['controller_arguments']=['--plan','new'];self.assertTrue(self.check())
if __name__=='__main__':unittest.main()
