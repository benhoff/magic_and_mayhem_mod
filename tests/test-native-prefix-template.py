import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import native_prefix_template as tool


class Prefix(unittest.TestCase):
    def fixture(self,root):
        source=root/'template';source.mkdir();target=root/'working/test/wineprefix';target.parent.mkdir(parents=True)
        (source/'.mnm-directplay-dx6.json').write_text(json.dumps(tool.prefix_identity(source)))
        (source/'system.reg').write_bytes(b'owned system');(source/'user.reg').write_bytes(b'owned user')
        return source,target

    def test_clone_rebinds_only_new_owner_and_preserves_template_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);source,target=self.fixture(root)
            before={p.name:p.read_bytes() for p in source.iterdir()};run=subprocess.run
            def child(command,**kwargs):
                return subprocess.CompletedProcess(command,0) if command[0]=='wineserver' else run(command,**kwargs)
            with patch.object(tool,'ROOT',root),patch.object(tool,'check_prefix') as check,patch.object(tool.subprocess,'run',side_effect=child):
                result=tool.clone(source,target,{})
            self.assertTrue(result['stopped_before_copy']);self.assertEqual(check.call_count,4)
            self.assertEqual({p.name:p.read_bytes() for p in source.iterdir()},before)
            self.assertEqual(json.loads((target/'.mnm-directplay-dx6.json').read_text()),tool.prefix_identity(target))
            self.assertEqual((target/'system.reg').read_bytes(),before['system.reg'])

    def test_foreign_wrong_or_redirected_owner_and_existing_target_refuse_before_launch(self):
        for kind in ('missing','wrong','redirected','target','outside'):
            with self.subTest(kind=kind),tempfile.TemporaryDirectory() as temporary:
                root=Path(temporary);source,target=self.fixture(root);marker=source/'.mnm-directplay-dx6.json'
                if kind=='missing':marker.unlink()
                if kind=='wrong':marker.write_text('{}')
                if kind=='redirected':marker.rename(source/'marker');marker.symlink_to(source/'marker')
                if kind=='target':target.mkdir()
                if kind=='outside':target=root/'outside'
                with patch.object(tool,'ROOT',root),patch.object(tool.subprocess,'run') as run,self.assertRaises(ValueError):tool.clone(source,target,{})
                run.assert_not_called()

    def test_active_or_changed_binary_refuses_without_rebinding(self):
        for kind in ('active','binary'):
            with self.subTest(kind=kind),tempfile.TemporaryDirectory() as temporary:
                root=Path(temporary);source,target=self.fixture(root)
                with patch.object(tool,'ROOT',root),patch.object(tool,'check_prefix',side_effect=ValueError('bad binary') if kind=='binary' else None),patch.object(tool.subprocess,'run',side_effect=subprocess.TimeoutExpired('wineserver',5)):
                    with self.assertRaises((ValueError,subprocess.TimeoutExpired)):tool.clone(source,target,{})
                self.assertFalse(target.exists())


if __name__=='__main__':unittest.main()
