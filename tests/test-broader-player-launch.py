"""Guard real launch selection and counter isolation without starting a game."""
import importlib.util
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_launch import LaunchBinding


class LaunchGuards(unittest.TestCase):
    def binding(self, root, provider='nvidia'):
        return LaunchBinding(root, dict(player_binary=dict(path='player'), counter_bindings={str(root/'old.sqlite3'):'owned.sqlite3'}), 'case', provider)

    def test_no_receipt_without_actual_player(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError,'unique NVIDIA'):
                self.binding(Path(d)).receipt()

    def test_software_rejects_hardware_receipt(self):
        b=self.binding(Path('/task'),'mesa');b.hosts=[{}];b.environments=[{}]
        with self.assertRaisesRegex(ValueError,'software player'):
            b.receipt()

    def test_other_child_environment_is_unchanged(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);calls=[]
            with patch('subprocess.Popen',side_effect=lambda *a,**k:calls.append((a,k))):
                b=self.binding(root);b.install()
                try:
                    import subprocess
                    subprocess.Popen(['other'],env={'TOKEN':'literal'})
                finally:b.restore()
            self.assertEqual(calls[0][1]['env'],{'TOKEN':'literal'});self.assertEqual(b.environments,[])

    def test_only_exact_owned_counter_is_redirected(self):
        import sqlite3
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);b=self.binding(root);b.install()
            try:
                with sqlite3.connect(root/'old.sqlite3') as db:db.execute('create table owned(x)')
                with sqlite3.connect(root/'other.sqlite3') as db:db.execute('create table other(x)')
            finally:b.restore()
            self.assertFalse((root/'old.sqlite3').exists());self.assertTrue((root/'owned.sqlite3').exists());self.assertTrue((root/'other.sqlite3').exists())

    def test_dynamic_owned_factory_receives_provider(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'tools').mkdir();legacy=types.ModuleType('owned');legacy.__file__=str(root/'tools/run-native-owned.py')
            legacy.start_display=lambda path,allow_glx=False:('legacy',path,allow_glx)
            factory=types.ModuleType('factory');factory.__file__=str(root/'tools/controller.py');factory.module=lambda name:legacy
            b=self.binding(root);b.patch(factory)
            with patch('broader_player_launch.start_nvidia',return_value=('server',{'display':':175'},b'cookie')):
                loaded=factory.module('run-native-owned.py')
                self.assertEqual(loaded.start_display('test',allow_glx=True)[0],'server')
                self.assertEqual(loaded.start_display('other',allow_glx=False),('legacy','other',False))
            self.assertEqual(len(b.hosts),1)

    def test_external_factory_is_untouched(self):
        b=self.binding(Path('/task'));m=types.ModuleType('external');m.__file__='/external/controller.py';original=lambda name:None;m.module=original
        self.assertIs(b.patch(m).module,original)

    def test_hardware_profile_binds_authentication_and_driver(self):
        b=self.binding(Path('/task'));b.hosts=[dict(display=':170',authority='/owned/auth')]
        b.environments=[dict(DISPLAY=':170',XAUTHORITY='/other/auth',LIBGL_ALWAYS_SOFTWARE=None,GALLIUM_DRIVER='zink')]
        with self.assertRaisesRegex(ValueError,'environment differs'):
            b.receipt()


if __name__=='__main__':unittest.main()
