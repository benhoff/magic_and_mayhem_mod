"""Refuse source drift and unintended child/metadata changes in current bindings."""
from pathlib import Path
import sqlite3
import sys
import tempfile
import types
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_worker_world_launch import WorldBinding,current_checkout_metadata

SOURCE=b'''def finish(result,actions):
    actions.append('preserve')
    result['source_commit']='actual-head'
    result['source_merge_parent']=subprocess.check_output(['git','rev-parse','MERGE_HEAD'],cwd=ROOT,text=True).strip()
    actions.append('quit')
    return result
'''


class WorldLaunchGuards(unittest.TestCase):
    def binding(self,root,provider='nvidia'):
        return WorldBinding(root,dict(player_binary=dict(path='player'),counter_bindings={},controller_modules=['control.py'],collector_prefix='research/current/'),'case',provider,'plan')

    def test_metadata_preserves_actions_and_actual_head(self):
        code,line=current_checkout_metadata(SOURCE,'collector.py');namespace={}
        exec(code,namespace);actions=[];result=namespace['finish']({},actions)
        self.assertEqual(actions,['preserve','quit']);self.assertEqual(result,dict(source_commit='actual-head',source_merge_parent=None));self.assertEqual(line,4)

    def test_changed_metadata_expression_refuses(self):
        with self.assertRaisesRegex(ValueError,'Unreviewed'):
            current_checkout_metadata(SOURCE.replace(b'MERGE_HEAD',b'HEAD'),'collector.py')

    def test_missing_metadata_anchor_refuses(self):
        with self.assertRaisesRegex(ValueError,'Missing/duplicate'):
            current_checkout_metadata(SOURCE.replace(b'source_merge_parent',b'other'),'collector.py')

    def test_duplicate_metadata_anchor_refuses(self):
        with self.assertRaisesRegex(ValueError,'Missing/duplicate'):
            current_checkout_metadata(SOURCE+SOURCE.replace(b'finish',b'other'),'collector.py')

    def test_real_player_alias_receives_only_declared_provider(self):
        import subprocess
        with tempfile.TemporaryDirectory()as directory:
            root=Path(directory);(root/'tools').mkdir();real=root/'actual';real.write_bytes(b'only-player');(root/'player').symlink_to(real)
            calls=[]
            with patch('subprocess.Popen',side_effect=lambda *a,**k:calls.append((a,k))):
                b=self.binding(root);b.install()
                try:subprocess.Popen([str(real),'--same-argument'],env={'LIBGL_ALWAYS_SOFTWARE':'1','DISPLAY':':175'})
                finally:b.restore()
            self.assertEqual(calls[0][0][0],[str(real),'--same-argument']);self.assertEqual(calls[0][1]['env']['GALLIUM_DRIVER'],'zink');self.assertNotIn('LIBGL_ALWAYS_SOFTWARE',calls[0][1]['env']);self.assertEqual(len(b.environments),1)

    def test_shell_launch_is_reported_as_launcher_environment(self):
        import subprocess
        with tempfile.TemporaryDirectory()as directory:
            root=Path(directory);calls=[]
            with patch('subprocess.Popen',side_effect=lambda *a,**k:calls.append((a,k))):
                b=self.binding(root,'mesa');b.install()
                try:subprocess.Popen([str(root/'run-native.sh'),'--literal'],env={'DISPLAY':':175'})
                finally:b.restore()
            receipt=b.receipt();self.assertFalse(receipt['actual_qt_environment_directly_sampled']);self.assertNotIn('actual_qt_environments',receipt);self.assertEqual(receipt['actual_public_launcher_environments'][0]['DISPLAY'],':175')

    def test_nested_direct_loader_gets_only_owned_plan_bindings(self):
        with tempfile.TemporaryDirectory()as directory:
            root=Path(directory);(root/'tools').mkdir();(root/'tools/control.py').write_text("PREFIX='historical/'\nPLAN='old'\nvalue=17\n")
            b=self.binding(root,'mesa');b.install()
            try:module=b.load('control.py')
            finally:b.restore()
            self.assertEqual((module.PREFIX,module.PLAN,module.value),('research/current/','plan',17))

    def test_scene_attachment_uses_explicit_build_only_record(self):
        with tempfile.TemporaryDirectory()as directory:
            root=Path(directory);(root/'tools').mkdir();file=root/'tools/run-scene-attachment.py'
            file.write_text("PLAYER='historical-game'\n")
            plan=dict(player_binary=dict(path='player'),counter_bindings={},controller_modules=[file.name],collector_prefix='research/current/',current_player_build_binding=dict(record='current-build-only'))
            b=WorldBinding(root,plan,'case','mesa','plan');b.install()
            try:module=b.load(file.name)
            finally:b.restore()
            self.assertEqual(module.PLAYER,'current-build-only')

    def test_restore_returns_original_global_factories(self):
        import importlib.util,subprocess
        original=(importlib.util.spec_from_file_location,subprocess.Popen,sqlite3.connect)
        with tempfile.TemporaryDirectory()as directory:
            b=self.binding(Path(directory),'mesa');b.install();b.restore()
        self.assertEqual(original,(importlib.util.spec_from_file_location,subprocess.Popen,sqlite3.connect))


if __name__=='__main__':unittest.main()
