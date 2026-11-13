import copy,subprocess,sys,tempfile,types,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pose_launch import binding_class,original_targets
from broader_picking_pose_target import PosedRows,bound_targets
from broader_picking_pose_assessment import with_actor_frames
import rendering_picking_resize_assessment as frozen

class Base:
 def __init__(self,root):
  self.root=root;self.plan=dict(family='picking',player_binary=dict(path='player'));self.calls=[]
  self.popen=lambda command,*a,**k:self.calls.append((command,k))
 def patch(self,module):return module
 def install(self):pass
 def restore(self):pass
 def receipt(self):return dict(game_actions_changed=False)

class Journey:
 def __init__(self,session):
  self.session=session;self.size=[800,600];self.source_frames=types.SimpleNamespace(frames={10:[]})
 def ready_target(self):return dict(frame=dict(generation=10,nonce='123'),thread=67,camera=[0]),dict(point=[100,200])

class Controls(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory(dir='/tmp');self.root=Path(self.tmp.name);(self.root/'scene-actors-0001.bin').write_bytes(b'fixture');self.b=binding_class(Base)(self.root)
  self.interaction=types.SimpleNamespace(__file__='/repo/tools/run-rendering-interaction.py',InteractionJourney=Journey)
  self.picking=types.SimpleNamespace(__file__='/repo/tools/run-rendering-picking-closeout.py',targets=original_targets)
  self.frame=dict(generation=10,nonce=123,thread=67,view=0,extent=[800,600]);self.polls=[]
 def tearDown(self):self.tmp.cleanup()
 def reader(self,path):
  parent=self
  class Reader:
   frames={10:parent.frame}
   def poll(self,through):parent.polls.append(through)
  return Reader()
 def journey(self):
  self.b.patch(self.interaction)
  return self.interaction.InteractionJourney(self.root)
 def test_exact_generation_binding(self):
  j=self.journey()
  with patch('broader_picking_pose_launch.LiveActorFrames',self.reader):state,body=j.ready_target()
  self.assertIsInstance(j.source_frames.frames[10],PosedRows);self.assertIs(j.source_frames.frames[10].actor_frame,self.frame);self.assertEqual(self.polls,[10])
 def test_wrong_nonce_refuses(self):
  self.frame['nonce']=999;j=self.journey()
  with patch('broader_picking_pose_launch.LiveActorFrames',self.reader),self.assertRaises(ValueError):j.ready_target()
 def test_wrong_thread_refuses(self):
  self.frame['thread']=68;j=self.journey()
  with patch('broader_picking_pose_launch.LiveActorFrames',self.reader),self.assertRaises(ValueError):j.ready_target()
 def test_wrong_view_refuses(self):
  self.frame['view']=1;j=self.journey()
  with patch('broader_picking_pose_launch.LiveActorFrames',self.reader),self.assertRaises(ValueError):j.ready_target()
 def test_wrong_extent_refuses(self):
  self.frame['extent']=[640,480];j=self.journey()
  with patch('broader_picking_pose_launch.LiveActorFrames',self.reader),self.assertRaises(ValueError):j.ready_target()
 def test_missing_sidecar_refuses(self):
  (self.root/'scene-actors-0001.bin').unlink();j=self.journey()
  with self.assertRaises(ValueError):j.ready_target()
 def test_duplicate_sidecar_refuses(self):
  d=self.root/'other';d.mkdir();(d/'scene-actors-0001.bin').write_bytes(b'fixture');j=self.journey()
  with self.assertRaises(ValueError):j.ready_target()
 def test_recursive_patch_is_idempotent(self):
  self.b.patch(self.interaction);cls=self.interaction.InteractionJourney;self.b.patch(self.interaction);self.assertIs(self.interaction.InteractionJourney,cls)
  self.b.patch(self.picking);self.b.patch(self.picking);self.assertIs(self.picking.targets,bound_targets)
 def test_foreign_target_refuses(self):
  self.picking.targets=lambda:None
  with self.assertRaises(ValueError):self.b.patch(self.picking)
 def test_post_binding_mutation_refuses(self):
  self.b.patch(self.picking);self.picking.targets=original_targets
  with self.assertRaises(ValueError):self.b.patch(self.picking)
 def test_unique_native_launch_gets_existing_flag(self):
  self.b.install()
  try:self.b.popen([str(self.root/'player')],env={'A':'B'})
  finally:self.b.restore()
  self.assertEqual(self.b.calls[0][1]['env'],{'A':'B','MNM_SCENE_ACTORS':'1'});self.assertEqual(len(self.b.pose_launches),1)
 def test_other_process_environment_unchanged(self):
  self.b.install()
  try:self.b.popen(['tool'],env={'A':'B'})
  finally:self.b.restore()
  self.assertEqual(self.b.calls[0][1]['env'],{'A':'B'});self.assertFalse(self.b.pose_launches)
 def test_conflicting_flag_refuses(self):
  self.b.install()
  try:
   with self.assertRaises(ValueError):self.b.popen([str(self.root/'player')],env={'MNM_SCENE_ACTORS':'0'})
  finally:self.b.restore()
 def test_receipt_requires_actual_binding_and_flag(self):
  with self.assertRaises(ValueError):self.b.receipt()
  self.b.patch(self.interaction);self.b.patch(self.picking);self.b.pose_launches=[dict(flag='MNM_SCENE_ACTORS',value='1')]
  r=self.b.receipt()['picking_pose_target'];self.assertEqual((r['quiet_source_samples'],r['exact_displayed_witnesses'],r['readiness_seconds']),(3,3,8));self.assertFalse(r['original_memory_writes']);self.assertTrue(r['normal_retirement_required'])
 def test_assessment_changes_only_exact_target_replay(self):
  import inspect,ast
  generated=with_actor_frames({});self.assertEqual(generated.__code__.co_argcount,frozen.assess.__code__.co_argcount)
  self.assertIn('actor_frames',generated.__code__.co_names);self.assertNotIn('actor_frames',frozen.assess.__code__.co_names)
 def test_factory_does_not_replace_frozen_assessor(self):
  original=frozen.assess;code=original.__code__;with_actor_frames({})
  self.assertIs(frozen.assess,original);self.assertIs(frozen.assess.__code__,code)
if __name__=='__main__':unittest.main()
