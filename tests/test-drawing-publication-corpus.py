#!/usr/bin/env python3
"""Independent portable publication identity, provenance and oracle guards."""
import copy,gzip,json,struct,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_publication_corpus as corpus
from drawing_corpus import BUILD,sha

class Corpus(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);(self.root/'data').mkdir()
  rows=[]
  for kind in (1,5,10,11,12,6):
   r=[0]*24;r[0]=96;r[1]=len(rows)+1;r[2]=kind;r[3]=1;r[5]=r[6]=r[7]=1
   if kind in (10,11,12):r[14]=1
   rows.append(r)
  header=[int.from_bytes(b'MNMP','little'),int.from_bytes(b'RO01','little'),1,64,0x40209ca7,65536,1]+[0]*9
  self.raw=struct.pack('<16I',*header)+b''.join(struct.pack('<24I',*r) for r in rows)
  self.row=[1,1,0,42,5,1,1,1,1,0x1000000,0,0,1,1,16,0xf800,0x7e0,31,2,1,0,1,0,0]
  self.frame={'ordinal':1,'attempt':1,'source_sequence':5,'queue':1,'canvas':1,'destination':1,'width':1,'height':1,'path':'frame.565','sha256':sha(b'\x34\x12')}
  self.observed={'boundary':'observed_primary_blt','wire_version':1,'readback_drops':0,'all_primary_routes_observed':False,'actions_bound':False,'producers':{'sha256':sha(self.raw),'records':6,'queues':1,'failures':[]},'frames':[self.frame],'trace_sha256':'','calls':[]}
  self.catalog={'schema':1,'build':'nocd','source_sha256':BUILD,'evidence':'original','historical_report_sha256':'','asset_binding_evidence':'bindings','scope':'Synthetic guard only; no original execution.'}
  self.original={'success':True,'source_executable_sha256':BUILD,'original_manifest_verified_before_after':True,'original_pixels_used_as_native_inputs':False}
  self.bindings={'success':True,'capture_report_sha256':'','original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,'asset_sources':{'image.bmp':'capture/image.bmp'},'inputs':{'capture/image.bmp':sha(b'encoded source')},'frames':[{'matching':True,'original_sha256':sha(b'\x34\x12')}]}
  self.reg={'builds':[{'id':'nocd','source_sha256':BUILD}],'evidence':[{'id':'original','kind':'isolated_original','build':'nocd','record':'original-report.json','sha256':'','original_hash_pointer':'/source_executable_sha256','assertions':[{'pointer':'/success','equals':True}]},{'id':'bindings','kind':'native_integration','record':'bindings.json','sha256':'','assertions':[{'pointer':'/success','equals':True}]},{'id':corpus.RETENTION,'kind':'static','record':'catalog.json','sha256':'','assertions':[{'pointer':'/schema','equals':1}]}]}
  self.catalog.update(input_chunks=[self.pack('input',self.raw)],assets=[dict(self.pack('asset',b'encoded source'),name='image.bmp',historical_path='capture/image.bmp')],frames=[{'identity':self.frame,'oracle':self.pack('oracle',b'\x34\x12')}])
  self.trace= b'MNMDP001'+struct.pack('<6I',1,96,0x40209ca7,65536,1024,0)+struct.pack('<24I',*self.row)
  self.refresh()
 def pack(self,name,data):
  packed=gzip.compress(data,mtime=0);p='data/'+name+'.gz';(self.root/p).write_bytes(packed);return {'path':p,'encoding':'gzip','sha256':sha(packed),'raw_sha256':sha(data),'historical_path':'capture/'+name}
 def refresh(self):
  r=self.row;self.observed['trace_sha256']=sha(self.trace);self.observed['calls']=[{'attempt':1,'route':'Blt','hresult':0,'last_error':42,'source_sequence':5,'queue':1,'source_canvas':1,'destination_canvas':1,'primary':True,'flags':0x1000000,'observer_lock_hresult':0,'observer_unlock_hresult':0,'readback_ordinal':1,'thread':0,'action_record':0}]
  self.catalog['trace']=self.pack('trace',self.trace);self.catalog['observation']=copy.deepcopy(self.observed);self.original['drawing_publication']=copy.deepcopy(self.observed)
  data=(json.dumps(self.original)+'\n').encode();(self.root/'original-report.json').write_bytes(data);self.reg['evidence'][0]['sha256']=sha(data);self.catalog['historical_report_sha256']=sha(data);self.bindings['capture_report_sha256']=sha(data)
  data=(json.dumps(self.bindings)+'\n').encode();(self.root/'bindings.json').write_bytes(data);self.reg['evidence'][1]['sha256']=sha(data)
  self.freeze()
 def freeze(self):
  data=(json.dumps(self.catalog)+'\n').encode();(self.root/'catalog.json').write_bytes(data);self.reg['evidence'][2]['sha256']=sha(data)
 def test_independent_portable_reference(self):
  raw,assets,boundaries,planes=corpus.admit(self.root,self.catalog,self.reg);self.assertEqual(len(planes),1);self.assertNotIn(b'\x34\x12',raw);self.assertEqual(boundaries[0]['sequence'],5)
 def test_native_evidence_cannot_supply_original(self):
  self.reg['evidence'][0]['kind']='native_integration'
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_scope_boundary_cannot_be_promoted(self):
  self.observed['boundary']='presented_frame';self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_frame_drops_cannot_be_hidden(self):
  self.observed['readback_drops']=1;self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_manifest_must_be_verified(self):
  self.original['original_manifest_verified_before_after']=False;self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_oracle_bytes_cannot_be_an_owned_asset(self):
  self.bindings['inputs']['capture/image.bmp']=sha(b'\x34\x12');self.catalog['assets'][0]=dict(self.pack('asset',b'\x34\x12'),name='image.bmp',historical_path='capture/image.bmp');self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_missing_plane_refuses(self):
  self.catalog['frames']=[];self.freeze()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_forged_callback_metadata_refuses(self):
  self.catalog['observation']['actions_bound']=True;self.freeze()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_malformed_trace_refuses_even_with_frozen_hash(self):
  self.trace=self.trace[:-1];self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_drifted_asset_binding_refuses(self):
  self.bindings['inputs']['capture/image.bmp']=sha(b'other source');self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_changed_full_plane_refuses(self):
  self.catalog['frames'][0]['oracle']=self.pack('oracle',b'\x56\x78');self.freeze()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_wrong_original_com_flags_refuse(self):
  row=copy.copy(self.row);row[9]=0;self.trace=self.trace[:32]+struct.pack('<24I',*row);self.refresh()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)
 def test_chunk_budget_refuses(self):
  self.catalog['input_chunks']*=17;self.freeze()
  with self.assertRaises(ValueError):corpus.admit(self.root,self.catalog,self.reg)

if __name__=='__main__':unittest.main()
