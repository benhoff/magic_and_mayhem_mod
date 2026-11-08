"""Original ACK alone cannot admit physical confirmation input."""
from pathlib import Path
from copy import deepcopy
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_transition_low_live import confirmation_ready
class Confirmation(unittest.TestCase):
 def fixture(self):return [dict(accessible_id='app.nativeGameViewport',showing=True,enabled=True)],dict(ack=6,confirmation=1),dict(active=1,held=[],size=[640,480])
 def test_delayed_publication_and_input_activation(self):
  nodes,original,inp=self.fixture();self.assertFalse(confirmation_ready([],original,inp));inactive=dict(inp,active=0);self.assertFalse(confirmation_ready(nodes,original,inactive));self.assertTrue(confirmation_ready(nodes,original,inp))
 def test_hidden_disabled_held_and_wrong_extent_do_not_admit(self):
  for key in ('showing','enabled','held','size'):
   nodes,o,i=self.fixture()
   if key in ('showing','enabled'):nodes[0][key]=False
   elif key=='held':i[key]=[1]
   else:i[key]=[800,600]
   self.assertFalse(confirmation_ready(nodes,o,i))
 def test_stale_ack_wrong_original_owner_and_duplicate_viewport_refuse(self):
  for kind in ('ack','confirmation','duplicate'):
   nodes,o,i=self.fixture()
   if kind=='duplicate':nodes=nodes+deepcopy(nodes)
   else:o[kind]=5 if kind=='ack' else 0
   with self.assertRaises(ValueError):confirmation_ready(nodes,o,i)
if __name__=='__main__':unittest.main()
