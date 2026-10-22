"""Independent native adapter bounds, temporal joins and journal refusals."""
import importlib.util
import struct
import sys
import tempfile
import tracemalloc
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_lifetime_index import LifetimeIndex
from drawing_source_worker_lease import decode as workers
from drawing_source_write_trace import snapshot,marker
from drawing_gpu_completion import decode as gpu
from drawing_storage_transitions import decode as storage
from drawing_wine_resource_copy import decode as copies
from drawing_wine_source_descriptor import analyze as descriptors


def fixture(name,function):
 spec=importlib.util.spec_from_file_location(name,ROOT/('tests/test-drawing-'+name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return getattr(m,function)()

def change(raw,offset,value):
 data=bytearray(raw);struct.pack_into('<I',data,offset,value);return data

def lifetimes(objects=256,repeated=3000):
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048];rows=[];call=0
 def pair(method,obj,generation,result=0):
  nonlocal call
  call+=1;r=[0]*24;r[:5]=[len(rows)+1,1,call,method,4]
  if method==2:r[10:12]=[obj,generation]
  rows.append(r);r=list(r);r[:2]=[len(rows)+1,2];r[8]=result;r[10:12]=[obj,generation]
  if method==2 and not result:r[12]=2
  rows.append(r)
 for i in range(1,objects+1):pair(1,i,i)
 for _ in range(repeated):pair(2,objects,objects,1)
 t=[0]*24;t[:5]=[len(rows)+1,4,1,call,4];t[11]=objects;rows.append(t)
 return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)

def worker_wire(count):
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSWL01');h[2:8]=[1,64,0x40209ca7,16384,2048,5000];rows=[]
 def row(phase,command=0,worker=0,n=0,done=0):
  r=[0]*16;r[:11]=[len(rows)+1,phase,1,1,7,10,command,worker,n,done,0];rows.append(r)
 row(1)
 for i in range(1,count+1):row(2,i,0,i,i-1);row(3,i,i,i,i)
 row(4,n=count,done=count);t=[0]*16;t[:3]=[len(rows)+1,5,1];rows.append(t)
 return struct.pack('<16I',*h)+b''.join(struct.pack('<16I',*r) for r in rows)


class FifthTen(unittest.TestCase):
 def test_lifetime_prefix_exact_create_and_retire(self):
  raw=fixture('wine-lifetime','wire');index=LifetimeIndex(raw)
  self.assertEqual([index.generation(i,1,1) for i in range(6)],[0,0,1,1,0,0])
  self.assertEqual(index.generation(5,1,0),0)
  for prefix in (-1,6):
   with self.assertRaises(ValueError):index.generation(prefix,1,1)

 def test_descriptor_pipeline_linear_memory_and_exact_prefix(self):
  life=lifetimes();h,sample,end=fixture('wine-source-descriptor','wire')
  sample[2:5]=[1,3,4];sample[6:8]=[2,2];sample[13:16]=[512,6512,2]
  descriptor=bytearray(struct.pack('<16I40I40I',*h,*sample,*end));descriptor[:8]=b'MNMWSD01'
  raw=bytearray(fixture('wine-submission','wire'));raw[:8]=b'MNMWIN02';struct.pack_into('<I',raw,8,2)
  for at in (64,256):
   struct.pack_into('<I',raw,at+13*4,0);struct.pack_into('<3I',raw,at+4*4,4,3,1);struct.pack_into('<3I',raw,at+27*4,1,2,512)
  struct.pack_into('<I',raw,len(raw)-192+10*4,256)
  with tempfile.TemporaryDirectory() as directory:
   path=Path(directory);(path/'wine-lifetime.bin').write_bytes(life);(path/'wine-submission.bin').write_bytes(raw);(path/'wine-source-descriptor.bin').write_bytes(descriptor)
   tracemalloc.start()
   try:result=descriptors(path);_,peak=tracemalloc.get_traced_memory()
   finally:tracemalloc.stop()
   self.assertTrue(result['all_sample_submission_identities_bound']);self.assertLess(peak,16*1024*1024)
   (path/'wine-source-descriptor.bin').write_bytes(change(descriptor,64+14*4,999999))
   with self.assertRaises(ValueError):descriptors(path)

 def test_worker_inclusive_command_capacity(self):
  self.assertEqual(workers(worker_wire(2048))['completed_commands'],2048)
  with self.assertRaisesRegex(ValueError,'capacity'):workers(worker_wire(2049))

 def test_worker_record_envelope_matches_header_cap(self):
  raw=worker_wire(1)[:64]+bytes(16385*64)
  with self.assertRaisesRegex(ValueError,'envelope'):workers(raw)

 def test_resource_copy_parent_depth_is_complete(self):
  raw,resources=fixture('wine-resource-copy','fixture')
  for word,value in ((4,0),(5,0),(5,33)):
   invalid=change(change(raw,64+word*4,value),256+word*4,value)
   with self.assertRaises(ValueError):copies(invalid,resources)

 def test_free_regions_cannot_claim_shared_allocation(self):
  raw=fixture('storage-transitions','candidate_fixture')
  for field in (12,15):
   invalid=change(raw,352+field*4,0x10000)
   with self.assertRaisesRegex(ValueError,'live region'):storage(invalid)
   invalid=change(invalid,352+5*4,23)
   self.assertFalse(storage(invalid)['candidates'][0]['flags']&8)

 def test_fence_command_and_submission_are_paired(self):
  raw=fixture('gpu-completion','journal')
  for field in (3,4):
   with self.assertRaises(ValueError):gpu(change(raw,64+field*4,0))
  self.assertEqual(gpu(change(change(raw,64+3*4,0),64+4*4,0))['signaled'],1)

 def test_only_armed_and_done_write_markers_exist(self):
  trace=snapshot(fixture('source-write-trace','wire'))
  for phase in (0,1,4,0xffffffff):
   raw=b'MNMSWD01'+struct.pack('<6I',1,32,trace['lease'],trace['canvas'],trace['owner'],phase)
   with self.assertRaisesRegex(ValueError,'phase'):marker(raw,trace,phase)


if __name__=='__main__':unittest.main()
