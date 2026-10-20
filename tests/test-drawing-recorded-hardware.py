from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_recorded_session import backend_environment
from native_render_driver import require_nvidia


class Backend(unittest.TestCase):
    def test_default_preserves_software_request_and_does_not_mutate_parent(self):
        parent=dict(DISPLAY=':owned',LIBGL_ALWAYS_SOFTWARE='0',OTHER='kept')
        result=backend_environment(inherited=parent)
        self.assertEqual(result['LIBGL_ALWAYS_SOFTWARE'],'1');self.assertEqual(result['OTHER'],'kept')
        self.assertEqual(parent['LIBGL_ALWAYS_SOFTWARE'],'0')

    def test_explicit_hardware_removes_only_software_force_and_keeps_owned_display_driver(self):
        parent=dict(DISPLAY=':owned',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='zink')
        result=backend_environment(hardware=True,inherited=parent)
        self.assertNotIn('LIBGL_ALWAYS_SOFTWARE',result)
        self.assertEqual(result['DISPLAY'],':owned');self.assertEqual(result['GALLIUM_DRIVER'],'zink')
        self.assertEqual(parent['LIBGL_ALWAYS_SOFTWARE'],'1')

    def test_software_after_hardware_clears_driver_selectors_without_changing_parent(self):
        selectors = dict(MESA_LOADER_DRIVER_OVERRIDE='zink', GALLIUM_DRIVER='zink',
                         __GLX_VENDOR_LIBRARY_NAME='nvidia',
                         __EGL_VENDOR_LIBRARY_FILENAMES='/nvidia.json',
                         QT_XCB_GL_INTEGRATION='xcb_glx', MNM_PROFILE_EGL_VENDOR='NVIDIA')
        parent = dict(DISPLAY=':owned', OTHER='kept', **selectors)
        result = backend_environment(inherited=parent)
        self.assertEqual(result['LIBGL_ALWAYS_SOFTWARE'], '1')
        self.assertEqual((result['DISPLAY'], result['OTHER']), (':owned', 'kept'))
        self.assertTrue(all(key not in result for key in selectors))
        self.assertTrue(all(parent[key] == value for key, value in selectors.items()))

    def test_hardware_without_display_or_with_xvfb_provisioning_refuses(self):
        for args in (dict(hardware=True,inherited={}),dict(hardware=True,headless=True,inherited={'DISPLAY':':owned'}),dict(hardware=1),dict(headless='yes')):
            with self.subTest(args=args),self.assertRaises(ValueError):backend_environment(**args)

    def test_both_executed_backend_identities_are_required(self):
        for drivers in (['NVIDIA','llvmpipe'],['llvmpipe','NVIDIA'],['NVIDIA',None],['NVIDIA','NVIDIA software']):
            with self.assertRaises(ValueError):require_nvidia(drivers)
        self.assertTrue(require_nvidia(['NVIDIA TITAN RTX','zink (NVIDIA TITAN RTX)'])['verified'])


if __name__=='__main__':unittest.main()
