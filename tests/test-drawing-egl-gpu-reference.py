from pathlib import Path
from hashlib import sha256
from copy import deepcopy
import sys,tempfile,struct,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_egl_gpu_reference import compare
class Cases(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name);self.pixels=struct.pack('<4H',0,31,0xf800,0x7e0);self.file=self.root/'egl-gl-source-000004.565';self.file.write_bytes(self.pixels)
  row=dict(ordinal=1,call=4,width=2,height=2,last_blit=dict(count=1,mask=0x4000,filter=0x2600,source_box=[0,0,2,2],destination_box=[0,2,2,0],read_width=2,read_height=2,read_fbo=1,draw_fbo=0),gpu_source=dict(path=self.file.name,sha256=sha256(self.pixels).hexdigest(),bytes=8,original_oracle_only=True));self.original=dict(egl_swap_completion_observation=dict(frames=[row]))
 def test_exact_independent_source_and_single_native_failure(self):
  r=compare(self.original,self.root,1,self.pixels,2,2);self.assertEqual(r['mismatching_pixels'],0);self.assertFalse(r['original_pixels_used_as_native_inputs'])
  r=compare(self.original,self.root,1,struct.pack('<4H',0xffff,31,0xf800,0x7e0),2,2);self.assertEqual(r['mismatching_pixels'],1)
 def test_hash_or_extent_changes_refuse(self):
  self.file.write_bytes(b'bad')
  with self.assertRaises(ValueError):compare(self.original,self.root,1,self.pixels,2,2)
 def test_unobserved_or_changed_projection_refuses(self):
  for key,value in [('mask',0),('source_box',[0,0,1,1]),('destination_box',[0,0,2,2]),('draw_fbo',1)]:
   old=deepcopy(self.original);old['egl_swap_completion_observation']['frames'][0]['last_blit'][key]=value
   with self.assertRaises(ValueError):compare(old,self.root,1,self.pixels,2,2)
 def test_missing_witness_is_not_a_match(self):self.assertIsNone(compare({},self.root,1,self.pixels,2,2))
 def test_oracle_path_escape_refuses(self):
  self.original['egl_swap_completion_observation']['frames'][0]['gpu_source']['path']='../outside.565'
  with self.assertRaises(ValueError):compare(self.original,self.root,1,self.pixels,2,2)
if __name__=='__main__':unittest.main()
