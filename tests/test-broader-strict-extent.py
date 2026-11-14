#!/usr/bin/env python3
import hashlib,io,sys,unittest
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_strict_extent import frame_extent
class ActualContext(unittest.TestCase):
 def png(self,size):
  b=io.BytesIO();Image.new('RGB',size).save(b,format='PNG');return b.getvalue()
 def test_matching_actual_native_frame(self):
  b=self.png((800,600));v=frame_extent(b,hashlib.sha256(b).hexdigest(),[800,600]);self.assertEqual(v['size'],[800,600])
 def test_wrong_extent_is_not_a_requested_context(self):
  b=self.png((640,480))
  with self.assertRaises(ValueError):frame_extent(b,hashlib.sha256(b).hexdigest(),[800,600])
 def test_changed_bytes_refused(self):
  b=self.png((800,600))
  with self.assertRaises(ValueError):frame_extent(b,'0'*64,[800,600])
 def test_non_image_refused(self):
  with self.assertRaises(OSError):frame_extent(b'bad',hashlib.sha256(b'bad').hexdigest(),[800,600])
if __name__=='__main__':unittest.main()
