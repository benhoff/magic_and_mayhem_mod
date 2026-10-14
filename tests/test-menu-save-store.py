#!/usr/bin/env python3
"""Persistent native-menu save ownership, concurrent sessions and unsafe paths."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('menu_save_store',ROOT/'tools/menu_save_store.py');store=importlib.util.module_from_spec(s);s.loader.exec_module(store)
class SaveStoreTest(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);self.game=self.root/'game';(self.game/'Save').mkdir(parents=True);self.owner=self.root/'owned'
    def tearDown(self):self.tmp.cleanup()
    def write(self,name,data=b'generated original-format placeholder'):(self.game/'Save'/name).write_bytes(data)
    def test_seed_write_delete_and_restart(self):
        self.write('first.sav',b'initial');state=store.stage(self.game,self.owner);self.assertTrue((self.owner/'.retain').exists());self.write('first.sav',b'changed');self.write('second.sav',b'new');self.assertEqual(store.commit(state)['changed'],['first.sav','second.sav'])
        state=store.stage(self.game,self.owner);(self.game/'Save/first.sav').unlink();store.commit(state)
        self.write('first.sav',b'legacy source still has deleted save');again=store.stage(self.game,self.owner);self.assertNotIn('first.sav',again['baseline']);self.assertFalse((self.game/'Save/first.sav').exists());self.assertEqual((self.game/'Save/second.sav').read_bytes(),b'new')
    def test_cancel_has_no_changes(self):
        self.write('first.sav',b'initial');state=store.stage(self.game,self.owner);self.assertEqual(store.commit(state)['changed'],[]);self.assertEqual((self.owner/'first.sav').read_bytes(),b'initial')
    def test_conflict_is_detected_before_any_merge(self):
        self.write('first.sav',b'initial');state=store.stage(self.game,self.owner);self.write('first.sav',b'session');self.write('second.sav',b'session new');(self.owner/'first.sav').write_bytes(b'concurrent')
        with self.assertRaisesRegex(ValueError,'Concurrent'):store.commit(state)
        self.assertEqual((self.owner/'first.sav').read_bytes(),b'concurrent');self.assertFalse((self.owner/'second.sav').exists());self.assertEqual((self.game/'Save/first.sav').read_bytes(),b'session')
    def test_symlinks_aliases_zero_and_case_conflicts(self):
        self.write('first.sav',b'initial');(self.game/'Save/link.sav').symlink_to(self.game/'Save/first.sav')
        with self.assertRaisesRegex(ValueError,'regular'):store.stage(self.game,self.owner)
        (self.game/'Save/link.sav').unlink()
        with self.assertRaisesRegex(ValueError,'alias'):store.stage(self.game,self.game/'Save')
        self.write('first.sav',b'')
        with self.assertRaisesRegex(ValueError,'size'):store.stage(self.game,self.owner)
        self.write('first.sav',b'initial');self.write('FIRST.SAV',b'other')
        with self.assertRaisesRegex(ValueError,'Case'):store.stage(self.game,self.owner)
if __name__=='__main__':unittest.main()
