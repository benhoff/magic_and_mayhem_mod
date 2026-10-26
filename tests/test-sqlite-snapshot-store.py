#!/usr/bin/env python3
"""Exact restore, shared chunk, corruption and deletion boundary checks."""
from contextlib import closing
from pathlib import Path
import hashlib,sqlite3,sys,tempfile,unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from sqlite_snapshot_store import Store,archive,filehash

class Snapshots(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.source=self.root/"source.sqlite3"
        with closing(sqlite3.connect(self.source)) as db,db:
            db.execute("create table evidence(id integer primary key,payload blob)")
            db.executemany("insert into evidence(payload) values(?)",[(b"retained evidence"*8192,)]*8)
        self.sha=filehash(self.source)
        self.local=self.root/"local.sqlite3";self.cold=self.root/"cold.sqlite3"
    def run_archive(self,**kw):
        return archive(self.source,self.local,self.cold,self.root/"scratch",0,0,**kw)
    def test_exact_restore_and_dual_receipts_before_removal(self):
        result=self.run_archive(remove=True,expected_sha=self.sha)
        self.assertTrue(result["exact_restore_verified"]);self.assertFalse(self.source.exists())
        for path in (self.local,self.cold):
            store=Store(path,readonly=True)
            try:
                out=self.root/(path.stem+"-restored.sqlite3")
                store.restore(self.sha,out);self.assertEqual(filehash(out),self.sha)
                self.assertEqual([r[0] for r in store.db.execute("select state from receipts order by id")],["verified","archived"])
            finally:store.close()
    def test_empty_sqlite_snapshot_restores_exactly(self):
        self.source.write_bytes(b"")
        sha=filehash(self.source)
        self.run_archive(remove=True,expected_sha=sha)
        store=Store(self.cold,readonly=True)
        try:
            target=self.root/"empty.sqlite3"
            store.restore(sha,target)
            self.assertEqual(target.read_bytes(),b"")
        finally:store.close()

    def test_related_snapshots_share_chunks(self):
        self.run_archive()
        store=Store(self.local,readonly=True)
        try:before=store.db.execute("select count(*) from chunks").fetchone()[0]
        finally:store.close()
        with closing(sqlite3.connect(self.source)) as db,db:db.execute("insert into evidence values(999,?)",(b"new row",))
        self.run_archive()
        store=Store(self.local,readonly=True)
        try:
            counts=store.db.execute("select count(distinct chunk),count(*) from parts").fetchone()
            self.assertLess(counts[0],counts[1]);self.assertGreaterEqual(counts[0],before)
        finally:store.close()
    def test_wrong_expected_hash_keeps_source(self):
        with self.assertRaisesRegex(ValueError,"does not match"):self.run_archive(remove=True,expected_sha="0"*64)
        self.assertEqual(filehash(self.source),self.sha)
    def test_cold_failure_keeps_source(self):
        with patch("sqlite_snapshot_store.Store.import_stage",side_effect=ValueError("cold failure")):
            with self.assertRaisesRegex(ValueError,"cold failure"):self.run_archive(remove=True)
        self.assertEqual(filehash(self.source),self.sha)
    def test_active_consumer_keeps_source(self):
        with patch("compact_snapshot_activity.busy",return_value=True):
            with self.assertRaisesRegex(ValueError,"active consumer"):self.run_archive(remove=True)
        self.assertTrue(self.source.exists())
    def test_reserve_refusal_keeps_source(self):
        with self.assertRaisesRegex(ValueError,"reserve"):
            archive(self.source,self.local,self.cold,self.root/"scratch",10**20,0,True)
        self.assertTrue(self.source.exists())
    def test_corrupt_chunk_and_manifest_refuse_restore(self):
        self.run_archive()
        with closing(sqlite3.connect(self.cold)) as db,db:
            db.execute("drop trigger chunks_no_update")
            db.execute("update chunks set payload=x'00' where sha256=(select sha256 from chunks limit 1)")
        store=Store(self.cold,readonly=True)
        try:
            with self.assertRaises(Exception):store.restore(self.sha,self.root/"bad.sqlite3")
            with self.assertRaises(ValueError):store.restore(self.sha,self.source)
        finally:store.close()
    def test_missing_part_refuses_restore(self):
        self.run_archive()
        with closing(sqlite3.connect(self.cold)) as db,db:
            db.execute("drop trigger parts_no_delete");db.execute("delete from parts where ordinal=0")
        store=Store(self.cold,readonly=True)
        try:
            with self.assertRaises(ValueError):store.restore(self.sha,self.root/"missing.sqlite3")
        finally:store.close()
if __name__=="__main__":unittest.main()
