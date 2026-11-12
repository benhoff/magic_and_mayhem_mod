#!/usr/bin/env python3
"""Reject missing public widget/host focus before admitting physical Escape."""
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from pause_focus import cancel_focus,key_down
class Node:
 def __init__(self,allowed=True):self.allowed=allowed;self.requests=0
 def get_component_iface(self):return self
 def grab_focus(self):self.requests+=1;return self.allowed
class Access:
 def __init__(self,focused=(False,True),enabled=True,showing=True,allowed=True):self.node=Node(allowed);self.values=list(focused);self.enabled=enabled;self.showing=showing
 def nodes(self):
  focus=self.values.pop(0) if len(self.values)>1 else self.values[0]
  return [(self.node,dict(accessible_id='root.miniMenuButton5',focused=focus,enabled=self.enabled,showing=self.showing,rect=[1,2,3,4]))]
class Host:
 def __init__(self,focused=True):self.focused=focused;self.activated=False
 def activate(self):self.activated=True
 def focus(self):return dict(host_focused=self.activated and self.focused,window=7,host_window=7)
class Focus(unittest.TestCase):
 def test_request_alone_waits_for_actual_widget_and_host(self):
  a=Access();h=Host();q=cancel_focus(a,h);self.assertTrue(h.activated);self.assertEqual(a.node.requests,1);self.assertTrue(q['focused'] and q['host_focus']['host_focused'])
 def test_refused_hidden_disabled_unfocused_or_wrong_host_fail(self):
  for a,h in [(Access(allowed=False),Host()),(Access(enabled=False),Host()),(Access(showing=False),Host()),(Access(focused=(False,)),Host()),(Access(focused=(True,)),Host(False))]:
   with self.subTest(a=a,h=h),self.assertRaises(ValueError):cancel_focus(a,h,timeout=.015)
 def test_actual_physical_keymap_bounds_and_edges(self):
  raw=bytearray(32);self.assertFalse(key_down(raw,9));raw[1]=2;self.assertTrue(key_down(raw,9));self.assertFalse(key_down(raw,8))
  for data,code in [(b'bad',9),(raw,True),(raw,7),(raw,256)]:
   with self.assertRaises(ValueError):key_down(data,code)
if __name__=='__main__':unittest.main()
