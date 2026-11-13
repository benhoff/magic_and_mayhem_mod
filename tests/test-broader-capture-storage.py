"""Capture path and process retirement guards; no game or capture policy change."""
import sys,tempfile,unittest
from types import SimpleNamespace
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_capture_storage import CaptureStorage,CaptureProcess

class Process:
    def __init__(self):self.returncode=None;self.pid=42
    def poll(self):return self.returncode
    def wait(self):self.returncode=0;return 0

class Storage(unittest.TestCase):
    def test_disabled_and_unrelated_environment_untouched(self):
        for enabled,env in[(False,{'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':'/unrelated'}),(True,{})]:
            s=CaptureStorage('/task','case',enabled);before=dict(env);self.assertFalse(s.prepare(env));self.assertEqual(env,before);self.assertEqual(s.entries,[])
    def test_only_closed_process_triggers_copy(self):
        p=Process();calls=[]
        class Copy:
            def copy_closed(self):calls.append('closed')
        wrapped=CaptureProcess(p,Copy());self.assertIsNone(wrapped.poll());self.assertEqual(calls,[])
        self.assertEqual(wrapped.wait(),0);self.assertEqual(calls,['closed']);self.assertEqual(wrapped.pid,42)
    def test_external_existing_and_aliased_paths_refuse(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);(root/'working').mkdir();(root/'working/out').mkdir();logical=root/'working/out/capture'
            s=CaptureStorage(root,'case',True)
            with self.assertRaises(ValueError):s.prepare({'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(root/'capture')})
            logical.mkdir()
            with self.assertRaises(ValueError):s.prepare({'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(logical)})
            logical.rmdir();(root/'working/alias').symlink_to(root/'working/out',target_is_directory=True)
            with self.assertRaises(ValueError):s.prepare({'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(root/'working/alias/capture')})
    def test_unclosed_copy_cannot_get_receipt(self):
        s=CaptureStorage('/task','case',True);s.entries=[dict(copied=False)]
        with self.assertRaises(ValueError):s.receipt()
    def test_copy_rejects_replaced_logical_alias(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);logical=root/'capture';physical=root/'ram';logical.mkdir();physical.mkdir()
            s=CaptureStorage(root,'case',True);s.entries=[dict(logical=str(logical),physical=str(physical),copied=False)]
            with self.assertRaises(ValueError):s.copy_closed()
    def test_exact_closed_copy_keeps_environment_and_ram_bytes(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);out=root/'working/out';out.mkdir(parents=True);base=root/'ram';base.mkdir()
            logical=out/'capture';environment={'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(logical)};before=dict(environment)
            storage=CaptureStorage(root,'case',True,base=base)
            with patch('broader_capture_storage.shutil.disk_usage',return_value=SimpleNamespace(free=200*1024**3)):
                self.assertTrue(storage.prepare(environment));physical=Path(storage.entries[0]['physical'])
                (physical/'commands-1.bin').write_bytes(b'exact closed fixture bytes')
                wrapped=storage.wrap(Process());self.assertIsNone(wrapped.poll());self.assertTrue(logical.is_symlink())
                self.assertEqual(wrapped.wait(),0)
            self.assertEqual(environment,before);self.assertFalse(logical.is_symlink())
            self.assertEqual((logical/'commands-1.bin').read_bytes(),(physical/'commands-1.bin').read_bytes())
            self.assertTrue((out/'capture.ram-alias').is_symlink());self.assertTrue(storage.receipt()['captured_bytes_preserved'])
    def test_non_tmpfs_and_low_headroom_refuse_before_creating_capture(self):
        for reason in('filesystem','headroom'):
            with tempfile.TemporaryDirectory()as d:
                root=Path(d);out=root/'working/out';out.mkdir(parents=True);base=root/'ram';base.mkdir()
                storage=CaptureStorage(root,'case',True,base=base)
                with patch('broader_capture_storage.subprocess.check_output',return_value='ext4'if reason=='filesystem'else'tmpfs'),patch('broader_capture_storage.shutil.disk_usage',return_value=SimpleNamespace(free=7*1024**3)):
                    with self.assertRaises(ValueError):storage.prepare({'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(out/'capture')})
                self.assertFalse((out/'capture').exists());self.assertEqual(storage.entries,[])
    def test_cold_reserve_refusal_keeps_original_alias_and_bytes(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);out=root/'working/out';out.mkdir(parents=True);base=root/'ram';base.mkdir()
            storage=CaptureStorage(root,'case',True,base=base);logical=out/'capture'
            with patch('broader_capture_storage.shutil.disk_usage',return_value=SimpleNamespace(free=200*1024**3)):storage.prepare({'MNM_ACCEPTANCE_CAPTURE_DIRECTORY':str(logical)})
            physical=Path(storage.entries[0]['physical']);(physical/'capture.bin').write_bytes(b'held')
            with patch('broader_capture_storage.shutil.disk_usage',return_value=SimpleNamespace(free=100*1024**3)):
                with self.assertRaises(ValueError):storage.copy_closed()
            self.assertTrue(logical.is_symlink());self.assertEqual((physical/'capture.bin').read_bytes(),b'held');self.assertFalse(storage.entries[0]['copied'])
    def test_no_wrapper_without_capture(self):
        s=CaptureStorage('/task','case',True);p=Process();self.assertIs(s.wrap(p),p)

if __name__=='__main__':unittest.main()
