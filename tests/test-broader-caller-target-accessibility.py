"""Target queries must equal filtering a complete live visible observation."""
import importlib.util,sys,types,unittest
from pathlib import Path
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from broader_caller_target_accessibility import TargetMenus
from broader_visible_accessibility import VisibleMenus
spec=importlib.util.spec_from_file_location('visible_controls',R/'tests/test-broader-visible-accessibility.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
Node=base.Node
class TargetNode(Node):
 def __init__(self,*a,enabled=True,focused=False,role='button',**kw):super().__init__(*a,**kw);self.enabled=enabled;self.focused=focused;self.role=role;self.described=0
 def contains(self,s):return {'showing':self.visible,'enabled':self.enabled,'focused':self.focused}[s]
 def get_extents(self,c):self.described+=1;return super().get_extents(c)
def reader(nodes):
 r=base.reader(nodes);r.__class__=TargetMenus;return r
class TargetObservation(unittest.TestCase):
 def test_exact_target_dictionary_order(self):
  a=TargetNode('mainMenuAction2',focused=True);b=TargetNode('other');r=reader([Node('app',[a,b],False)])
  expected=[v for _,v in VisibleMenus.nodes(r)if v['accessible_id'].endswith('.mainMenuAction2')]
  self.assertEqual([v for _,v in r.targets('mainMenuAction2')],expected)
 def test_unrelated_geometry_is_not_queried(self):
  a=TargetNode('mainMenuAction2');b=TargetNode('other');reader([Node('app',[a,b],False)]).find('Quick Battle','button');self.assertEqual((a.described,b.described),(1,0))
 def test_hidden_descendants_pruned(self):
  a=TargetNode('mainMenuAction2');r=reader([Node('app',[Node('hidden',[a],False)],False)]);self.assertEqual(r.targets('mainMenuAction2'),[]);self.assertEqual(a.visits,0)
 def test_hidden_list_descendant_retained(self):
  a=TargetNode('mainMenuAction2');listing=Node('list',[a],False);listing.role='list';r=reader([Node('app',[listing],False)]);self.assertIs(r.find('Quick Battle','button')[0],a)
 def test_ambiguity_refuses(self):
  r=reader([Node('app',[TargetNode('mainMenuAction2'),TargetNode('mainMenuAction2')],False)])
  with self.assertRaises(ValueError):r.find('Quick Battle','button')
 def test_disabled_target_stays_disabled(self):
  r=reader([Node('app',[TargetNode('mainMenuAction2',enabled=False)],False)]);self.assertFalse(r.find('Quick Battle','button')[1]['enabled'])
 def test_role_filter(self):
  r=reader([Node('app',[TargetNode('mainMenuAction2',role='label')],False)]);self.assertEqual(r.targets('mainMenuAction2','button'),[])
 def test_no_visibility_cache(self):
  a=TargetNode('mainMenuAction2');r=reader([Node('app',[a],False)]);self.assertEqual(len(r.targets('mainMenuAction2')),1);a.visible=False;self.assertEqual(r.targets('mainMenuAction2'),[])
 def test_no_geometry_or_enabled_cache(self):
  a=TargetNode('mainMenuAction2');r=reader([Node('app',[a],False)]);r.find('Quick Battle','button');a.enabled=False;self.assertFalse(r.find('Quick Battle','button')[1]['enabled']);self.assertEqual(a.described,2)
 def test_name_fallback_preserved(self):
  a=TargetNode('fallback');r=reader([Node('app',[a],False)]);self.assertIs(r.find('fallback','button')[0],a)
 def test_other_process_excluded(self):self.assertEqual(reader([TargetNode('mainMenuAction2',pid=18)]).targets('mainMenuAction2'),[])
 def test_ambiguous_process_refuses(self):
  with self.assertRaises(ValueError):reader([Node('a'),Node('b')]).targets('mainMenuAction2')
 def test_cycle_budget(self):
  n=Node('cycle');n.children=[n]
  with self.assertRaises(ValueError):reader([n]).targets('mainMenuAction2')
if __name__=='__main__':unittest.main()
