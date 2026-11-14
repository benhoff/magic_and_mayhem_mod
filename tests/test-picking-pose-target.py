import copy,hashlib,struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pose_target import targets,bound_targets,PosedRows,POLICY
from scene_actor_observation import crc32c

def case():
 blob=bytearray(83);struct.pack_into('<5I',blob,0,len(blob),6,3,0xfffffffc,0xfffffffd)
 for y in range(3):struct.pack_into('<II',blob,40+y*8,64+y*3,73)
 blob[64:73]=bytes([1,4,1,1,4,1,2,2,2]);f=[0]*24;f[2]=9;f[8:10]=[100,200];f[15]=1;f[19]=len(blob);f[22]=0x500512
 a=[0]*200;a[3:5]=[1,400];a[6:9]=[5,62,5];a[12]=1;a[13]=1;a[16]=1;a[28:32]=[0,7,(-76)%2**32,(-82)%2**32];a[160]=20;a[172]=7
 d=[0]*20;d[3]=1;d[5:7]=[100,200];d[9]=0x8ad08ad0;d[11:18]=[len(blob),crc32c(blob),6,3,(-4)%2**32,(-3)%2**32,7];d[18]=1
 frame=dict(generation=10,nonce=123,thread=67,ordinal=3,source_entry_sequence=22,raw_record_sha256='a'*64,actors=[tuple(a)],draws=[tuple(d)])
 return [(tuple(f),bytes(blob))],[100,200,7],[5,62,5],frame
class Controls(unittest.TestCase):
 def test_exact_observed_ANI_offsets(self):
  body,cell=targets(*case());self.assertEqual(cell['point'],[176,282]);self.assertEqual(cell['animation_body_offset'],[-76,-82]);self.assertEqual(cell['displayed_sprite_index'],7);self.assertEqual(cell['target_policy'],POLICY);self.assertNotIn('opaque_pixel',cell)
 def test_different_actual_record_resolves_same_sprite(self):
  args=case();a=list(args[3]['actors'][0]);a[30:32]=[(-18)%2**32,(-85)%2**32];args[3]['actors']=[tuple(a)]
  self.assertEqual(targets(*args)[1]['point'],[118,285])
 def mutate(self,change):
  args=case();change(args)
  with self.assertRaises(ValueError):targets(*args)
 def actor(self,args,index,value):
  a=list(args[3]['actors'][0]);a[index]=value;args[3]['actors']=[tuple(a)]
 def draw(self,args,index,value):
  d=list(args[3]['draws'][0]);d[index]=value;args[3]['draws']=[tuple(d)]
 def test_missing_actor_refuses(self):self.mutate(lambda a:a[3].update(actors=[]))
 def test_duplicate_actor_refuses(self):self.mutate(lambda a:a[3]['actors'].append(a[3]['actors'][0]))
 def test_wrong_owner_refuses(self):self.mutate(lambda a:self.actor(a,2,1))
 def test_wrong_type_refuses(self):self.mutate(lambda a:self.actor(a,1,1))
 def test_zero_health_refuses(self):self.mutate(lambda a:self.actor(a,4,0))
 def test_wrong_footprint_refuses(self):self.mutate(lambda a:self.actor(a,13,2))
 def test_pose_mismatch_refuses(self):self.mutate(lambda a:self.actor(a,6,4))
 def test_absent_role_refuses(self):self.mutate(lambda a:self.actor(a,12,0))
 def test_ANI_opcode_refuses(self):self.mutate(lambda a:self.actor(a,28,6))
 def test_ANI_sprite_index_refuses(self):self.mutate(lambda a:self.actor(a,29,8))
 def test_history_index_refuses(self):self.mutate(lambda a:a[1].__setitem__(2,8))
 def test_missing_draw_refuses(self):self.mutate(lambda a:a[3].update(draws=[]))
 def test_shadow_draw_is_not_body_refuses(self):self.mutate(lambda a:self.draw(a,7,6))
 def test_adjusted_queue_Y_refuses(self):self.mutate(lambda a:self.draw(a,9,0))
 def test_wrong_draw_crc_refuses(self):self.mutate(lambda a:self.draw(a,12,0))
 def test_wrong_draw_anchor_refuses(self):self.mutate(lambda a:self.draw(a,5,99))
 def test_wrong_draw_index_refuses(self):self.mutate(lambda a:self.draw(a,17,8))
 def test_plain_source_rows_refused_by_bound_callable(self):
  a=case()
  with self.assertRaises(ValueError):bound_targets(*a[:3])
 def test_explicit_same_generation_source_binding(self):
  a=case();self.assertEqual(bound_targets(PosedRows(a[0],a[3]),a[1],a[2]),targets(*a))
if __name__=='__main__':unittest.main()
