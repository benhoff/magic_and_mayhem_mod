import sys,types,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_visible_accessibility import VisibleMenus
from rendering_menu_accessibility import AccessibleMenus
class Node:
 def __init__(self,name,children=(),visible=True,pid=17):self.name=name;self.children=list(children);self.visible=visible;self.pid=pid;self.visits=0
 def get_process_id(self):return self.pid
 def get_state_set(self):self.visits+=1;return self
 def contains(self,state):return self.visible if state=='showing'else state=='enabled'
 def get_component_iface(self):return self
 def get_extents(self,c):return types.SimpleNamespace(x=1,y=2,width=30,height=40)
 def get_name(self):return self.name
 def get_role_name(self):return getattr(self,'role','panel')
 def get_accessible_id(self):return 'test.'+self.name
 def get_value_iface(self):return None
 def get_child_count(self):return len(self.children)
 def get_child_at_index(self,i):return self.children[i]
def reader(apps):
 r=VisibleMenus.__new__(VisibleMenus);r.pid=17;r.a=types.SimpleNamespace(get_desktop=lambda _:Node('desktop',apps),StateType=types.SimpleNamespace(SHOWING='showing',FOCUSED='focused',ENABLED='enabled'),CoordType=types.SimpleNamespace(SCREEN=0));return r
class VisibleTraversal(unittest.TestCase):
 def test_exact_dfs_visible_properties_and_order(self):
  r=reader([Node('app',[Node('window',[Node('one'),Node('hidden',[Node('also-hidden',visible=False)],False),Node('two')])],False)])
  self.assertEqual([v for _,v in r.nodes()],[v for _,v in AccessibleMenus.nodes(r)])
 def test_hidden_application_still_traversed(self):self.assertEqual([v['name']for _,v in reader([Node('app',[Node('window')],False)]).nodes()],['window'])
 def test_hidden_descendant_is_not_queried(self):
  leaf=Node('unobserved',visible=False);r=reader([Node('app',[Node('hidden',[leaf],False)])]);r.nodes();self.assertEqual(leaf.visits,0)
 def test_hidden_combo_list_items_are_preserved(self):
  n=Node('popup',[Node('item')],False);n.role='list';r=reader([Node('app',[n],False)]);self.assertEqual([v for _,v in r.nodes()],[v for _,v in AccessibleMenus.nodes(r)])
 def test_no_cross_poll_visibility_cache(self):
  n=Node('page',visible=False);r=reader([Node('app',[n],False)]);self.assertEqual(r.nodes(),[]);n.visible=True;self.assertEqual(len(r.nodes()),1)
 def test_other_process_excluded(self):self.assertEqual(reader([Node('other',pid=18)]).nodes(),[])
 def test_ambiguous_process_refuses(self):
  with self.assertRaises(ValueError):reader([Node('a'),Node('b')]).nodes()
 def test_cycle_hits_visited_budget(self):
  n=Node('cycle');n.children=[n];r=reader([n])
  with self.assertRaises(ValueError):r.nodes()
if __name__=='__main__':unittest.main()
