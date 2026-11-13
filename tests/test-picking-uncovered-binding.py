import importlib.util,sys,time,types,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_uncovered_launch import binding_class,quiet_sources
from broader_picking_uncovered_target import POLICY,targets
from broader_picking_uncovered_assessment import source_assessment
from broader_picking_pose_assessment import with_actor_frames as pose_assessment
from broader_picking_pose_target import PosedRows
from rendering_picking_resize import targets as original_targets,ready
from picking_closeout import quiet_sources as original_quiet
s=importlib.util.spec_from_file_location('source_pose_fixture',Path(__file__).with_name('test-picking-pose-target.py'));fixture=importlib.util.module_from_spec(s);s.loader.exec_module(fixture)
class Controls(unittest.TestCase):
 def bind(self):
  class Base:
   def __init__(self):self.plan=dict(family='picking');self.popen=lambda:None
   def patch(self,m):return m
  class Journey:
   def latest(self):return None
  camera=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',LiveViewport=object)
  interaction=types.SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=Journey)
  ns=dict(interaction=interaction)
  exec('''def main():
 class PickingJourney(interaction.InteractionJourney):
  def rebuild(self,low):
   before=self.latest()
   after=self.latest()
 return PickingJourney
''',ns)
  controller=types.SimpleNamespace(__file__='run-rendering-picking-closeout.py',main=ns['main'],targets=original_targets,ready=ready,quiet_sources=original_quiet)
  b=binding_class(Base)()
  for m in[camera,interaction,controller]:b.patch(m);b.patch(m)
  return b,controller
 def args(self):
  a=fixture.case();a[3]['extent']=[800,600];return a
 def samples(self,point):return [dict(generation=i,nonce='1',position=[1,2,3],anchor=[100,200],point=point,view=0)for i in[1,2,3]]
 def test_recursive_idempotent_binding(self):
  b,m=self.bind();self.assertEqual(len(b.uncovered_controllers),1);self.assertIs(m.quiet_sources,quiet_sources)
 def test_original_clear_quiet_predicate_preserved(self):
  rows=self.samples([100,200]);self.assertEqual(quiet_sources(rows),original_quiet(rows))
 def test_unavailable_point_never_quiet(self):self.assertFalse(quiet_sources(self.samples([-1,-1])))
 def test_bound_source_policy_and_cell_preserved(self):
  b,m=self.bind();a=self.args();body,cell=m.targets(PosedRows(a[0],a[3]),*a[1:3]);self.assertEqual(body['target_policy'],POLICY);self.assertEqual((body,cell),targets(*a))
 def test_rebinding_tamper_refuses(self):
  b,m=self.bind();m.targets=lambda:None
  with self.assertRaisesRegex(ValueError,'binding changed'):b.patch(m)
 def test_source_assessment_changes_only_target_global(self):
  frames={};old=pose_assessment(frames);new=source_assessment(frames)
  self.assertEqual(new.__code__.co_code,old.__code__.co_code);self.assertEqual(new.__kwdefaults__,old.__kwdefaults__)
  self.assertIs(new.__globals__['pose_targets'],targets)
  self.assertEqual({k:v for k,v in new.__globals__.items()if k not in ('pose_targets','assess')},{k:v for k,v in old.__globals__.items()if k not in ('pose_targets','assess')})
 def test_wrong_actor_identity_not_swallowed(self):
  b,m=self.bind();a=self.args();a[3]['actors']=[]
  with self.assertRaises(ValueError):m.targets(PosedRows(a[0],a[3]),*a[1:3])
 def test_cell_stays_available_while_body_fully_covered(self):
  import struct
  b,m=self.bind();a=self.args();f,blob=a[0][0];f=list(f);f[8:10]=[100,201];blob=bytearray(blob);struct.pack_into('<I',blob,16,(-2)%2**32);a[0].append((tuple(f),bytes(blob)))
  body,cell=m.targets(PosedRows(a[0],a[3]),*a[1:3]);self.assertEqual(body['point'],[-1,-1]);self.assertEqual(cell['point'],[176,282]);self.assertEqual(b.uncovered_pending_samples,1)
if __name__=='__main__':unittest.main()
