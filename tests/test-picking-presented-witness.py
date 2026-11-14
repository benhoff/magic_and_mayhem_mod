import copy,json,struct,sys,tempfile,types,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_presented_viewport import PresentedViewport
from broader_picking_witness_launch import binding_class,original_ready
from broader_picking_witness_assessment import event_deadlines
from rendering_picking_resize import targets

class Controls(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory(dir='/tmp');self.root=Path(self.tmp.name);self.path=self.root/'viewport-1.bin';self.path.write_bytes(b'MNMACE01');self.index=0;self.v=PresentedViewport(self.root)
 def tearDown(self):self.tmp.cleanup()
 def append(self,kind,**fields):
  row=dict(kind=kind,index=self.index,**fields);self.index+=1;raw=json.dumps(row).encode()
  with self.path.open('ab')as f:f.write(struct.pack('<II',len(raw),0)+raw)
  return row
 def paint(self,g=2,host=101,swap=True,**fields):
  p=self.append('paint',paint=g,world=str(g),host_us=str(host),native_gpu_frame=True,gpu_read_complete=True,pixels_captured=True,**fields)
  if swap:self.append('swap',paint=g,host_us=str(host+1))
  return p
 def test_unswapped_is_never_candidate(self):
  self.v.arm('a',100);self.paint(swap=False);self.assertIsNone(self.v.poll());self.assertFalse(self.v.candidates)
 def test_actual_swap_retained(self):
  self.v.arm('a',100);p=self.paint();self.assertEqual(self.v.poll(),p);self.assertEqual(list(self.v.candidates.values()),[p])
 def test_partial_metadata_retried(self):
  self.paint();raw=self.path.read_bytes();self.path.write_bytes(raw[:-1]);self.assertIsNone(self.v.poll());self.path.write_bytes(raw);self.assertEqual(self.v.poll()['world'],'2')
 def test_partial_body_retried(self):
  raw=json.dumps(dict(index=0,kind='paint',world='2')).encode();self.path.write_bytes(b'MNMACE01'+struct.pack('<II',len(raw),3)+raw+b'x');self.assertIsNone(self.v.poll());self.assertEqual(self.v.index,0)
 def test_index_gap_refused(self):
  self.index=1;self.paint()
  with self.assertRaisesRegex(ValueError,'sequence gap'):self.v.poll()
 def test_swap_identity_refused(self):
  self.paint(swap=False);self.append('swap',paint=3)
  with self.assertRaisesRegex(ValueError,'identity'):self.v.poll()
 def test_failed_close_refused(self):
  self.append('close',complete=False,error='failure')
  with self.assertRaisesRegex(ValueError,'refused'):self.v.poll()
 def test_successful_close_allowed(self):
  self.append('close',complete=True);self.assertIsNone(self.v.poll())
 def test_boundary_times(self):
  self.v.arm('a',100)
  for g,h in enumerate([99,100,101,8000100,8000101],2):self.paint(g,h)
  self.v.poll();self.assertEqual([p['host_us']for p in self.v.candidates.values()],['101','8000100'])
 def test_arm_after_poll_reuses_only_window(self):
  self.paint();self.v.poll();self.v.arm('a',100);self.assertEqual(len(self.v.candidates),1)
 def test_no_overlapping_windows(self):
  self.v.arm('a',100)
  with self.assertRaisesRegex(ValueError,'overlap'):self.v.arm('b',200)
 def test_finish_identity(self):
  self.v.arm('a',100)
  with self.assertRaisesRegex(ValueError,'identity'):self.v.finish('b')
 def test_late_join_retried_and_distinct(self):
  self.v.arm('a',100);self.paint();self.v.poll();self.paint(3,105);self.v.poll()
  def ready(rows,ads,paint,*rest):return dict(generation=int(paint['world']),paint=paint['paint'])if rows and ads else None
  with patch('broader_picking_presented_viewport.ready',ready):
   self.assertIsNone(self.v.match([],[],self.v.last,{}, {},'object'))
   self.assertEqual(self.v.match([1],[1],self.v.last,{}, {},'object')['generation'],2)
   self.assertEqual(self.v.match([1],[1],self.v.last,{}, {},'object')['generation'],3)
   self.assertIsNone(self.v.match([1],[1],self.v.last,{}, {},'object'))
  self.v.finish('a');self.assertEqual(self.v.completed[0]['witness_generations'],[2,3]);self.assertFalse(self.v.candidates)
 def test_inactive_history_bounded(self):
  for g in range(1,4098):self.paint(g,g)
  self.v.poll();self.assertEqual(len(self.v.presented),4096);self.assertEqual(self.v.presented[0]['world'],'2')
 def test_active_history_overflow_refused(self):
  self.v.arm('a',0)
  for g in range(1,4098):self.paint(g,g)
  with self.assertRaisesRegex(ValueError,'history budget'):self.v.poll()
 def test_trace_overflow_refused(self):
  self.v.arm('a',0);self.v.trace=[None]*16384
  with self.assertRaisesRegex(ValueError,'diagnostic budget'):self.v.match([],[],None,{}, {},'object')
 def test_gpu_incomplete_not_candidate(self):
  self.v.arm('a',100);p=self.paint();raw=self.path.read_bytes().replace(b'"gpu_read_complete": true',b'"gpu_read_complete":false');self.path.write_bytes(raw)
  # Re-encode the changed metadata with the correct declared byte count.
  self.path.write_bytes(b'MNMACE01');self.index=0;p['gpu_read_complete']=False;self.append('paint',**{k:v for k,v in p.items()if k not in('kind','index')});self.append('swap',paint=2);self.v.poll();self.assertFalse(self.v.candidates)
 def test_deadline_assessment_rejects_late(self):
  a=dict(proof=dict(picking_windows=[dict(released=dict(host_us='100'),witnesses=[dict(paint=2)])]));o=dict(paints={2:(dict(paint=2,host_us='8000101'),b'')})
  with self.assertRaisesRegex(ValueError,'eight-second'):event_deadlines(a,o)
 def test_deadline_assessment_accepts_boundary(self):
  a=dict(proof=dict(picking_windows=[dict(released=dict(host_us='100'),witnesses=[dict(paint=2)])]));o=dict(paints={2:(dict(paint=2,host_us='8000100'),b'')});self.assertTrue(event_deadlines(a,o))
 def test_recursive_bindings_and_actual_mark(self):
  class Base:
   def __init__(self):self.plan=dict(family='picking');self.popen=lambda:None
   def patch(self,m):return m
  class Journey:
   def mark(self,action,phase):return dict(host_us='100')
  b=binding_class(Base)();camera=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',LiveViewport=object);interaction=types.SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=Journey);picking=types.SimpleNamespace(__file__='run-rendering-picking-closeout.py',targets=targets,ready=original_ready)
  for m in[camera,interaction,picking]:b.patch(m);b.patch(m)
  v=camera.LiveViewport(self.root);j=interaction.InteractionJourney.__new__(interaction.InteractionJourney);j.viewport=v
  j.mark('high-object','released');self.assertEqual(v.active['released_us'],100);j.mark('high-object','settled');self.assertIsNone(v.active)
  self.assertIs(original_ready,__import__('rendering_picking_resize').ready)
if __name__=='__main__':unittest.main()
