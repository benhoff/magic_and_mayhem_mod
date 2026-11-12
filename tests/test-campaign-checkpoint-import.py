import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('checkpoint_import',ROOT/'tools/stage-campaign-checkpoint.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)


class ImportTest(unittest.TestCase):
    def test_lightweight_checkpoint_is_a_byte_copy_proof_only_with_checkpoint_context(self):
        for mode in ('capture','restore'):
            report=dict(kind='public_native_campaign_lightweight_diagnostic',scenario='checkpoint-'+mode,configuration=dict(checkpoint_mode=mode))
            self.assertTrue(module.checkpoint_acquisition(report))
            self.assertFalse(module.checkpoint_acquisition(dict(report,scenario='victory')))
        self.assertFalse(module.checkpoint_acquisition(dict(kind='public_native_campaign_lightweight_diagnostic')))

    def test_import_preserves_existing_saves_and_is_idempotent(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);store=root/'store';game=root/'game';(game/'Save').mkdir(parents=True)
            (game/'Save/user.sav').write_bytes(b'original-user-save')
            from menu_save_store import stage
            stage(game,store)
            result=module.import_exact(b'original-checkpoint','checkpoint-ready',store,root/'temporary')
            self.assertEqual(result['changed'],['checkpoint-ready.sav'])
            self.assertEqual((store/'user.sav').read_bytes(),b'original-user-save')
            self.assertEqual(module.import_exact(b'original-checkpoint','checkpoint-ready',store,root/'temporary')['changed'],[])

    def test_conflict_and_invalid_names_preserve_every_original_byte(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);store=root/'store'
            module.import_exact(b'original-checkpoint','checkpoint-ready',store,root/'temporary')
            before={p.name:p.read_bytes() for p in store.iterdir()}
            for raw,name in ((b'different','checkpoint-ready'),(b'payload','../user'),(b'','checkpoint-empty')):
                with self.assertRaises(ValueError):module.import_exact(raw,name,store,root/'temporary')
                self.assertEqual({p.name:p.read_bytes() for p in store.iterdir()},before)


if __name__=='__main__':unittest.main()
