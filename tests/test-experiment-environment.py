"""Real-filesystem reuse, isolation, retained-state restoration and refusal controls."""
import hashlib
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import experiment_environment as e


class Environment(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);(self.root/'working/tests').mkdir(parents=True)
        self.source=self.root/'working/template';self.source.mkdir();(self.source/'config').mkdir()
        (self.source/'config/settings.cfg').write_bytes(b'clean settings')
        (self.source/'large.dll').write_bytes(bytes(range(256))*4096)
        (self.source/'relative').symlink_to('config')
        self.addCleanup(lambda:e._LEASES.__setitem__(slice(None),[x for x in e._LEASES if x.root!=self.root]))
    def lease(self,name):
        run=self.root/'working/tests'/name;run.mkdir();return e.acquire(self.root,run/'game',self.source,kind='game')
    def test_repeated_runs_reuse_inode_reset_mutations_and_restore_old_saves(self):
        a=self.lease('run-1');inode=(a.target/'large.dll').stat().st_ino;before=e._scan(self.source)[0]
        (a.target/'config/settings.cfg').write_bytes(b'changed setting')
        (a.target/'save.mns').write_bytes(b'unique save evidence');(a.target/'extra.dll').write_bytes(b'patch')
        (a.target/'relative').unlink();(a.target/'relative').symlink_to('large.dll')
        receipt=a.receipt;a.close();self.assertFalse(a.target.exists())
        b=self.lease('run-2');self.assertEqual((b.target/'large.dll').stat().st_ino,inode)
        self.assertEqual({k:v for k,v in e._scan(b.target)[0].items() if k not in ('.retain','.')},{k:v for k,v in before.items() if k!='.'});self.assertFalse((b.target/'save.mns').exists());b.close()
        restored=e.restore(receipt);self.assertEqual((restored/'save.mns').read_bytes(),b'unique save evidence')
        self.assertEqual((restored/'config/settings.cfg').read_bytes(),b'changed setting')
        self.assertEqual(os.readlink(restored/'relative'),'large.dll');self.assertEqual(e._scan(self.source)[0],before)
        with self.assertRaisesRegex(ValueError,'existing'):e.restore(receipt)
    def test_two_slots_third_refuses_without_another_tree(self):
        a=self.lease('run-1');b=self.lease('run-2')
        self.assertNotEqual((a.target/'large.dll').stat().st_ino,(b.target/'large.dll').stat().st_ino)
        with self.assertRaisesRegex(ValueError,'Both environment slots'):self.lease('run-3')
        a.close();b.close();self.assertEqual(len(list(a.family.glob('slot-*'))),2)
    def test_active_lease_is_held_without_reset_or_killing_client(self):
        a=self.lease('run-1');(a.target/'save.mns').write_bytes(b'keep me')
        process=subprocess.Popen([sys.executable,'-c','import sys,time;f=open(sys.argv[1],"rb");print("ready",flush=True);time.sleep(60)',str(a.target/'save.mns')],stdout=subprocess.PIPE,text=True)
        try:
            self.assertEqual(process.stdout.readline().strip(),'ready')
            with self.assertRaisesRegex(ValueError,'host process'):a.close()
            self.assertIsNone(process.poll());self.assertEqual((a.target/'save.mns').read_bytes(),b'keep me')
            with e._db(a.family/'pool.sqlite3') as db:self.assertEqual(db.execute('select phase from slots where slot=?',(a.slot,)).fetchone()[0],'held')
        finally:process.terminate();process.wait(timeout=5);process.stdout.close()
    def test_busy_template_refuses_before_copy(self):
        with patch.object(e,'_busy',return_value=True),patch.object(e,'_copy') as copy:
            with self.assertRaisesRegex(ValueError,'host process'):self.lease('run-1')
            copy.assert_not_called()
    def test_changed_template_and_changed_baseline_refuse(self):
        a=self.lease('run-1');a.close();(self.source/'large.dll').write_bytes(b'drift')
        with self.assertRaisesRegex(ValueError,'Template or immutable baseline'):self.lease('run-2')
    def test_new_pin_keeps_environment_and_configuration_local(self):
        a=self.lease('run-1');(a.target/'.retain').write_text('keep full environment')
        with self.assertRaisesRegex(ValueError,'pinned'):a.close()
        self.assertTrue(a.target.is_dir());self.assertEqual((a.target/'config/settings.cfg').read_bytes(),b'clean settings')
    def test_foreign_mutation_of_idle_tree_is_held(self):
        a=self.lease('run-1');a.close();tree=a.family/('slot-'+str(a.slot))/'tree';(tree/'foreign.raw').write_bytes(b'preserve')
        with self.assertRaisesRegex(ValueError,'Unrecorded changes'):self.lease('run-2')
        self.assertEqual((tree/'foreign.raw').read_bytes(),b'preserve')
    def test_destination_aliases_and_parent_traversal_refuse(self):
        alias=self.root/'working/alias';alias.symlink_to(self.source,target_is_directory=True)
        for target in (alias/'game',self.root/'working/tests/../../outside/game'):
            with self.assertRaises(ValueError):e.acquire(self.root,target,self.source,kind='game')
    def test_corrupted_delta_does_not_change_baseline(self):
        a=self.lease('run-1');(a.target/'save.mns').write_bytes(b'original save');a.close()
        with e._db(a.receipt) as db:db.execute('update delta set payload=?',(b'bad',))
        with self.assertRaises((ValueError,e.zlib.error)):e.restore(a.receipt)
        self.assertFalse((a.family/'pristine/save.mns').exists())
    def test_process_exit_returns_environment_and_crash_recovery_preserves_changes(self):
        program="""import sys,os
from pathlib import Path
sys.path.insert(0,sys.argv[1])
import experiment_environment as e
root=Path(sys.argv[2]);target=Path(sys.argv[3]);source=Path(sys.argv[4])
loan=e.stage(root,target,source,kind='game')
(target/'saved').write_bytes(b'process save')
if sys.argv[5]=='crash':os._exit(7)
"""
        for mode in ('normal','crash'):
            target=self.root/'working/tests'/mode/'game';target.parent.mkdir()
            result=subprocess.run([sys.executable,'-B','-c',program,str(Path(e.__file__).parent),str(self.root),str(target),str(self.source),mode],capture_output=True,text=True)
            self.assertEqual(result.returncode,0 if mode=='normal' else 7,result.stderr)
            receipt=target.parent/'environment-game.sqlite3'
            if mode=='normal':self.assertFalse(target.exists())
            else:
                self.assertEqual((target/'saved').read_bytes(),b'process save')
                original=receipt.read_bytes();receipt=e.recover(receipt)
                self.assertEqual((target.parent/'environment-game.sqlite3').read_bytes(),original)
                self.assertFalse(target.exists())
            e.restore(receipt);self.assertEqual((target/'saved').read_bytes(),b'process save')
    def test_recovery_refuses_live_lease_and_preserves_prior_failures(self):
        a=self.lease('run-1')
        with self.assertRaises(BlockingIOError):e.recover(a.receipt)
        with patch.object(e,'_idle',side_effect=ValueError('temporary busy')):
            with self.assertRaisesRegex(ValueError,'temporary busy'):a.close()
        receipt=e.recover(a.receipt)
        with e._db(a.family/'pool.sqlite3') as db:
            self.assertIn('temporary busy',db.execute('select error from failures').fetchone()[0])
        self.assertTrue(receipt.is_file());self.assertFalse(a.target.exists())
    def test_declared_output_alias_reuses_but_arbitrary_link_is_refused(self):
        other=self.root/'scratch';other.mkdir()
        alias=self.root/'working/tests/ram';alias.symlink_to(other,target_is_directory=True)
        run=alias/'run';run.mkdir()
        a=e.acquire(self.root,run/'game',self.source,kind='game');a.close()
        self.assertTrue((other/'run/environment-game.sqlite3').is_file())
    def test_extended_attributes_and_root_metadata_reset_and_restore(self):
        before=self.source.stat().st_mode&0o777
        a=self.lease('attrs-1');os.setxattr(a.target,'user.fixture',b'root metadata')
        os.setxattr(a.target/'large.dll','user.DOSATTRIB',b'0x21')
        a.target.chmod(0o750);a.close()
        b=self.lease('attrs-2')
        self.assertNotIn('user.fixture',os.listxattr(b.target))
        self.assertNotIn('user.DOSATTRIB',os.listxattr(b.target/'large.dll'))
        self.assertEqual(b.target.stat().st_mode&0o777,before);b.close()
        restored=e.restore(a.receipt)
        self.assertEqual(os.getxattr(restored,'user.fixture'),b'root metadata')
        self.assertEqual(os.getxattr(restored/'large.dll','user.DOSATTRIB'),b'0x21')
        self.assertEqual(restored.stat().st_mode&0o777,0o750)
    def test_other_filesystem_worktree_has_local_bounded_pool(self):
        run=self.root/'working/tests/run';run.mkdir()
        other=self.root/'other-checkout';(other/'tools').mkdir(parents=True)
        (other/'tools/working-retention.py').write_text('# guard on another filesystem')
        original_stat=Path.stat
        def different_device(path,**kwargs):
            value=original_stat(path,**kwargs)
            if path==other:
                fields=list(value);fields[2]=-1
                return os.stat_result(fields)
            return value
        with patch.object(e,'_owner',return_value=other),patch.object(Path,'stat',different_device),patch.object(e,'_mount_root',side_effect=lambda path:other if path.is_relative_to(other) else self.root):
            pool=e._pool(self.root,run/'game')
            self.assertEqual(pool,self.root/'working/.environment-pool')
            self.assertTrue((pool/'.retain').is_file())
            with patch.object(e.subprocess,'run') as copy:
                e._copy(self.source,run/'game',self.root)
                self.assertEqual(copy.call_args.args[0][0],'cp')
    def test_same_device_bind_mount_uses_its_own_pool(self):
        mounted=self.root/'working/tests/mounted';run=mounted/'run';run.mkdir(parents=True)
        with patch.object(e,'_owner',return_value=self.root),patch.object(e,'_mount_root',side_effect=lambda path:mounted if path.is_relative_to(mounted) else self.root):
            pool=e._pool(self.root,run/'game')
        self.assertEqual(pool,mounted/'.environment-pool')
    def test_long_lived_player_prefix_is_preserved_by_default(self):
        target=self.root/'working/native-player/wineprefix';target.parent.mkdir()
        lease=e.stage(self.root,target,self.source)
        self.assertIsNone(lease)
        self.assertEqual((target/'large.dll').read_bytes(),(self.source/'large.dll').read_bytes())
        self.assertFalse((target.parent/'environment-wineprefix.sqlite3').exists())
    def test_invalid_policy_refuses_instead_of_silent_fallback(self):
        with patch.dict(os.environ,{'MNM_REUSE_ENVIRONMENTS':'invalid'}):
            with self.assertRaises(ValueError):e.stage(self.root,self.root/'working/tests/no',self.source,kind='game')

if __name__=='__main__':unittest.main()
