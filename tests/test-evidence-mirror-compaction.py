#!/usr/bin/env python3
"""Real sparse-mirror construction and failure boundary checks."""
from contextlib import closing
import io,json
from pathlib import Path
import sqlite3,subprocess,sys,tempfile,unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from evidence_objects import Pair,verify_pair,logical_checkpoint,configured
from evidence_workspace import seal,restore,scan
from evidence_mirror_compaction import prepare,apply,fingerprint

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

if __name__=='__main__':unittest.main()
