#!/usr/bin/env python3
"""Real sparse-mirror construction and failure boundary checks."""
from contextlib import closing
import io,json,os
from pathlib import Path
import sqlite3,subprocess,sys,tempfile,unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from evidence_objects import Pair,verify_pair,logical_checkpoint,configured
from evidence_workspace import seal,restore,scan
from evidence_mirror_compaction import prepare,apply,fingerprint
import evidence_mirror_compaction as migration

class Compaction(unittest.TestCase):
    def setUp(self):
        self.ram=tempfile.TemporaryDirectory(prefix='mm-mirror-');self.addCleanup(self.ram.cleanup);self.root=Path(self.ram.name)
        parent=ROOT/'working/tests/mirror-compaction';parent.mkdir(parents=True,exist_ok=True)
        self.disk=tempfile.TemporaryDirectory(prefix='run-',dir=parent);self.addCleanup(self.disk.cleanup)
        self.primary=self.root/'working/primary.sqlite3';self.replica=Path(self.disk.name)/'replica.sqlite3';self.plan=self.root/'working/plan.sqlite3'
        subprocess.run(['git','init','-q',str(self.root)],check=True)
        self.policy=self.root/'policy.json';self.policy.write_text(json.dumps(dict(schema=1,durable_copies=2,primary_minimum_free_gib=0,replica_minimum_free_gib=0)))
        for key,value in [('mm.evidencePrimary',self.primary),('mm.evidenceReplica',self.replica),('mm.evidenceStoragePolicy',self.policy)]:subprocess.run(['git','-C',str(self.root),'config',key,str(value)],check=True)
        self.tree=self.root/'working/capture';self.tree.mkdir(parents=True)
        self.payload=b'raw pixels\0'*20000
        (self.tree/'frames.bin').write_bytes(self.payload);(self.tree/'result.json').write_text('{"negative":true}')
        (self.tree/'tested.exe').write_bytes(b'MZ'+b'tested'*20000)
        self.members=scan(self.tree)[0]
        with configured(self.root)as pair:self.workspace=seal(self.root,self.tree,pair,retire=True)

    def hybrid(self):
        value=json.loads(self.policy.read_text());value.update(schema=2,bulk_copies=1);self.policy.write_text(json.dumps(value))

    def test_lossless_compaction_and_historical_restore(self):
        plan=prepare(self.primary,self.replica,self.plan);self.assertEqual(plan['bulk_objects'],1)
        result=apply(self.plan,plan['plan_sha256'],minimum_free=(0,0));self.assertTrue(result['remaining_primary_full_hash_verified'])
        self.hybrid()
        with configured(self.root)as pair:
            verify_pair(pair);restore(self.root,self.tree,pair)
            self.assertEqual(scan(self.tree)[0],self.members)
            sha=next(r['sha256']for r in self.members if r['path']=='frames.bin')
            self.assertTrue(pair.stores[0].has(sha));self.assertFalse(pair.stores[1].has(sha))

    def test_changed_source_keeps_original_mirror(self):
        plan=prepare(self.primary,self.replica,self.plan)
        with configured(self.root)as pair:pair.bytes(b'new record after planning')
        with self.assertRaisesRegex(ValueError,'changed after planning'):apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        with Pair(self.primary,self.replica)as pair:verify_pair(pair)

    def test_corrupt_surviving_payload_never_retires_replica(self):
        plan=prepare(self.primary,self.replica,self.plan)
        with closing(sqlite3.connect(self.primary))as db,db:
            db.execute('DROP TRIGGER blocks_no_update')
            sha,payload=db.execute('SELECT sha256,payload FROM blocks ORDER BY size DESC LIMIT 1').fetchone()
            db.execute('UPDATE blocks SET payload=? WHERE sha256=?',(bytes(len(payload)),sha))
        with self.assertRaises(Exception):apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        from coverage_evidence import Store
        with closing(Store(self.replica,readonly=True))as store:self.assertGreater(store.verify(),0)

    def test_review_checkpoint_blob_is_not_demoted(self):
        cache=self.root/'working/review.sqlite3'
        with closing(sqlite3.connect(cache))as db,db:
            db.execute('CREATE TABLE unique_inputs(bytes BLOB)');db.execute('INSERT INTO unique_inputs VALUES(?)',(self.payload,))
        logical_checkpoint(self.root,cache)
        result=prepare(self.primary,self.replica,self.plan);self.assertEqual(result['bulk_objects'],0)

    def test_plan_hash_and_stage_capacity_refuse_before_retirement(self):
        plan=prepare(self.primary,self.replica,self.plan)
        with self.assertRaisesRegex(ValueError,'plan changed'):apply(self.plan,'0'*64,minimum_free=(0,0))
        with self.assertRaisesRegex(ValueError,'breach reserve'):apply(self.plan,plan['plan_sha256'],minimum_free=(0,10**30))
        with Pair(self.primary,self.replica)as pair:verify_pair(pair)

    def test_swap_failure_preserves_full_old_mirror(self):
        plan=prepare(self.primary,self.replica,self.plan)
        with patch('evidence_mirror_compaction.os.replace',side_effect=OSError('injected swap failure')):
            with self.assertRaisesRegex(OSError,'swap failure'):apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        from coverage_evidence import Store
        with closing(Store(self.replica,readonly=True))as old:self.assertGreater(old.verify(),0)
        self.assertTrue(self.replica.with_name(self.replica.name+'.hybrid-prepared').is_file())

    def failed_after_scrub(self):
        plan=prepare(self.primary,self.replica,self.plan)
        with patch('evidence_mirror_compaction.copy_blocks',side_effect=RuntimeError('stage stopped')):
            with self.assertRaisesRegex(RuntimeError,'stage stopped'):
                apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        self.replica.with_name(self.replica.name+'.hybrid-prepared').rename(self.replica.with_name('failed-stage.sqlite3'))
        return plan

    def test_completed_scrub_reuse_is_explicit_and_does_not_rehash_primary(self):
        plan=self.failed_after_scrub();copied=[]
        from coverage_evidence import Store
        original=Store.copy
        def observe(store,*args,**kwargs):
            if store.path==self.primary:copied.append(args[0])
            return original(store,*args,**kwargs)
        with patch.object(Store,'copy',observe):
            result=apply(self.plan,plan['plan_sha256'],minimum_free=(0,0),reuse_primary_verification=True,verified_source_commit='HEAD')
        self.assertFalse(result['primary_full_hash_executed']);self.assertEqual(copied,[])
        self.assertEqual(result['primary_verification']['mode'],'retained_full_scrub')
        self.assertTrue(result['replacement_verified'])

    def test_reuse_refuses_physical_touch_even_with_unchanged_catalog(self):
        plan=self.failed_after_scrub();os.utime(self.primary,None)
        with self.assertRaisesRegex(ValueError,'storage changed'):
            apply(self.plan,plan['plan_sha256'],minimum_free=(0,0),reuse_primary_verification=True,verified_source_commit='HEAD')
        from coverage_evidence import Store
        with closing(Store(self.replica,readonly=True))as old:self.assertGreater(old.verify(),0)

    def test_reuse_requires_reviewed_unchanged_decoder_revision(self):
        plan=self.failed_after_scrub()
        with self.assertRaisesRegex(ValueError,'reviewed source commit'):
            apply(self.plan,plan['plan_sha256'],minimum_free=(0,0),reuse_primary_verification=True)
        with patch('evidence_mirror_compaction.decoder_revision',side_effect=ValueError('decoder source changed')):
            with self.assertRaisesRegex(ValueError,'decoder source changed'):
                apply(self.plan,plan['plan_sha256'],minimum_free=(0,0),reuse_primary_verification=True,verified_source_commit='HEAD')

    def test_primary_reserve_loss_during_copy_preserves_old_replica(self):
        plan=prepare(self.primary,self.replica,self.plan);original=migration.copy_blocks;usage=migration.shutil.disk_usage
        def no_primary_space(new,paths,floors):
            def measured(path):
                result=usage(path)
                return result._replace(free=0)if Path(path)==self.primary.parent else result
            with patch('evidence_mirror_compaction.shutil.disk_usage',side_effect=measured):original(new,paths,floors)
        with patch('evidence_mirror_compaction.copy_blocks',side_effect=no_primary_space):
            with self.assertRaisesRegex(ValueError,'reserve crossed'):apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        from coverage_evidence import Store
        with closing(Store(self.replica,readonly=True))as old:self.assertGreater(old.verify(),0)

    def test_blob_copy_stays_below_small_temporary_spill_budget(self):
        # Actual SQLite temp-file allocation is the observable constraint. The
        # old INSERT..SELECT path is a negative control for the same fixture.
        with configured(self.root)as pair:pair.bytes(b'MZ'+os.urandom(64*1024**2))
        plan=prepare(self.primary,self.replica,self.plan);connect=sqlite3.connect;peak=[0]
        class Observed(sqlite3.Connection):
            def set_progress_handler(conn,callback,interval):
                def observe():
                    directory=os.environ.get('SQLITE_TMPDIR','')
                    for fd in Path('/proc/self/fd').iterdir():
                        try:
                            target=os.readlink(fd)
                            if (directory and target.startswith(directory))or'etilqs'in target:peak[0]=max(peak[0],fd.stat().st_size)
                        except OSError:pass
                    return int(peak[0]>8*1024**2)or callback()
                return super().set_progress_handler(observe,min(interval,100))
        def observed(*args,**kwargs):
            if '.hybrid-prepared'in str(args[0]):kwargs['factory']=Observed
            conn=connect(*args,**kwargs)
            if isinstance(conn,Observed):conn.execute('PRAGMA temp_store=FILE')
            return conn
        def old_copy(new,paths,floors):
            new.db.execute('''INSERT INTO blocks SELECT b.*FROM retained_source.blocks b
                WHERE b.sha256 IN(SELECT DISTINCT p.block FROM retained_source.object_parts p
                    JOIN main.objects o ON o.sha256=p.sha256)''')
        with patch('sqlite3.connect',side_effect=observed),patch('evidence_mirror_compaction.copy_blocks',side_effect=old_copy):
            with self.assertRaises(sqlite3.OperationalError):apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        self.assertGreater(peak[0],8*1024**2)
        self.replica.with_name(self.replica.name+'.hybrid-prepared').rename(self.replica.with_name('failed-spill.sqlite3'));peak[0]=0
        previous=os.environ.get('SQLITE_TMPDIR')
        with patch('sqlite3.connect',side_effect=observed):result=apply(self.plan,plan['plan_sha256'],minimum_free=(0,0))
        self.assertTrue(result['replacement_verified']);self.assertLessEqual(peak[0],8*1024**2)
        self.assertEqual(os.environ.get('SQLITE_TMPDIR'),previous)

if __name__=='__main__':unittest.main()
