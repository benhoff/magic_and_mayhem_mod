#!/usr/bin/env python3
"""Preserve every payload, source binding and unique metadata across consolidation."""
from pathlib import Path
from contextlib import closing
import io,json,sqlite3,sys,tempfile,unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from coverage_evidence import Store
from evidence_consolidation import consolidate,restore,description

class Consolidation(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.source=self.root/"source.sqlite3";self.local=self.root/"local.sqlite3";self.cold=self.root/"cold.sqlite3"
        self.catalogs=[self.root/"receipts-local.sqlite3",self.root/"receipts-cold.sqlite3"]
        store=Store(self.source)
        try:
            with store.db:
                for i,payload in enumerate((b"shared payload"*100000,b"unique report"*100000)):
                    sha=store.put(io.BytesIO(payload),len(payload),str(i)*40)
                    store.db.execute("insert into versions values(?,?,?)",(str(i)*40,"research/runtime/report"+str(i)+".json",sha))
                store.db.execute("insert into metadata values('unique','retained value')")
            self.before=json.loads(json.dumps(description(store)))
        finally:store.close()
    def run_consolidate(self,**kw):
        return consolidate(self.source,self.local,self.cold,self.catalogs,self.root/"scratch",0,0,**kw)
    def test_union_alias_and_real_source_catalog_restore(self):
        local=Store(self.local)
        payload=b"shared payload"*100000
        try:
            with local.db:local.put(io.BytesIO(payload),len(payload),"a"*40)
        finally:local.close()
        result=self.run_consolidate()
        self.assertTrue(result["exact_payload_restore_verified"]);self.assertTrue(self.source.is_symlink())
        restored=self.root/"restored.sqlite3"
        restore(self.catalogs[1],self.source,restored,self.cold)
        store=Store(restored,readonly=True)
        try:
            self.assertEqual(json.loads(json.dumps(description(store))),self.before)
            self.assertEqual(store.verify(),2)
        finally:store.close()
    def test_binding_collision_keeps_source(self):
        local=Store(self.local)
        try:
            with local.db:
                sha=local.put(io.BytesIO(b"different"),9)
                local.db.execute("insert into versions values(?,?,?)",("0"*40,"research/runtime/report0.json",sha))
        finally:local.close()
        with self.assertRaisesRegex(ValueError,"binding conflict"):self.run_consolidate()
        self.assertFalse(self.source.is_symlink());self.assertTrue(self.source.is_file())
    def test_failed_restore_keeps_source(self):
        with patch("evidence_consolidation.restore",side_effect=ValueError("restore failed")):
            with self.assertRaisesRegex(ValueError,"restore failed"):self.run_consolidate()
        self.assertFalse(self.source.is_symlink())
    def test_active_consumer_refuses(self):
        with patch("evidence_consolidation.busy",return_value=True):
            with self.assertRaisesRegex(ValueError,"active consumer"):self.run_consolidate()
        self.assertFalse(self.source.is_symlink())
    def test_unknown_tables_refuse(self):
        with closing(sqlite3.connect(self.source)) as db,db:db.execute("create table unique_input(value blob)")
        with self.assertRaisesRegex(ValueError,"Unknown evidence tables"):self.run_consolidate()
        self.assertFalse(self.source.is_symlink())
    def test_corrupt_payload_refuses_before_source_replacement(self):
        with closing(sqlite3.connect(self.source)) as db,db:
            db.execute("drop trigger chunks_no_update");db.execute("update chunks set payload=x'00'")
        with self.assertRaises(Exception):self.run_consolidate()
        self.assertFalse(self.source.is_symlink())
    def test_reserve_refusal_keeps_source(self):
        with self.assertRaisesRegex(ValueError,"reserve"):
            consolidate(self.source,self.local,self.cold,self.catalogs,self.root/"scratch",0,10**20)
        self.assertFalse(self.source.is_symlink())
if __name__=="__main__":unittest.main()
