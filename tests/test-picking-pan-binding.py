import sys,types,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pan_launch import binding_class
from picking_closeout import quiet_sources
from rendering_picking_resize import ready,targets
class Controls(unittest.TestCase):
 def bind(self):
  class Base:
   def __init__(self):self.plan=dict(family='picking');self.popen=lambda:None
   def patch(self,m):return m
  class Journey:
   def __init__(self):self.forwarded=[]
   def latest(self):return None
   def held_key(self,*args):self.forwarded.append(args);return 'original'
  interaction=types.SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=Journey);camera=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',LiveViewport=object);ns=dict(interaction=interaction)
  exec('''def main():
 class PickingJourney(interaction.InteractionJourney):
  def rebuild(self,low):
   before=self.latest()
   after=self.latest()
 return PickingJourney
''',ns)
  controller=types.SimpleNamespace(__file__='run-rendering-picking-closeout.py',main=ns['main'],targets=targets,ready=ready,quiet_sources=quiet_sources);b=binding_class(Base)()
  for m in[camera,interaction,controller]:b.patch(m);b.patch(m)
  return b,interaction.InteractionJourney()
 def test_recursive_idempotent_binding(self):
  b,j=self.bind();self.assertEqual(len(b.pan_modules),1);self.assertEqual(b.pan_journeys,[j])
 def test_non_pan_actions_delegate_unchanged(self):
  b,j=self.bind();self.assertEqual(j.held_key('high-initial-sharp-rotate-0',ord('.'),.22,.65),'original');self.assertEqual(j.forwarded,[('high-initial-sharp-rotate-0',ord('.'),.22,.65,True)])
 def test_only_declared_pan_uses_bounded_observer(self):
  b,j=self.bind()
  with patch('broader_picking_pan_launch.perform',return_value='ack')as observed:self.assertEqual(j.held_key('low-integer-pan',0xff52,.22,.8),'ack');observed.assert_called_once_with(j,'low-integer-pan',0xff52,.22,.8,True)
  self.assertFalse(j.forwarded)
if __name__=='__main__':unittest.main()
