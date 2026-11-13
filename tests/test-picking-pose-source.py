"""Production-encoder equivalence and incremental source-integrity controls."""
import importlib.util,struct,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pose_source import LiveActorFrames
from scene_actor_observation import decode
s=importlib.util.spec_from_file_location('pose_original_encoder_controls',Path(__file__).with_name('test-scene-actor-observation.py'));original=importlib.util.module_from_spec(s);s.loader.exec_module(original)
class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):original.Controls.setUpClass();cls.raw=original.Controls.raw
 @classmethod
 def tearDownClass(cls):original.Controls.tearDownClass()
 def setUp(self):self.tmp=tempfile.TemporaryDirectory(dir='/tmp');self.path=Path(self.tmp.name)/'poses.bin'
 def tearDown(self):self.tmp.cleanup()
 def read(self,raw=None):
  self.path.write_bytes(self.raw if raw is None else raw);r=LiveActorFrames(self.path,keep_all=True);r.poll();return r
 def mutation(self,offset,value):
  b=bytearray(self.raw);struct.pack_into('<I',b,offset,value)
  with self.assertRaises(ValueError):self.read(b)
 def test_actual_production_encoder_matches_strict_decoder(self):
  reader=self.read();old=decode(self.raw);self.assertTrue(reader.complete)
  for new,expected in zip(reader.frames.values(),old['frames']):
   self.assertEqual({k:v for k,v in new.items()if k!='raw_record_sha256'},expected)
 def test_requested_generation_does_not_chase_producer_EOF(self):
  self.path.write_bytes(self.raw);r=LiveActorFrames(self.path);r.poll(1);self.assertEqual(list(r.frames),[1]);self.assertFalse(r.complete)
  r.poll(2);self.assertEqual(list(r.frames),[1,2]);self.assertFalse(r.complete);r.poll();self.assertTrue(r.complete)
 def test_partial_frame_waits_without_consuming(self):
  self.path.write_bytes(self.raw[:200]);r=LiveActorFrames(self.path);r.poll();self.assertEqual(r.at,64);self.assertFalse(r.frames)
  with self.path.open('ab')as f:f.write(self.raw[200:])
  r.poll();self.assertEqual(len(r.frames),2);self.assertTrue(r.complete)
 def test_partial_header_waits(self):
  r=self.read(self.raw[:100]);self.assertEqual(r.at,64);self.assertFalse(r.complete)
 def test_no_footer_is_not_complete(self):self.assertFalse(self.read(self.raw[:-64]).complete)
 def test_repeated_poll_does_not_duplicate(self):
  r=self.read();r.poll();self.assertEqual(r.ordinal,2)
 def test_owner_change_refuses(self):self.mutation(64+struct.unpack_from('<I',self.raw,64)[0]+24,88)
 def test_nonce_change_refuses(self):self.mutation(64+struct.unpack_from('<I',self.raw,64)[0]+16,999)
 def test_crc_change_refuses(self):self.mutation(64+96,0)
 def test_wrong_header_refuses(self):self.mutation(8,1)
 def test_lost_terminal_refuses(self):self.mutation(len(self.raw)-64+24,1)
 def test_bad_terminal_count_refuses(self):self.mutation(len(self.raw)-64+16,999)
 def test_frame_order_refuses(self):self.mutation(68,2)
 def test_invalid_extent_refuses(self):self.mutation(64+72,123)
 def test_unknown_reserved_refuses(self):self.mutation(64+124,1)
 def test_truncated_file_refuses(self):
  r=self.read();self.path.write_bytes(self.raw[:64])
  with self.assertRaises(ValueError):r.poll()
 def test_replaced_file_refuses(self):
  r=self.read();other=self.path.with_suffix('.new');other.write_bytes(self.raw);other.replace(self.path)
  with self.assertRaises(ValueError):r.poll()
 def test_append_after_footer_refuses(self):
  r=self.read()
  with self.path.open('ab')as f:f.write(b'x')
  with self.assertRaises(ValueError):r.poll()
 def test_redirected_file_refuses(self):
  self.path.write_bytes(self.raw);alias=self.path.with_suffix('.alias');alias.symlink_to(self.path)
  with self.assertRaises(ValueError):LiveActorFrames(alias).poll()
if __name__=='__main__':unittest.main()
