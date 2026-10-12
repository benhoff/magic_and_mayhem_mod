import fcntl,json,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_fixture_prefix import prepare


class Prefix(unittest.TestCase):
    def test_reuse_requires_owned_suite_marker_and_exclusive_idle(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prefix=root/'working/tests/drawing-fixture-prefix-suite/run-owned/wineprefix';prefix.mkdir(parents=True)
            marker=prefix.parent/'fixture-prefix-owner.json'
            with self.assertRaises(FileNotFoundError):prepare(root,root,root,prefix)
            marker.write_text(json.dumps(dict(schema=1,root=str(root),scope='serial-owned-fixtures-only')))
            with patch('drawing_fixture_prefix.subprocess.run') as run:
                actual,lock=prepare(root,root,root,prefix);self.assertEqual(actual,prefix)
                self.assertEqual(run.call_args.args[0],['wineserver','-w']);self.assertEqual(run.call_args.kwargs['timeout'],2)
                with self.assertRaises(BlockingIOError):prepare(root,root,root,prefix)
                lock.close();actual,lock=prepare(root,root,root,prefix);lock.close()
            marker.write_text(json.dumps(dict(schema=1,root='/foreign',scope='serial-owned-fixtures-only')))
            with self.assertRaises(ValueError):prepare(root,root,root,prefix)

    def test_game_and_external_prefix_refused(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prefix=root/'working/experiments/game/wineprefix';prefix.mkdir(parents=True)
            with self.assertRaises(ValueError):prepare(root,root,root,prefix)

    def test_default_keeps_private_clone(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);run=root/'run';run.mkdir();template=root/'template';template.mkdir()
            with patch('drawing_fixture_prefix.subprocess.run') as call:
                prefix,lock=prepare(root,run,template);self.assertEqual(prefix,run/'wineprefix');self.assertIsNone(lock)
                self.assertEqual(call.call_args.args[0],['cp','-a','--reflink=auto',str(template),str(prefix)])


if __name__=='__main__':unittest.main()
