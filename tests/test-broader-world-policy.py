import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_world_policy import assess,POLICY
class Reader:
 def __init__(self,values):self.values=values
 def json(self,n):return self.values[n]
 def digest(self,n):return'hash-'+n
def fixture():
 ref=lambda n:dict(record=n,sha256='hash-'+n)
 raw=dict(acquisition=ref('acquired'),sources_stable=True,comparison_processes_stopped=True,original_manifest_verified_before_after=True)
 certificate=dict(schema=1,policy=POLICY,original_padding_equivalence=False,index_sha256='index',worlds=[dict(generation=1,width=4,height=2,taps=[dict(draw=5,destination=7,outside_sample=8,retained_word=123)])])
 build=dict(success=True,sources={},binary=ref('binary'));controls=dict(success=True,cases=26,guard_sha256='hash-tools/broader_finite_edge_oracle.hpp')
 r=Reader(dict(raw=raw,certificate=certificate,build=build,controls=controls))
 group=dict(source_sha256='source',output_identity='/owned/reference',index_sha256='index',worlds=1,certificate=ref('certificate'))
 q=dict(acquisition=ref('acquired'),success=True,sources_stable=True,comparison_processes_stopped=True,original_manifest_verified_before_after=True,comparison=dict(success=True,mismatching_pixels=0),original_reference=dict(worlds=1),native_edge_policy=dict(id=POLICY,ignored_pixels=0,all_output_pixels_compared=True,original_padding_equivalence=False,production_native_code_changed=False,raw_original_comparison=ref('raw'),oracle_build=ref('build'),controls=ref('controls'),reference_groups=[group],certified_worlds=1,retained_taps=1))
 return r,q
class WorldPolicy(unittest.TestCase):
 def test_complete_join(self):r,q=fixture();self.assertTrue(assess(r,q)['verified'])
 def test_ignored_pixel_refuses(self):
  r,q=fixture();q['native_edge_policy']['ignored_pixels']=1
  with self.assertRaises(ValueError):assess(r,q)
 def test_missing_world_refuses(self):
  r,q=fixture();r.values['certificate']['worlds']=[]
  with self.assertRaises(ValueError):assess(r,q)
 def test_in_range_exception_refuses(self):
  r,q=fixture();r.values['certificate']['worlds'][0]['taps'][0]['outside_sample']=7
  with self.assertRaises(ValueError):assess(r,q)
 def test_duplicate_reference_refuses(self):
  r,q=fixture();q['native_edge_policy']['reference_groups']*=2
  with self.assertRaises(ValueError):assess(r,q)
 def test_raw_capture_mismatch_refuses(self):
  r,q=fixture();r.values['raw']['acquisition']['sha256']='other'
  with self.assertRaises(ValueError):assess(r,q)
 def test_unknown_policy_refuses(self):
  r,q=fixture();q['native_edge_policy']['id']='other'
  with self.assertRaises(ValueError):assess(r,q)
 def test_mismatch_cannot_gain_acceptance(self):
  r,q=fixture();q['comparison']['mismatching_pixels']=1
  with self.assertRaises(ValueError):assess(r,q)
 def test_summary_must_count_every_tap(self):
  r,q=fixture();q['native_edge_policy']['retained_taps']=0
  with self.assertRaises(ValueError):assess(r,q)
if __name__=='__main__':unittest.main()
