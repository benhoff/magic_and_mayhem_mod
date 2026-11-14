"""Adversarial admission controls for explicit native-policy pixel comparisons."""
import copy,hashlib,io,json,sys,tarfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_policy_acceptance import assess,POLICY
class Reader:
 def __init__(self):self.values={}
 def add(self,name,value):
  self.values[name]=value if isinstance(value,bytes)else json.dumps(value,sort_keys=True).encode()
  return dict(record=name,sha256=self.digest(name))
 def digest(self,name):return hashlib.sha256(self.values[name]).hexdigest()
 def read_bytes(self,name):return self.values[name]
 def json(self,name):return json.loads(self.values[name])
def fixture(tap=None):
 r=Reader();r.add('tools/broader_finite_edge_oracle.hpp',b'guard');binary=r.add('binary',b'compiled-policy');build=r.add('build',dict(success=True,sources={},binary=binary));controls=r.add('controls',dict(success=True,cases=26,original_game_runs=0,guard_sha256=r.digest('tools/broader_finite_edge_oracle.hpp')))
 epochs=[dict(root=str(i),world_domain=dict(extents=[[4,2]],worlds=[dict(generation=1,width=4,height=2)]),observations={},asset_inputs={},original_reference=dict(worlds=1,reference_model='original-rasters-with-explicit-native-finite-edge',original_equivalence_for_padding=False),comparison=dict(success=True,mismatching_pixels=0))for i in(0,1)]
 raw=r.add('raw',dict(acquisition=dict(record='acquired',sha256='a'*64),sources_stable=True,comparison_processes_stopped=True,original_manifest_verified_before_after=True,epochs=copy.deepcopy(epochs)))
 certificates=[];members=[];stream=io.BytesIO()
 with tarfile.open(fileobj=stream,mode='w:gz')as archive:
  for i in(0,1):
   taps=[tap or dict(draw=5,destination=7,outside_sample=8,retained_word=123)]if i else[]
   value=dict(schema=1,policy=POLICY,original_padding_equivalence_claimed=False,taps=taps)
   files={'world-000001.565.finite-edge.json':value,'worlds.json':dict(success=True,worlds=[dict(generation='1',width=4,height=2)])}
   for name,data in files.items():
    body=json.dumps(data).encode();name='epoch-'+str(i)+'/reference/'+name;entry=tarfile.TarInfo(name);entry.size=len(body);archive.addfile(entry,io.BytesIO(body));members.append(dict(path=name,bytes=len(body),sha256=hashlib.sha256(body).hexdigest()))
   certificates.append(dict(epoch=i,world_certificates=1,retained_taps=len(taps),affected_worlds=[dict(world=1,taps=taps)]if taps else[]))
 body=stream.getvalue();artifact=dict(**r.add('archive',body),bytes=len(body),encoding='tar-gzip',members=members)
 c=dict(acquisition=dict(record='acquired',sha256='a'*64),epochs=epochs,artifacts=[artifact],native_edge_policy=dict(id=POLICY,all_viewport_pixels_compared=True,ignored_pixels=0,raw_original_result_preserved=True,original_equivalence_for_retained_taps=False,production_renderer_changed=False,original_game_runs_added=0,raw_original_comparison=raw,oracle_build=build,controls=controls,raw_original_mismatching_pixels=0,certificates=certificates))
 return r,c
class PolicyAcceptance(unittest.TestCase):
 def test_full_certificate_joins(self):
  r,c=fixture();self.assertTrue(assess(r,c)['verified'])
 def test_unknown_policy_refuses(self):
  r,c=fixture();c['native_edge_policy']['id']='ignore-some-pixels'
  with self.assertRaises(ValueError):assess(r,c)
 def test_ignored_pixels_refuse(self):
  r,c=fixture();c['native_edge_policy']['ignored_pixels']=1
  with self.assertRaises(ValueError):assess(r,c)
 def test_uncovered_summary_refuses(self):
  r,c=fixture();c['native_edge_policy']['certificates'][1]['retained_taps']=0
  with self.assertRaises(ValueError):assess(r,c)
 def test_changed_archive_refuses(self):
  r,c=fixture();r.values['archive']+=b'changed'
  with self.assertRaises(ValueError):assess(r,c)
 def test_valid_in_range_sample_cannot_gain_exception(self):
  r,c=fixture(dict(draw=5,destination=6,outside_sample=7,retained_word=123))
  with self.assertRaises(ValueError):assess(r,c)
 def test_different_capture_refuses(self):
  r,c=fixture();c['acquisition']['sha256']='b'*64
  with self.assertRaises(ValueError):assess(r,c)
 def test_raw_negative_count_cannot_be_erased(self):
  r,c=fixture();c['native_edge_policy']['raw_original_mismatching_pixels']=1
  with self.assertRaises(ValueError):assess(r,c)
 def test_raw_equivalence_claim_refuses(self):
  r,c=fixture();c['epochs'][1]['original_reference']['original_equivalence_for_padding']=True
  with self.assertRaises(ValueError):assess(r,c)
if __name__=='__main__':unittest.main()
