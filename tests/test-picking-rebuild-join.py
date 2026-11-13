import copy,json,struct,sys,tempfile,time,types,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_rebuild_join import select,rebuild_code,POLICY,TRACE_LIMIT
from broader_picking_rebuild_launch import binding_class
from broader_picking_rebuild_assessment import rebuild_joins
from rendering_picking_resize import targets,ready

class Controls(unittest.TestCase):
 def setUp(self):
  self.before=dict(ordinal=1,kind=1,frame=dict(generation=10,nonce='123'),thread=7,high_resolution=0)
  self.after=dict(ordinal=2,kind=1,frame=dict(generation=20,nonce='123'),thread=7,high_resolution=1)
  self.paint=dict(paint='3',world='20',frame_width=800,frame_height=600,presentation_owner='gui',native_gpu_frame=True,gpu_read_complete=True,pixels_captured=True)
 def pick(self,paints=None,rows=None,**kw):
  args=dict(before=self.before,size=[800,600],old_world=15,low=False,now=20,deadline=30);args.update(kw)
  return select([self.paint]if paints is None else paints,[self.after]if rows is None else rows,**args)
 def test_exact_join(self):self.assertEqual(self.pick(),(self.paint,self.after))
 def test_missing_row_waits(self):self.assertIsNone(self.pick(rows=[]))
 def test_future_row_is_not_same_generation(self):
  self.after['frame']['generation']=21;self.assertIsNone(self.pick())
 def test_old_row_is_not_same_generation(self):
  self.after['frame']['generation']=19;self.assertIsNone(self.pick())
 def test_wrong_flag_waits(self):self.after['high_resolution']=0;self.assertIsNone(self.pick())
 def test_low_flag_join(self):
  self.after['high_resolution']=0;self.paint.update(frame_width=640,frame_height=480);self.assertIsNotNone(self.pick(size=[640,480],low=True))
 def test_nonce_mismatch(self):self.after['frame']['nonce']='456';self.assertIsNone(self.pick())
 def test_thread_mismatch(self):self.after['thread']=8;self.assertIsNone(self.pick())
 def test_wrong_extent(self):self.paint['frame_width']=640;self.assertIsNone(self.pick())
 def test_wrong_owner(self):self.paint['presentation_owner']='worker';self.assertIsNone(self.pick())
 def test_incomplete_gpu(self):
  for key in ['native_gpu_frame','gpu_read_complete','pixels_captured']:
   with self.subTest(key=key):
    p=dict(self.paint);p[key]=False;self.assertIsNone(self.pick(paints=[p]))
 def test_minimum_five_worlds(self):self.assertIsNone(self.pick(old_world=16))
 def test_before_not_fresh(self):self.before['frame']['generation']=20;self.assertIsNone(self.pick())
 def test_deadline_equal_refuses(self):self.assertIsNone(self.pick(now=30))
 def test_deadline_expired_refuses(self):self.assertIsNone(self.pick(now=31))
 def test_latest_same_generation_state_required(self):
  wrong=copy.deepcopy(self.after);wrong['ordinal']=3;wrong['high_resolution']=0;self.assertIsNone(self.pick(rows=[self.after,wrong]))
 def test_newest_actual_paint_selected(self):
  p=dict(self.paint,paint='4');self.assertEqual(self.pick(paints=[self.paint,p])[0]['paint'],'4')
 def binding(self,root):
  class Base:
   def __init__(self):self.plan=dict(family='picking');self.popen=lambda:None
   def patch(self,m):return m
  class Journey:
   def latest(s):s.original_reads+=1;return self.before
  interaction=types.SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=Journey)
  camera=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',LiveViewport=object)
  namespace=dict(interaction=interaction,time=time)
  exec('''def main():
 class PickingJourney(interaction.InteractionJourney):
  def rebuild(self,low):
   before=self.latest()
   old_world=15
   deadline=time.monotonic()+30
   paint=self.viewport.poll()
   after=self.latest()
   return paint,after
 return PickingJourney
''',namespace)
  controller=types.SimpleNamespace(__file__='run-rendering-picking-closeout.py',main=namespace['main'],targets=targets,ready=ready)
  code=rebuild_code(controller.main);b=binding_class(Base)()
  for m in [camera,interaction,controller]:b.patch(m);b.patch(m)
  j=controller.main()();j.original_reads=0;j.session=root;j.size=[800,600];j.viewport=camera.LiveViewport(root)
  return b,j,controller,code
 def test_recursive_binding_exact_cached_row_and_unchanged_code(self):
  with tempfile.TemporaryDirectory(dir='/tmp')as tmp:
   root=Path(tmp);(root/'frame-input.bin').write_bytes(b'0'*16);p=dict(self.paint,index=0,kind='paint');swap=dict(index=1,kind='swap',paint='3');raw=b'MNMACE01'
   for row in [p,swap]:
    meta=json.dumps(row).encode();raw+=struct.pack('<II',len(meta),0)+meta
   (root/'viewport-1.bin').write_bytes(raw)
   b,j,c,code=self.binding(root)
   with patch('broader_picking_rebuild_launch.decode',return_value=dict(rows=[self.after])):
    paint,after=j.rebuild(False)
   self.assertEqual(after,self.after);self.assertEqual(paint,p);self.assertEqual(j.original_reads,1)
   self.assertEqual(len(b.rebuild_joins),1);self.assertFalse(b.rebuild_pending)
   self.assertIs(rebuild_code(c.main),code);self.assertIs(j.rebuild.__code__,code)
   self.assertEqual(j.latest(),self.before);self.assertEqual(j.original_reads,2)
 def test_only_swapped_paints_can_join(self):
  with tempfile.TemporaryDirectory(dir='/tmp')as tmp:
   root=Path(tmp);(root/'frame-input.bin').write_bytes(b'0'*16);meta=json.dumps(dict(self.paint,index=0,kind='paint')).encode();(root/'viewport-1.bin').write_bytes(b'MNMACE01'+struct.pack('<II',len(meta),0)+meta)
   b,j,_,_=self.binding(root)
   with patch('broader_picking_rebuild_launch.decode',return_value=dict(rows=[self.after])):
    paint,after=j.rebuild(False)
   self.assertIsNone(paint);self.assertEqual(after,self.before);self.assertEqual(j.original_reads,2)
   self.assertFalse(b.rebuild_joins)
 def test_unexpected_controller_refused(self):
  with self.assertRaisesRegex(ValueError,'unique frozen'):rebuild_code(lambda:None)
 def assessment(self):
  join=dict(original_before_ordinal=1,original_after=self.after,paint=self.paint,old_world=15,low=False,deadline=30,selected_monotonic=20)
  rebuild=dict(original_before=self.before,original_after=self.after,confirmed_paint=self.paint,after=[800,600])
  policy=dict(policy=POLICY,original_rebuild_bytecode_unchanged=True,original_deadline_seconds=30,physical_actions_unchanged=True,same_generation_nonce_thread_required=True,only_actual_swapped_paints=True,history_row_limit=4096,diagnostic_row_limit=TRACE_LIMIT,joins=[join,copy.deepcopy(join)],observations=[])
  return dict(provider_adaptation=dict(picking_rebuild_source_join=policy),proof=dict(display_rebuilds=[rebuild,copy.deepcopy(rebuild)])),dict(paints={3:(self.paint,b'')},swaps=[3])
 def test_assessment_accepts_exact_join(self):
  a,o=self.assessment();self.assertTrue(rebuild_joins(a,o))
 def test_assessment_missing_join_refused(self):
  a,o=self.assessment();a['provider_adaptation']['picking_rebuild_source_join']['joins'].pop()
  with self.assertRaisesRegex(ValueError,'Both'):rebuild_joins(a,o)
 def test_assessment_unpresented_refused(self):
  a,o=self.assessment();o['swaps']=[]
  with self.assertRaisesRegex(ValueError,'observations differ'):rebuild_joins(a,o)
 def test_assessment_late_refused(self):
  a,o=self.assessment();a['provider_adaptation']['picking_rebuild_source_join']['joins'][0]['selected_monotonic']=30
  with self.assertRaisesRegex(ValueError,'Invalid rebuild'):rebuild_joins(a,o)
 def test_assessment_row_mismatch_refused(self):
  a,o=self.assessment();a['provider_adaptation']['picking_rebuild_source_join']['joins'][0]['original_after']=self.before
  with self.assertRaisesRegex(ValueError,'observations differ'):rebuild_joins(a,o)
if __name__=='__main__':unittest.main()
