#!/usr/bin/env python3
"""Hybrid custody, restore, repair and frozen-writer refusal controls."""
from contextlib import closing
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

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from evidence_objects import Pair,configured,_checkpoint_objects,verify_pair
from evidence_workspace import seal,restore,scan
from coverage_evidence import Reader,write_report
from evidence_retention import file_copies


class Hybrid(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix='mm-hybrid-');self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        parent=ROOT/'working/tests/evidence-retention';parent.mkdir(parents=True,exist_ok=True)
        self.disk=tempfile.TemporaryDirectory(prefix='run-',dir=parent);self.addCleanup(self.disk.cleanup)
        self.primary=self.root/'working/primary.sqlite3';self.replica=Path(self.disk.name)/'replica.sqlite3'
        subprocess.run(['git','init','-q',str(self.root)],check=True)
        self.policy=self.root/'policy.json';self.policy.write_text(json.dumps(dict(schema=2,durable_copies=2,bulk_copies=1,primary_minimum_free_gib=0,replica_minimum_free_gib=0)))
        for key,value in [('mm.evidencePrimary',self.primary),('mm.evidenceReplica',self.replica),('mm.evidenceStoragePolicy',self.policy)]:
            subprocess.run(['git','-C',str(self.root),'config',key,str(value)],check=True)
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}')

    def test_bulk_once_essential_twice_and_honest_restore_counts(self):
        tree=self.root/'working/run';tree.mkdir(parents=True)
        (tree/'frames.bin').write_bytes(b'bulk raster\0'*12000)
        (tree/'run.json').write_text('{"result":"failed observation"}')
        (tree/'player.exe').write_bytes(b'MZ'+b'actual tested executable'*6000)
        before=scan(tree)[0]
        with configured(self.root)as pair:
            result=seal(self.root,tree,pair,retire=True)
            self.assertEqual(result['replicas_verified'],1)
            self.assertEqual(result['metadata_replicas_verified'],2)
            self.assertEqual(result['bulk_one_copy_files'],1)
            self.assertIn('Minimum retained payload copies: 1.',(tree/'.retain').read_text())
            bulk=next(r for r in before if r['path']=='frames.bin')
            self.assertTrue(pair.stores[0].has(bulk['sha256']))
            self.assertFalse(pair.stores[1].has(bulk['sha256']))
            restore(self.root,tree,pair)
            self.assertEqual(scan(tree)[0],before)
            verify_pair(pair)

    def test_anonymous_reports_remain_essential(self):
        with configured(self.root)as pair:
            body=b'large critical metadata '*7000;sha=pair.bytes(body)
            self.assertEqual(pair.object_copies(sha),2)
            self.assertEqual(pair.read(sha,len(body),1),body)

    def test_generic_research_binary_reports_remain_essential(self):
        for name,body,copies in [('payload.bin',b'raw'*30000,2),('tested.bin',b'\x7fELF'+b'x'*90000,2)]:
            path='research/runtime/'+name
            sha=write_report(self.root,path,body)
            with Reader(self.root)as reader:self.assertEqual(reader.read_bytes(path),body)
            with configured(self.root)as pair:self.assertEqual(pair.object_copies(sha),copies)

    def test_explicit_second_copy_does_not_silently_fall_back(self):
        with configured(self.root)as pair:
            body=b'bulk'*30000;sha=pair.put(io.BytesIO(body),len(body),copies=1)
            with self.assertRaisesRegex(ValueError,'one retained copy'):pair.read(sha,len(body),1)
            self.assertEqual(pair.read_available(sha,len(body),1),body)

    def test_bulk_can_promote_but_never_implicitly_demote(self):
        with configured(self.root)as pair:
            body=b'unique bytes'*9000;sha=pair.put(io.BytesIO(body),len(body),copies=1)
            pair.bytes(body)
            self.assertEqual(pair.object_copies(sha),2)
            pair.put(io.BytesIO(body),len(body),copies=1)
            self.assertEqual(pair.object_copies(sha),2)
            self.assertEqual(pair.read(sha,len(body),1),body)
            with self.assertRaisesRegex(sqlite3.IntegrityError,'demotion'):
                pair.stores[0].db.execute('UPDATE retention_classes SET copies=1 WHERE sha256=?',(sha,))

    def test_checkpoint_does_not_refill_bulk_mirror(self):
        with configured(self.root)as pair:
            bulk=b'bulk'*30000;sha=pair.put(io.BytesIO(bulk),len(bulk),copies=1)
            pair.bytes(b'essential record')
            pair.root('test','owned',dict(bulk=dict(sha256=sha,size=len(bulk))))
            result=_checkpoint_objects(pair)
            self.assertEqual(result['bulk_one_copy_objects'],1)
            self.assertFalse(pair.stores[1].has(sha))
            verify_pair(pair)

    def test_missing_essential_is_a_failure(self):
        with configured(self.root)as pair:
            pair.bytes(b'critical')
            with closing(sqlite3.connect(self.replica))as db,db:
                db.execute('PRAGMA foreign_keys=OFF')
                db.execute('DROP TRIGGER objects_no_delete');db.execute('DROP TRIGGER object_parts_no_delete')
                db.execute('DELETE FROM object_parts');db.execute('DELETE FROM objects')
            with self.assertRaisesRegex(ValueError,'Essential replica'):
                from evidence_retention import verify_catalogs
                verify_catalogs(pair)

    def test_old_writer_rejected_and_read_protocol_retained(self):
        with configured(self.root)as pair:sha=pair.bytes(b'critical')
        with closing(sqlite3.connect(self.primary))as old:
            self.assertEqual(old.execute('SELECT size FROM objects WHERE sha256=?',(sha,)).fetchone(),(8,))
            with self.assertRaisesRegex(sqlite3.OperationalError,'mm_retention_writer'):
                old.execute('INSERT INTO objects VALUES(?,?,?)',('a'*64,1,None))

    def test_strict_policy_cannot_reopen_sparse_pair(self):
        with configured(self.root):pass
        data=json.loads(self.policy.read_text());data['schema']=1;data.pop('bulk_copies');self.policy.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError,'schema-2'):configured(self.root)

    def test_save_source_configuration_small_diagnostics_are_essential(self):
        for name in ('save1.dat','user-saves/custom.bin','probe.py','run.json','game.cfg','player.dll'):
            self.assertEqual(file_copies(name,200000),2,name)
        self.assertEqual(file_copies('unknown.raw',10),2)
        self.assertEqual(file_copies('full-capture.raw',200000),1)


if __name__=='__main__':unittest.main()
