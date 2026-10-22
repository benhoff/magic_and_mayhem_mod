#!/usr/bin/env python3
"""Adversarial source/update journal tests; observations do not grant ownership."""
import copy,importlib.util,struct,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from engine_cadence import decode_bytes
from drawing_wine_blit_worker import decode as workers
from drawing_producer_updates import decode as producer_updates
from drawing_primitive_lifetimes import decode as primitives
from drawing_cursor_gate_journal import account
from drawing_source_cursor_gate import decode as gate_binding
from drawing_publication import analyze as action_analysis
import drawing_update_accounting as accounting


def load(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def rows(data,width):return [list(r) for r in struct.iter_unpack('<'+str(width)+'I',data[64:])]
def pack(data,items):return bytes(data[:64])+b''.join(struct.pack('<'+str(len(r))+'I',*r) for r in items)


def producer_wire():
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDUP01');h[2:7]=[1,96,0x40209ca7,256,2];items=[]
 for q in (1,2):
  p=[64,1,q*10,20,30+q,0,7,1,q*40,0,1000,0,8,1,0,0]
  for phase,prefix in ((1,q*20-10),(2,q*20)):
   r=[0]*24;r[:8]=[len(items)+1,phase,q,7,prefix,1,1,1];r[8:24]=p;items.append(r)
 t=[0]*24;t[:6]=[5,4,1,0,2,40];items.append(t)
 return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in items)


class CompletionGuards(unittest.TestCase):
 def test_cadence_mode_is_a_byte(self):
  h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCAD01');h[2:11]=[2,96,1000,0,1,0,0,7,262144]
  r=[0]*24;r[:4]=[1,2,7,10];data=pack(struct.pack('<16I',*h),[r]);self.assertEqual(decode_bytes(data)[0],1000)
  r[10]=256
  with self.assertRaisesRegex(ValueError,'identity/time'):decode_bytes(pack(data,[r]))

 def test_worker_lifecycle_terminal_is_not_a_prefix(self):
  f=load('eleventh_worker','tests/test-drawing-wine-blit-worker.py');data,life=f.fixture();items=rows(data,48)
  for r in items[:-1]:r[7]=1
  with self.assertRaisesRegex(ValueError,'generation'):workers(pack(data,items),life)

 def test_worker_object_capacity_and_terminal_thread(self):
  f=load('eleventh_worker_capacity','tests/test-drawing-wine-blit-worker.py');data,life=f.fixture()
  for kind in ('object','thread'):
   items=rows(data,48)
   if kind=='object':
    for r in items[:-1]:r[8]=2049
   else:items[-1][4]=0
   with self.subTest(kind=kind),self.assertRaises(ValueError):workers(pack(data,items),life)

 def test_worker_lifecycle_lookup_has_bounded_peak_storage(self):
  import tracemalloc
  f=load('eleventh_worker_storage','tests/test-drawing-wine-blit-worker.py');data,_=f.fixture();items=[]
  h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048]
  for identity in range(1,1025):
   entry=[0]*24;entry[:5]=[len(items)+1,1,identity,1,7];items.append(entry)
   returned=entry[:];returned[0]+=1;returned[1]=2;returned[10:12]=[identity,identity];items.append(returned)
  terminal=[0]*24;terminal[:5]=[2049,4,1,1024,7];terminal[11]=1024;items.append(terminal)
  lifetime=pack(struct.pack('<16I',*h),items);wire=rows(data,48)
  for r in wire[:-1]:r[7]=2048;r[9]=1;r[11]=2
  tracemalloc.start()
  try:
   observed=workers(pack(data,wire),lifetime);peak=tracemalloc.get_traced_memory()[1]
  finally:tracemalloc.stop()
  self.assertEqual(observed['unknown_destination_generations'],0);self.assertLess(peak,16*1024*1024)

 def test_producer_identity_clock_and_prefix_are_session_consistent(self):
  data=producer_wire();self.assertEqual(producer_updates(data)['queue_count'],2)
  for kind in ('owner','frequency','prefix'):
   items=rows(data,24)
   if kind=='owner':
    for r in items[2:4]:r[3]=8;r[14]=8
   elif kind=='frequency':
    for r in items[2:4]:r[18]=1001
   else:items[2][4]=19
   with self.subTest(kind=kind),self.assertRaises(ValueError):producer_updates(pack(data,items))

 def test_accounting_never_truncates_outside_source_prefix(self):
  f=load('eleventh_accounting','tests/test-drawing-update-accounting.py')
  for kind in ('negative','past-end','reverse'):
   args=f.fixture()
   if kind=='negative':args[2]['points'][0]['producer_prefix']=-1
   elif kind=='past-end':args[2]['points'][-1]['producer_prefix']=len(args[4])+1
   else:args[2]['points'][95]['producer_prefix']-=1
   with self.subTest(kind=kind),self.assertRaises(ValueError):accounting.build(*args)

 def test_accounting_rejects_boolean_ids_and_invalid_clock(self):
  f=load('eleventh_accounting_types','tests/test-drawing-update-accounting.py')
  for kind in ('identity','frequency'):
   args=f.fixture()
   if kind=='identity':args[1]['calls'][0]['entry']=True
   else:args[0]['frequency']=args[1]['frequency']=0
   with self.subTest(kind=kind),self.assertRaises(ValueError):accounting.build(*args)

 def test_primitive_classification_may_change_during_acquisition(self):
  f=load('eleventh_primitive','tests/test-drawing-primitive-lifetimes.py');h,items,p,c=f.fixture()
  for i in (3,6,7,8):items[i][11]=1
  self.assertTrue(primitives(f.raw(h,items),p,c)['success'])
  items[6][11]=0
  with self.assertRaisesRegex(ValueError,'classification'):primitives(f.raw(h,items),p,c)

 def test_completed_cursor_token_cannot_be_reissued(self):
  point=[0]*16;point[13]=3
  def row(phase,token=1,levels=0):
   p=point[:];p[13]=levels
   return dict(phase=phase,token=token,call_identity=0,mutex_identity=0,role_mask=0,owner=7,outer_identity=1,inner_identity=2,diagnostic_result=1,point=p,reason=3)
  data=dict(rows=[row(6,levels=3),row(10),row(7),row(6,levels=3),row(10),row(7),row(9,token=0)],terminal_reason=3)
  self.assertIn('invalid begin token',account(data)['gaps'])

 def test_gate_binding_cannot_invent_unacquired_source_lease(self):
  f=load('eleventh_gate_binding','tests/test-drawing-source-cursor-gate.py')
  for refused in (False,True):
   data=f.fixture(refused);items=rows(data,48);items[0][5]=9
   with self.subTest(refused=refused),self.assertRaisesRegex(ValueError,'Unacquired'):gate_binding(pack(data,items))
  data=f.fixture();items=rows(data,48);items[-1][2]=0
  with self.assertRaisesRegex(ValueError,'terminal'):gate_binding(pack(data,items))

 def test_menu_dispatch_queue_prefix_cannot_regress(self):
  f=load('eleventh_actions','tests/test-drawing-publication.py');case=f.AppliedActions();case.setUp();self.addCleanup(case.doCleanups)
  case.actions[0][12]=1;case.actions[1][12]=0;case.save_actions()
  with self.assertRaisesRegex(ValueError,'Unmatched'):action_analysis(case.root)


if __name__=='__main__':unittest.main()
