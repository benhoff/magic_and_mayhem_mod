#!/usr/bin/env python3
"""Retained low-frame title pixels and strict compatibility-view guards."""
from pathlib import Path
import sys,unittest,tempfile,json,hashlib,subprocess,io
from copy import deepcopy
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from campaign_storage import validate_mini_inputs
from pause_low_ingress import fitted_title,validate_inputs,RECT,PROCESSING
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
class Low(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   a=r.json('research/runtime/rendering-pause-results-20261107/default-640-01-acquisition.json')
   cls.image=artifact(r,a,'mini-ingress-1-4-before.png')
  with Image.open(io.BytesIO(cls.image)) as image:
   b=io.BytesIO();fitted_title(image).save(b,format='PNG');cls.crop=b.getvalue()
 def files(self,directory):
  d=Path(directory);(d/'source.png').write_bytes(self.image);(d/'crop.png').write_bytes(self.crop)
  observation=dict(image='source.png',image_sha256=hashlib.sha256(self.image).hexdigest(),crop='crop.png',crop_sha256=hashlib.sha256(self.crop).hexdigest(),rect=list(RECT),ocr_preprocessing=PROCESSING,title_rule='hermes-low-fit-title-v1',ocr_mode=7,ocr_returncode=0,text='Hermes\n')
  base=dict(success=True,mode='mini',skip_click=True,input_active_checked=True,viewport_pixels=[1060,795],source_frame_pixels=[640,480])
  record=dict(base,tutorial_observations=[observation],tutorial_actions=[dict(action='dismiss',logical_point=[400,300],button=1)],escape_sent=False,events=[dict(event='button',button=1,down=v,point=[400,300]) for v in (True,False)])
  (d/'storage-input-001.json').write_text(json.dumps(record))
  (d/'storage-input-002.json').write_text(json.dumps(dict(base,tutorial_observations=[],tutorial_actions=[],escape_sent=True,events=[dict(event='escape',down=v) for v in (True,False)])))
  return record
 def test_retained_image_recognizes_complete_title(self):
  with tempfile.TemporaryDirectory(prefix='pause-low-test-') as t:
   p=Path(t)/'crop.png';p.write_bytes(self.crop)
   r=subprocess.run(['tesseract',str(p),'stdout','--psm','7'],capture_output=True,text=True,check=True)
   self.assertEqual(r.stdout.strip(),'Hermes')
 def test_exact_source_geometry_then_legacy_edges(self):
  with tempfile.TemporaryDirectory(prefix='pause-low-test-') as t:
   self.files(t);v=validate_inputs(t,validate_mini_inputs)
   self.assertTrue(v['verified']);self.assertEqual(v['fitted_title_observations'],1);self.assertEqual(v['source_frame_pixels'],[640,480]);self.assertEqual(len(v['raw_records']),2)
 def test_bad_geometry_processing_and_pixels_refuse(self):
  for what in ('source','aspect','rectangle','processing','pixels'):
   with tempfile.TemporaryDirectory(prefix='pause-low-test-') as t:
    record=self.files(t)
    if what=='source':record['source_frame_pixels']=[800,600]
    elif what=='aspect':record['viewport_pixels']=[1061,795]
    elif what=='rectangle':record['tutorial_observations'][0]['rect']=[395,105,515,140]
    elif what=='processing':record['tutorial_observations'][0]['ocr_preprocessing']=dict(PROCESSING,scale=4)
    else:
     p=Path(t)/'crop.png'
     with Image.open(p) as image:
      image.putpixel((0,0),0);image.save(p)
     record['tutorial_observations'][0]['crop_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    (Path(t)/'storage-input-001.json').write_text(json.dumps(record))
    with self.subTest(what=what),self.assertRaises(ValueError):validate_inputs(t,validate_mini_inputs)
if __name__=='__main__':unittest.main()
