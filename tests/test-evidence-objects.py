#!/usr/bin/env python3
"""Real dual-filesystem object, logical backup, handoff and cache controls."""
import hashlib
import io
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import unittest
import zlib
from contextlib import closing
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Store,Reader,write_report,CHUNK
from evidence_objects import Pair,configured,logical_checkpoint,restore_logical,restore_legacy
from evidence_workspace import seal,restore,scan,MARKER


class Objects(unittest.TestCase):
    def setUp(self):
        self.ram=tempfile.TemporaryDirectory(prefix='mm-objects-');self.addCleanup(self.ram.cleanup)
        parent=ROOT/'working/tests/evidence-objects';parent.mkdir(parents=True,exist_ok=True)
        self.disk=tempfile.TemporaryDirectory(prefix='run-',dir=parent);self.addCleanup(self.disk.cleanup)
        self.root=Path(self.ram.name);self.primary=self.root/'working/primary.sqlite3';self.replica=Path(self.disk.name)/'replica.sqlite3'
        subprocess.run(['git','init','-q',str(self.root)],check=True)
        self.config=self.root/'storage.json';self.config.write_text(json.dumps(dict(schema=1,durable_copies=2,primary_minimum_free_gib=0,replica_minimum_free_gib=0)))
        for k,v in [('mm.evidencePrimary',self.primary),('mm.evidenceReplica',self.replica),('mm.evidenceStoragePolicy',self.config)]:
            subprocess.run(['git','-C',str(self.root),'config',k,str(v)],check=True)
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}')

    def test_shared_blocks_two_replicas_and_legacy_reader(self):
        a=b'a'*CHUNK+b'b'*CHUNK;b=b'a'*CHUNK+b'c'*CHUNK
        with configured(self.root) as pair:
            sha=pair.bytes(a);pair.bytes(b)
            for s in pair.stores:
                self.assertEqual(s.db.execute('SELECT count(*) FROM blocks').fetchone()[0],3)
                self.assertEqual(s.db.execute('SELECT count(*) FROM object_parts').fetchone()[0],4)
                self.assertEqual(s.verify(),2)
        # Independent legacy read protocol: no knowledge of blocks or parts.
        with closing(sqlite3.connect(self.primary)) as db:
            self.assertEqual(db.execute("SELECT value FROM metadata WHERE key='schema'").fetchone(),('1',))
            raw=b''.join(zlib.decompress(p) for _,p in db.execute('SELECT ordinal,payload FROM chunks WHERE sha256=? ORDER BY ordinal',(sha,)))
            self.assertEqual(raw,a)

    def test_same_filesystem_and_aliases_refuse(self):
        with self.assertRaisesRegex(ValueError,'independent'):
            Pair(self.primary,self.primary.with_name('other.sqlite3'))
        link=self.root/'working/alias';link.symlink_to(self.root/'working',target_is_directory=True)
        with self.assertRaisesRegex(ValueError,'Unsafe'):
            Pair(link/'objects.sqlite3',self.replica)

    def test_replica_failure_never_publishes_git_pointer_and_retry_recovers(self):
        name='research/runtime/objects-control.json';body=b'{"real":"new evidence"}'
        original=Store.put
        def fail(store,*args,**kwargs):
            if store.path==self.replica:raise OSError('replica unavailable')
            return original(store,*args,**kwargs)
        with patch.object(Store,'put',fail),self.assertRaises(OSError):write_report(self.root,name,body)
        self.assertFalse((self.root/name).exists())
        write_report(self.root,name,body)
        with Reader(self.root) as r:self.assertEqual(r.read_bytes(name),body)
        with configured(self.root) as pair:self.assertEqual(pair.read(hashlib.sha256(body).hexdigest(),len(body),1),body)

    def test_legacy_import_preserves_all_auxiliary_records_and_versions(self):
        source=self.root/'working/legacy.sqlite3';old=Store(source)
        with old.db:
            sha=old.put(io.BytesIO(b'negative evidence'),17,'a'*40)
            old.db.execute('INSERT INTO versions VALUES(?,?,?)',('a'*40,'research/runtime/old.json',sha))
            old.db.execute('CREATE TABLE json_sweeps(id TEXT PRIMARY KEY,payload TEXT)')
            old.db.execute('INSERT INTO json_sweeps VALUES(?,?)',('held','original failed attempt'))
            old.db.execute('CREATE INDEX sweep_payload ON json_sweeps(payload)')
        old.close()
        with configured(self.root) as pair:
            result=pair.import_legacy(source)
            target=self.root/'working/restored.sqlite3';restore_legacy(pair,result['manifest'],target)
        with closing(sqlite3.connect(target)) as db:
            self.assertEqual(db.execute('SELECT * FROM json_sweeps').fetchall(),[('held','original failed attempt')])
            self.assertEqual(db.execute('SELECT git_blob,path,sha256 FROM versions').fetchall(),[('a'*40,'research/runtime/old.json',sha)])

    def test_logical_checkpoint_preserves_rowids_nuls_blobs_triggers_wal_and_versions(self):
        source=self.root/'working/cache.sqlite3';source.parent.mkdir(exist_ok=True)
        db=sqlite3.connect(source);self.addCleanup(db.close)
        db.execute('PRAGMA journal_mode=WAL');db.execute('PRAGMA user_version=17')
        db.execute('CREATE TABLE t(id INTEGER PRIMARY KEY AUTOINCREMENT,value TEXT,bytes BLOB,number REAL)')
        db.execute('CREATE TABLE other(key TEXT PRIMARY KEY,value TEXT) WITHOUT ROWID')
        db.execute('CREATE TABLE counts(value INTEGER)');db.execute('INSERT INTO counts VALUES(0)')
        db.execute('CREATE TRIGGER changed AFTER INSERT ON t BEGIN UPDATE counts SET value=value+1; END')
        db.execute('INSERT INTO t(value,bytes,number) VALUES(?,?,?)',('discard',b'old',1.0))
        db.execute('DELETE FROM t');db.execute('INSERT INTO t(value,bytes,number) VALUES(?,?,?)',('text\0tail',b'\0\xff',float('inf')))
        db.execute('INSERT INTO other VALUES(?,?)',('key','pending draft'));db.commit()
        result=logical_checkpoint(self.root,source)
        self.assertFalse(result['byte_exact_container'])
        with configured(self.root) as pair:
            count=pair.stores[0].db.execute('SELECT count(*) FROM blocks').fetchone()[0]
            target=self.root/'working/restored-cache.sqlite3';restore_logical(pair,result['manifest'],target)
        with closing(sqlite3.connect(target)) as restored:
            self.assertEqual(restored.execute('SELECT rowid,* FROM t').fetchall(),db.execute('SELECT rowid,* FROM t').fetchall())
            self.assertEqual(restored.execute('SELECT * FROM counts').fetchall(),[(2,)])
            self.assertEqual(restored.execute('PRAGMA user_version').fetchone(),(17,))
        again=logical_checkpoint(self.root,source)
        self.assertEqual(again['manifest'],result['manifest'])
        with configured(self.root) as pair:self.assertEqual(pair.stores[0].db.execute('SELECT count(*) FROM blocks').fetchone()[0],count)

    def tree(self):
        p=self.root/'working/run';p.mkdir(parents=True);(p/'nested').mkdir()
        (p/'.retain').write_bytes(b'unresolved failure: retain entire witness\n')
        (p/'nested/.retain').write_bytes(b'preserve unique save')
        (p/'nested/save').write_bytes(b'owned save\0bytes')
        (p/'relative').symlink_to('nested/save');(p/'external').symlink_to('/unavailable/external/input')
        return p

    def test_workspace_real_restore_pin_transfer_and_historical_path(self):
        path=self.tree();before=scan(path)[0]
        with configured(self.root) as pair:
            result=seal(self.root,path,pair,retire=True)
            self.assertTrue(result['real_restore_verified']);self.assertTrue(result['retired'])
            self.assertTrue((path/MARKER).is_file());self.assertFalse((path/'nested/save').exists())
            restore(self.root,path,pair)
            self.assertEqual(scan(path)[0],before)

    def test_live_workspace_is_not_retired(self):
        path=self.tree()
        child=subprocess.Popen([sys.executable,'-c','import sys,time;f=open(sys.argv[1],"rb");print("ready",flush=True);time.sleep(30)',str(path/'nested/save')],stdout=subprocess.PIPE,text=True)
        try:
            self.assertEqual(child.stdout.readline().strip(),'ready')
            with configured(self.root) as pair,self.assertRaisesRegex(ValueError,'active'):seal(self.root,path,pair,retire=True)
            self.assertEqual((path/'nested/save').read_bytes(),b'owned save\0bytes')
        finally:child.terminate();child.wait();child.stdout.close()

    def test_workspace_budget_and_checkout_refuse_with_source_retained(self):
        path=self.tree()
        with configured(self.root) as pair,self.assertRaisesRegex(ValueError,'budget'):seal(self.root,path,pair,retire=True,restore_budget=0)
        self.assertTrue((path/'nested/save').is_file())
        (path/'.git').mkdir()
        with configured(self.root) as pair,self.assertRaisesRegex(ValueError,'Source checkouts'):seal(self.root,path,pair,retire=True)

    def test_review_cache_preserves_pending_drafts_through_eviction_and_lazy_open(self):
        from coverage_bookkeeping import Workspace
        from evidence_cache import evict,marker
        w=Workspace(self.root);w.db.execute('INSERT INTO drafts VALUES(?,?)',('pending','{"do_not_discard":true}'));w.db.commit();path=w.path;w.close()
        result=evict(self.root,path);self.assertTrue(result['logical_restore_verified']);self.assertFalse(path.exists());self.assertTrue(marker(path).exists())
        w=Workspace(self.root)
        try:self.assertEqual(w.db.execute('SELECT context_json FROM drafts WHERE id=?',('pending',)).fetchone(),('{"do_not_discard":true}',))
        finally:w.close()


    def test_corrupt_replica_cannot_retire_original_workspace(self):
        path=self.tree();body=(path/'nested/save').read_bytes()
        with configured(self.root) as pair:
            sha=pair.bytes(body);db=pair.stores[1].db
            block=db.execute('SELECT block FROM object_parts WHERE sha256=?',(sha,)).fetchone()[0]
            db.execute('DROP TRIGGER blocks_no_update');db.execute('UPDATE blocks SET payload=? WHERE sha256=?',(b'corrupt',block));db.commit()
            with self.assertRaises((ValueError,zlib.error)):seal(self.root,path,pair,retire=True)
        self.assertEqual((path/'nested/save').read_bytes(),body)
        self.assertFalse((path/MARKER).exists())

    def test_source_mutation_during_seal_is_held(self):
        path=self.tree();original=Pair.put;changed=[False]
        def racing(pair,stream,size,*args,**kwargs):
            result=original(pair,stream,size,*args,**kwargs)
            if str(getattr(stream,'name','')).endswith('/nested/save') and not changed[0]:
                changed[0]=True;(path/'nested/save').write_bytes(b'new unique state')
            return result
        with configured(self.root) as pair,patch.object(Pair,'put',racing),self.assertRaisesRegex(ValueError,'changed'):
            seal(self.root,path,pair,retire=True)
        self.assertEqual((path/'nested/save').read_bytes(),b'new unique state')

    def test_completed_execution_handoff_preserves_unique_source_phase(self):
        from evidence_workspace import register_completed,finish_completed
        path=self.tree();source=self.root/'tools/unique.py';source.parent.mkdir();source.write_bytes(b'original source')
        name='research/runtime/completed.json'
        body=(json.dumps(dict(success=False,private_wine_stopped=True,sources={'tools/unique.py':hashlib.sha256(source.read_bytes()).hexdigest()}))+'\n').encode()
        sha=write_report(self.root,name,body)
        register_completed(self.root,[path],dict(record=name,sha256=sha))
        source.write_bytes(b'later source')
        results=finish_completed(self.root)
        self.assertEqual(len(results),1);self.assertTrue((path/MARKER).is_file())
        with configured(self.root) as pair:
            row=pair.stores[0].db.execute("SELECT sha256,size FROM retention_roots WHERE kind='execution_sources'").fetchone()
            values=json.loads(pair.read(*row));ref=values['sources']['tools/unique.py']
            self.assertEqual(pair.read(ref['sha256'],ref['size']),b'original source')


    def test_legacy_snapshot_retirement_requires_full_record_match(self):
        from sqlite_snapshot_store import archive,Store as SnapshotStore
        from evidence_legacy_retirement import retire
        source=self.root/'working/old-evidence.sqlite3';source.parent.mkdir(parents=True)
        old=Store(source)
        with old.db:old.put(io.BytesIO(b'original capture'),16)
        old.close()
        cache=self.root/'working/review.sqlite3'
        with closing(sqlite3.connect(cache)) as db:
            db.execute('CREATE TABLE pending(id INTEGER PRIMARY KEY,value TEXT)')
            db.execute('INSERT INTO pending VALUES(17,?)',('unresolved unique draft',));db.commit()
        catalogs=[self.root/'working/snapshots/catalog.sqlite3',Path(self.disk.name)/'snapshots/catalog.sqlite3']
        for i,path in enumerate((source,cache)):
            scratch=self.root/f'scratch-{i}'
            archive(path,*catalogs,scratch,0,0,False)
        with configured(self.root) as pair:imported=pair.import_legacy(source)
        checkpoint=logical_checkpoint(self.root,cache)
        # A later cache version must not authorize removal of the older draft.
        with closing(sqlite3.connect(cache)) as db:
            db.execute('UPDATE pending SET value=?',('later phase',));db.commit()
        changed=logical_checkpoint(self.root,cache)
        with self.assertRaisesRegex(ValueError,'differs'):retire(self.root,catalogs,imported['manifest'],changed['manifest'])
        for path in catalogs:
            store=SnapshotStore(path,readonly=True);store.close()
        result=retire(self.root,catalogs,imported['manifest'],checkpoint['manifest'])
        self.assertTrue(result['physical_page_images_retired'])
        with configured(self.root) as pair:
            restored=self.root/'working/restored-old-draft.sqlite3'
            restore_logical(pair,checkpoint['manifest'],restored)
        with closing(sqlite3.connect(restored)) as db:
            self.assertEqual(db.execute('SELECT * FROM pending').fetchall(),[(17,'unresolved unique draft')])
        with self.assertRaisesRegex(ValueError,'retired'):SnapshotStore(catalogs[0],readonly=True)
        self.assertTrue(retire(self.root,catalogs,imported['manifest'],checkpoint['manifest'])['success'])

    def test_grouped_objects_are_invisible_until_durable_commit_and_rollback(self):
        with configured(self.root) as pair:
            with self.assertRaisesRegex(ValueError,'interrupted'):
                with pair.batch():
                    pair.bytes(b'uncommitted scientific bytes')
                    pair.root('pending','owned',dict(value='retained only after commit'))
                    with closing(sqlite3.connect(self.primary)) as db:
                        self.assertEqual(db.execute('SELECT count(*) FROM objects').fetchone(),(0,))
                    raise ValueError('interrupted transaction')
            for store in pair.stores:
                self.assertEqual(store.db.execute('SELECT count(*) FROM objects').fetchone(),(0,))
            with pair.batch():
                ref=pair.root('completed','owned',dict(value='two durable copies'))
            self.assertEqual(pair.read(ref['sha256'],ref['size']),pair.read(ref['sha256'],ref['size'],1))

    def test_incremental_sync_restores_root_and_version_metadata_without_images(self):
        from evidence_objects import checkpoint_objects,verify_pair
        with configured(self.root) as pair:
            ref=pair.root('owned','historical',dict(value='negative observation'))
            with pair.stores[0].db:
                pair.stores[0].db.execute('INSERT INTO versions VALUES(?,?,?)',('b'*40,'research/runtime/old.json',ref['sha256']))
            with self.assertRaisesRegex(ValueError,'version'):verify_pair(pair)
        # Simulate an interrupted mirror with no published pointer or retirement.
        self.replica.unlink()
        result=checkpoint_objects(self.root)
        self.assertGreater(result['transferred_objects'],0)
        self.assertFalse(result['physical_database_snapshot'])
        with configured(self.root) as pair:
            verify_pair(pair)
            self.assertEqual(pair.stores[1].db.execute('SELECT path FROM versions').fetchone(),('research/runtime/old.json',))
        self.assertEqual(checkpoint_objects(self.root)['transferred_objects'],0)

    def test_large_inflight_import_does_not_block_existing_report_readers(self):
        body=os.urandom(3*1024**2)
        with configured(self.root) as pair:
            stable=pair.bytes(b'previously published report')
            with pair.batch():
                sha=pair.bytes(body)
                # Exceeds SQLite's page cache: rollback-journal spilling used
                # to hold an exclusive lock and fail unrelated worktree reads.
                for store in pair.stores:
                    reader=Store(store.path,readonly=True)
                    try:
                        self.assertTrue(reader.has(stable));self.assertFalse(reader.has(sha))
                        self.assertEqual(reader.db.execute('PRAGMA journal_mode').fetchone(),('wal',))
                    finally:reader.close()
            self.assertEqual(pair.read(sha,len(body),1),body)
        for path in (self.primary,self.replica):
            reader=Store(path,readonly=True)
            try:self.assertEqual(reader.verify(),2)
            finally:reader.close()

    def test_held_prefix_failure_does_not_queue_automatic_workspace_retirement(self):
        from evidence_workspace import register_completed,finish_completed
        path=self.tree();name='research/runtime/held-prefix.json'
        body=(json.dumps(dict(success=False,private_wine_stopped=True,prefix_pool=dict(returned=False),sources={}))+'\n').encode()
        sha=write_report(self.root,name,body)
        with self.assertRaisesRegex(ValueError,'Held prefix'):
            register_completed(self.root,[path],dict(record=name,sha256=sha))
        self.assertEqual(finish_completed(self.root),[])
        self.assertEqual((path/'nested/save').read_bytes(),b'owned save\0bytes')

    def test_nonfinite_reserve_refuses(self):
        data=json.loads(self.config.read_text());data['primary_minimum_free_gib']=float('nan');self.config.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError,'reserve'):configured(self.root)


if __name__=='__main__':unittest.main()
