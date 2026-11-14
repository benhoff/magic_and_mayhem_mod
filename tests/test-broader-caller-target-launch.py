import ast,importlib.util,inspect,sys,types,unittest
from pathlib import Path
from unittest.mock import patch
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from broader_caller_target_launch import WorldBinding,BaseWorldBinding,TargetMenus,target_click_id,verify_click_body
spec=importlib.util.spec_from_file_location('target_camera_contract',R/'tools/run-rendering-camera-acceptance.py');camera=importlib.util.module_from_spec(spec);spec.loader.exec_module(camera)
class BindingTests(unittest.TestCase):
 def binding(self):
  v=WorldBinding.__new__(WorldBinding);v.plan={'family':'caller'};v.target_menu_bindings=v.target_click_bindings=0;v.target_bound_modules=set();return v
 def test_actual_click_body_AST_equal(self):self.assertTrue(verify_click_body(camera.CameraJourney.click_id))
 def test_menu_observer_bound_once(self):
  b=self.binding();m=types.SimpleNamespace(__file__='run-rendering-menu-acceptance.py',AccessibleMenus=TargetMenus.__bases__[0])
  with patch.object(BaseWorldBinding,'patch',side_effect=lambda m:m):self.assertIs(b.patch(m).AccessibleMenus,TargetMenus)
  self.assertEqual(b.target_menu_bindings,1)
 def test_camera_subclass_preserves_other_methods(self):
  b=self.binding();m=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',CameraJourney=camera.CameraJourney)
  with patch.object(BaseWorldBinding,'patch',side_effect=lambda m:m):b.patch(m)
  self.assertTrue(issubclass(m.CameraJourney,camera.CameraJourney));self.assertIs(m.CameraJourney.wait_progress,camera.CameraJourney.wait_progress);self.assertIs(m.CameraJourney.click_id,target_click_id)
 def test_other_modules_unchanged(self):
  b=self.binding();m=types.SimpleNamespace(__file__='other.py')
  with patch.object(BaseWorldBinding,'patch',side_effect=lambda m:m):self.assertIs(b.patch(m),m)
  self.assertEqual((b.target_menu_bindings,b.target_click_bindings),(0,0))
 def test_wrong_menu_class_refuses(self):
  b=self.binding();m=types.SimpleNamespace(__file__='run-rendering-menu-acceptance.py',AccessibleMenus=object)
  with patch.object(BaseWorldBinding,'patch',side_effect=lambda m:m),self.assertRaises(ValueError):b.patch(m)
 def test_repeated_camera_patch_is_idempotent(self):
  b=self.binding();m=types.SimpleNamespace(__file__='run-rendering-camera-acceptance.py',CameraJourney=camera.CameraJourney)
  with patch.object(BaseWorldBinding,'patch',side_effect=lambda m:m):
   b.patch(m);first=m.CameraJourney;b.patch(m);b.patch(m)
  self.assertIs(m.CameraJourney,first);self.assertEqual(b.target_click_bindings,1)
 def test_parent_menu_reset_restores_target_observer(self):
  b=self.binding();m=types.SimpleNamespace(__file__='run-rendering-menu-acceptance.py',AccessibleMenus=TargetMenus.__bases__[0])
  def parent(m):m.AccessibleMenus=TargetMenus.__bases__[0];return m
  with patch.object(BaseWorldBinding,'patch',side_effect=parent):
   b.patch(m);b.patch(m);b.patch(m)
  self.assertIs(m.AccessibleMenus,TargetMenus);self.assertEqual(b.target_menu_bindings,1)
 def test_disabled_control_refuses_without_click(self):
  j=types.SimpleNamespace(access=types.SimpleNamespace(targets=lambda n:[(None,{'accessible_id':'test.'+n,'enabled':False})]))
  with self.assertRaises(ValueError):target_click_id(j,'control')
 def test_duplicate_controls_refuse(self):
  j=types.SimpleNamespace(access=types.SimpleNamespace(targets=lambda n:[(None,{'accessible_id':'test.'+n,'enabled':True})]*2))
  with self.assertRaises(ValueError):target_click_id(j,'control')
 def test_physical_click_receipt_retained(self):
  rect=[1,2,3,4];j=types.SimpleNamespace(access=types.SimpleNamespace(targets=lambda n:[(None,{'accessible_id':'test.'+n,'enabled':True,'rect':rect})]),input=types.SimpleNamespace(click=lambda r:{'actual':r}),actions=[]);target_click_id(j,'control');self.assertEqual(j.actions[0]['physical'],{'actual':rect});self.assertEqual(j.actions[0]['object'],'control')
if __name__=='__main__':unittest.main()
