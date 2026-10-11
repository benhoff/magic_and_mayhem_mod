#!/usr/bin/env python3
import sys,unittest,struct
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'tools'))
from drawing_api_resources import Resources
from drawing_api_contracts import contract
from drawing_api_surface_contracts import SITES


def call(method,cid=1,args=None,eax=0,slots=None,receiver=0x1000):
 return dict(method=method,call_id=cid,site=0x58d1ec,phase=3,thread=5,args=args or [receiver,0x2000],arguments_read=True,receiver=receiver,eax=eax,snapshots=slots or [])
def output(name,value):return dict(name=name,event=2,state=1,data=struct.pack('<I',value).hex())

class Tests(unittest.TestCase):
 def test_exact_all_sites_and_legacy_revision(self):
  self.assertEqual(len(SITES),141)
  for row in SITES.values():
   item=dict(site=row['site'],call={'bytes':row['call_bytes']},target_mode={'kind':'register_memory'})
   self.assertEqual(contract(item,2)['formal_words'],row['formal_words'])
   self.assertEqual(contract(item,1)['origin'],'unreviewed')
 def test_callback_and_outside_offset_stay_unreviewed(self):
  for site in (0x58b71d,0x58c34d,0x580000):
   self.assertEqual(contract(dict(site=site,call={'bytes':'ff12'},target_mode={'kind':'register_memory'}),2)['origin'],'unreviewed')
 def test_exact_signature_and_revision_refusal(self):
  with self.assertRaises(ValueError):contract(dict(site=0x58b69d,call={'bytes':'ff5264'},target_mode={'kind':'register_memory'}),2)
  for revision in (True,0,3,'2'):
   with self.assertRaises(ValueError):contract(dict(site=0),revision)
 def test_dc_pair(self):
  r=Resources();r.observe(call('IDirectDrawSurface2.GetDC',slots=[output('HDC output',0x3456)]));r.observe(call('IDirectDrawSurface2.ReleaseDC',2,args=[0x1000,0x3456]));out=r.result();self.assertFalse(out['gaps']);self.assertFalse(out['open_intervals']);self.assertEqual(out['closed_or_unresolved_intervals'][0]['end_call'],2)
 def test_failed_outputs_and_failed_release_do_not_close(self):
  r=Resources();r.observe(call('IDirectDrawSurface2.GetDC',eax=0x88760000,slots=[output('HDC output',0x3456)]));self.assertFalse(r.leases)
  r.observe(call('IDirectDrawSurface2.GetDC',2,slots=[output('HDC output',0x3456)]));r.observe(call('IDirectDrawSurface2.ReleaseDC',3,args=[0x1000,0x3456],eax=0x88760000));self.assertEqual(len(r.leases),1)
 def test_unknown_origin_and_handle_mismatch_remain_pending(self):
  r=Resources();r.observe(call('IDirectDrawSurface2.ReleaseDC'));r.observe(call('IDirectDrawSurface2.GetDC',2,slots=[output('HDC output',0x3456)]));r.observe(call('IDirectDrawSurface2.ReleaseDC',3,args=[0x1000,0x4567]));self.assertEqual(len(r.gaps),2);self.assertEqual(len(r.leases),1)
 def test_overlapping_success_is_not_silently_replaced(self):
  r=Resources()
  for cid in (1,2):r.observe(call('IDirectDrawSurface2.GetDC',cid,slots=[output('HDC output',0x3456)]))
  self.assertEqual(len(r.gaps),1);self.assertEqual(r.intervals[0]['state'],'overlapped, close unassigned')
 def test_lock_descriptor_and_unlock(self):
  desc=[0]*27;desc[0]=108;desc[1]=0x100f;desc[9]=0x123400;desc[4]=24
  r=Resources();r.observe(call('IDirectDrawSurface2.Lock',slots=[dict(name='DDSURFACEDESC input/output',event=2,state=1,data=struct.pack('<27I',*desc).hex())]));r.observe(call('IDirectDrawSurface2.Unlock',2,args=[0x1000,0]));self.assertFalse(r.gaps);self.assertEqual(r.intervals[0]['descriptor'][4],24)
 def test_bad_descriptor_size_is_gap(self):
  desc=[0]*27;desc[0]=124;desc[1]=0x100f;desc[9]=0x123400
  r=Resources();r.observe(call('IDirectDrawSurface2.Lock',slots=[dict(name='DDSURFACEDESC input/output',event=2,state=1,data=struct.pack('<27I',*desc).hex())]));self.assertFalse(r.leases);self.assertEqual(len(r.gaps),1)
 def test_missing_or_null_output_is_gap(self):
  for slots in ([],[output('HDC output',0)],[dict(name='HDC output',event=2,state=3,data='')]):
   r=Resources();r.observe(call('IDirectDrawSurface2.GetDC',slots=slots));self.assertFalse(r.leases);self.assertEqual(len(r.gaps),1)
 def test_receiver_candidate_required(self):
  r=Resources();c=call('IDirectDrawSurface2.GetDC',slots=[output('HDC output',123)]);c['receiver']=0;r.observe(c);self.assertFalse(r.leases);self.assertEqual(len(r.gaps),1)
 def test_release_does_not_claim_destruction(self):
  r=Resources();r.observe(call('IUnknown.Release'));self.assertEqual(len(r.releases),1);self.assertIn('does not prove destruction',r.releases[0]['scope'])
 def test_interface_failure_is_raw(self):
  r=Resources();r.observe(call('IUnknown.QueryInterface',eax=0x80004002,slots=[output('interface pointer output',123)]));self.assertEqual(r.outputs[0]['validity'],'raw, validity unassigned')

if __name__=='__main__':unittest.main()
