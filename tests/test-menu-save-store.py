#!/usr/bin/env python3
"""Persistent native-menu save ownership, concurrent sessions and unsafe paths."""
import importlib.util
from contextlib import nullcontext
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
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
    def test_stopped_session_recovers_into_fresh_launch_without_deleting_missing(self):
        session=self.root/'working/session';session.mkdir(parents=True)
        self.game.rename(session/'game');self.game=session/'game'
        self.owner=self.root/'working/saves'
        self.write('previous.sav',b'previous');state=store.stage(self.game,self.owner)
        (session/'manifest.json').write_text(json.dumps({'save_store':state}))
        (self.game/'Save/previous.sav').unlink();self.write('Ben.sav',b'completed save')
        store.recover_session(session,self.root)
        # Retrying after a prior successful exit/recovery must be harmless.
        store.recover_session(session,self.root)
        fresh=self.root/'working/restarted';fresh.mkdir()
        imported=store.stage(fresh,self.owner)
        self.assertEqual(set(imported['baseline']),{'previous.sav','Ben.sav'})
        self.assertEqual((fresh/'Save/Ben.sav').read_bytes(),b'completed save')
    def test_recovery_keeps_concurrent_store_edit_and_session_copy(self):
        self.write('first.sav',b'initial');state=store.stage(self.game,self.owner)
        self.write('first.sav',b'session');(self.owner/'first.sav').write_bytes(b'concurrent')
        with self.assertRaisesRegex(ValueError,'Concurrent'):store.commit(state,preserve_missing=True)
        self.assertEqual((self.owner/'first.sav').read_bytes(),b'concurrent')
        self.assertEqual((self.game/'Save/first.sav').read_bytes(),b'session')
    def test_recovery_rejects_manifest_redirects(self):
        session=self.root/'working/session';session.mkdir(parents=True)
        manifest=session/'manifest.json'
        manifest.write_text(json.dumps({'save_store':{'game':str(self.game),'store':str(self.owner)}}))
        with self.assertRaisesRegex(ValueError,'differs'):store.recover_session(session,self.root)
        manifest.write_text(json.dumps({'save_store':{'game':str(session/'game'),'store':str(self.owner)}}))
        with self.assertRaisesRegex(ValueError,'store must'):store.recover_session(session,self.root)
        with self.assertRaisesRegex(ValueError,'session must'):store.recover_session(self.root,self.root)
    def test_abnormal_launcher_exit_recovers_before_next_launch(self):
        spec=importlib.util.spec_from_file_location('save_test_runner',ROOT/'tools/run-menu-observer.py')
        runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
        session=self.game.parent
        state=store.stage(self.game,self.owner)
        metadata={'origin':'menu_action_bridge','events':str(session/'events.bin'),'save_store':state}
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
            (self.game/name).write_bytes(b'synthetic binary');metadata[key]=hashlib.sha256(b'synthetic binary').hexdigest()
        (session/'manifest.json').write_text(json.dumps(metadata))
        def failed_game(*args,**kwargs):
            self.write('Ben.sav',b'completed before renderer failure')
            return subprocess.CompletedProcess(args,7)
        with patch.object(sys,'argv',['run-menu-observer.py',str(session)]),patch.dict(os.environ,{'MNM_VERIFY_ORIGINALS':'0'}),patch.object(runner.subprocess,'run',side_effect=failed_game),patch('terrain_light_session.supervise',return_value=nullcontext()):
            self.assertEqual(runner.main(),7)
        self.assertEqual((self.owner/'Ben.sav').read_bytes(),b'completed before renderer failure')
        self.assertTrue(json.loads((session/'save-store-result.json').read_text())['merged'])
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
