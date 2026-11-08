#!/usr/bin/env python3
"""Execute the production observer encoder on owned memory; reject malformed wires."""
import copy,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from scene_actor_observation import decode,crc32c
from scene_combat_input import first_frame_extent

class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.temp=tempfile.TemporaryDirectory(prefix='scene-actor-controls-',dir='/tmp');cls.out=Path(cls.temp.name)
  subprocess.run(['cc','-m32','-std=c11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/scene-actor-observation-fixture.c'),'-o',str(cls.out/'fixture')],check=True)
  for kind in ('normal','foreign','disabled','zero-health','bad-base'):
   subprocess.run([cls.out/'fixture',cls.out/(kind+'.bin'),kind],check=True)
  cls.raw=(cls.out/'normal.bin').read_bytes();cls.result=decode(cls.raw)
 @classmethod
 def tearDownClass(cls):cls.temp.cleanup()
 def mutation(self,offset,value):
  raw=bytearray(self.raw);struct.pack_into('<I',raw,offset,value)
  with self.assertRaises(ValueError):decode(raw)
 def test_actual_production_encoder(self):
  q=self.result;self.assertTrue(q['complete']);self.assertEqual(q['owner'],67);self.assertEqual([f['generation']for f in q['frames']],[1,2])
  self.assertEqual([f['gameplay_sequence']for f in q['frames']],[4,7]);self.assertEqual([f['actors'][0][4]for f in q['frames']],[200,190])
  self.assertFalse(q['actor_allocation_generations_proved']);self.assertEqual(q['frames'][0]['actors'][0][172:174],(2,0))
  self.assertEqual(len(q['frames'][0]['draws']),6)
  self.assertEqual(q['frames'][0]['actors'][0][12],107)
 def test_zero_health_preserves_only_ungated_body_and_role_five(self):
  q=decode((self.out/'zero-health.bin').read_bytes());a=q['frames'][0]['actors'][0]
  self.assertEqual(a[4],0);self.assertEqual(a[12],33)
  self.assertEqual({d[1]for d in q['frames'][0]['draws']},{0,5})
 def test_overflowing_relative_base_is_absent_without_dereference(self):
  q=decode((self.out/'bad-base.bin').read_bytes());a=q['frames'][0]['actors'][0]
  self.assertFalse(a[12]&1);self.assertEqual(a[16],0)
  self.assertTrue(a[12]&(1<<3))
 def test_legacy_v1_layout_stays_readable(self):
  h=[0]*16;h[:9]=[0x534d4e4d,0x31304f41,1,64,0x40209ca7,8192,256,80,256*1024**2]
  f=[0]*32;f[0:3]=[128,1,1];f[4]=123;f[6]=67;f[10]=3;f[16:20]=[1,9,640,480]
  end=[0]*16;end[:8]=[0x534d4e4d,0x31454f41,1,64,1,192,0,67]
  q=decode(struct.pack('<16I32I16I',*h,*f,*end))
  self.assertEqual(q['version'],1);self.assertTrue(q['complete'])
 def test_disabled_observer_writes_nothing(self):self.assertFalse((self.out/'disabled.bin').exists())
 def test_actual_encoder_is_accepted_by_physical_input_consumer(self):
  self.assertEqual(first_frame_extent(self.out/'normal.bin'),[800,600])
 def test_input_consumer_waits_for_complete_producer_frame(self):
  p=self.out/'partial.bin';p.write_bytes(self.raw[:192]);self.assertIsNone(first_frame_extent(p))
 def test_foreign_owner_closes_with_explicit_refusal(self):
  with self.assertRaisesRegex(ValueError,'loss/terminal'):decode((self.out/'foreign.bin').read_bytes())
 def test_crc_known_vector(self):self.assertEqual(crc32c(b'123456789'),0xe3069283)
 def test_missing_closure(self):
  with self.assertRaises(ValueError):decode(self.raw[:-64])
 def test_loss_marker(self):self.mutation(len(self.raw)-64+24,1)
 def test_wrong_header(self):self.mutation(8,3)
 def test_truncated_frame(self):
  with self.assertRaises(ValueError):decode(self.raw[:200])
 def test_wrong_payload_crc(self):self.mutation(64+24*4,0)
 def test_missing_generation(self):self.mutation(64+2*4,0)
 def test_foreign_frame_thread(self):
  second=64+struct.unpack_from('<I',self.raw,64)[0];self.mutation(second+6*4,88)
 def test_different_session(self):
  second=64+struct.unpack_from('<I',self.raw,64)[0];self.mutation(second+4*4,999)
 def test_missing_queue_entry(self):self.mutation(64+16*4,0)
 def test_wrong_extent(self):self.mutation(64+18*4,123)
 def test_unbounded_actor_count(self):self.mutation(64+13*4,257)
 def test_unbounded_draw_count(self):self.mutation(64+14*4,4097)
 def test_garbled_reserved_field(self):self.mutation(64+31*4,1)
 def test_frame_order(self):self.mutation(64+4,2)
 def test_terminal_count(self):self.mutation(len(self.raw)-64+4*4,999)
 def test_initial_partial_is_not_complete(self):
  q=decode(self.raw[:-64],complete=False);self.assertFalse(q['complete']);self.assertEqual(len(q['frames']),2)

if __name__=='__main__':unittest.main()
