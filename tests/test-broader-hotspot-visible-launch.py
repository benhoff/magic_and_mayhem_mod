import sys,types,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_hotspot_visible_launch import WorldBinding,BaseWorldBinding,VisibleMenus

class HotspotVisibleBinding(unittest.TestCase):
    def binding(self,family='hotspot'):
        result=WorldBinding.__new__(WorldBinding);result.plan={'family':family};return result
    def test_only_hotspot_menu_module_uses_visible_reader(self):
        node=types.SimpleNamespace(__file__='/owned/tools/run-rendering-menu-acceptance.py',AccessibleMenus=object)
        with patch.object(BaseWorldBinding,'patch',lambda self,module:module):
            self.assertIs(self.binding().patch(node).AccessibleMenus,VisibleMenus)
    def test_controller_module_is_unchanged(self):
        sentinel=object();node=types.SimpleNamespace(__file__='/owned/tools/run-rendering-cursor-hotspot.py',AccessibleMenus=sentinel)
        with patch.object(BaseWorldBinding,'patch',lambda self,module:module):
            self.assertIs(self.binding().patch(node).AccessibleMenus,sentinel)
    def test_base_patch_result_is_used(self):
        initial=types.SimpleNamespace(__file__='initial.py');result=types.SimpleNamespace(__file__='run-rendering-menu-acceptance.py')
        with patch.object(BaseWorldBinding,'patch',return_value=result)as parent:
            self.assertIs(self.binding().patch(initial),result);parent.assert_called_once_with(initial)
        self.assertIs(result.AccessibleMenus,VisibleMenus)
    def test_non_hotspot_family_refuses_before_any_patch(self):
        with patch.object(BaseWorldBinding,'patch')as parent:
            for family in ('font','caller','palette','defeat','hotspot-extra',None):
                with self.subTest(family=family),self.assertRaises(ValueError):self.binding(family).patch(types.SimpleNamespace())
            parent.assert_not_called()
    def test_missing_family_refuses(self):
        binding=self.binding();binding.plan={}
        with patch.object(BaseWorldBinding,'patch')as parent,self.assertRaises(KeyError):binding.patch(types.SimpleNamespace())
        parent.assert_not_called()
    def test_receipt_preserves_parent_and_declares_no_action_or_limit_change(self):
        original=dict(original_marker={'retained':True})
        with patch.object(BaseWorldBinding,'receipt',return_value=dict(original)):
            result=self.binding().receipt()
        self.assertEqual(result['original_marker'],original['original_marker']);proof=result['hotspot_accessibility_observation']
        self.assertFalse(proof['cached_observations']);self.assertFalse(proof['actions_changed']);self.assertFalse(proof['deadlines_changed']);self.assertFalse(proof['native_code_or_limits_changed'])
    def test_existing_collector_body_is_exact_except_import_and_family_guard(self):
        root=Path(__file__).resolve().parents[1];old=(root/'tools/run-broader-visible-world.py').read_text();new=(root/'tools/run-broader-hotspot-visible-world.py').read_text()
        normalized=new.replace('from broader_hotspot_visible_launch import WorldBinding','from broader_visible_world_launch import WorldBinding').replace("        require(plan.get('family')=='hotspot','Hotspot-visible collector requires the hotspot family')\n",'')
        self.assertEqual(normalized,old)
if __name__=='__main__':unittest.main()
