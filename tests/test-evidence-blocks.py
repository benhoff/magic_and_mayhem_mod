#!/usr/bin/env python3
"""Bounded concurrent encoding, duplicate reuse, durability and queue controls."""
import hashlib
import io
import os
from pathlib import Path
import sqlite3
import sys
import tempfile
import threading
import time
import unittest
from unittest.mock import patch
import zlib
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Store,CHUNK
from evidence_blocks import CompressionPool
from evidence_objects import Pair
from evidence_sweep_schedule import order_runs
from evidence_workspace import scan,seal,MARKER


class Blocks(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix='mm-blocks-');self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
    def pair(self,workers=4):
        return Pair(self.root/'primary.sqlite3',self.root/'replica.sqlite3',require_independent=False,compression_workers=workers)
    def verify(self,pair,data):
        sha=hashlib.sha256(data).hexdigest()
        for i in range(2):self.assertEqual(pair.read(sha,len(data),i),data)

    def test_compress_each_unique_block_once_across_objects_and_replicas(self):
        a=b'a'*CHUNK;b=b'b'*CHUNK;c=b'c'*CHUNK
        original=zlib.compress
        with self.pair() as pair,patch('evidence_blocks.zlib.compress',wraps=original) as compress:
            with pair.batch():
                pair.bytes(a+a+b);pair.bytes(a+c);pair.bytes(a+a+b)
            self.assertEqual(compress.call_count,3)
            self.verify(pair,a+a+b);self.verify(pair,a+c)
            encoded=[s.db.execute('SELECT sha256,size,payload FROM blocks ORDER BY sha256').fetchall() for s in pair.stores]
            self.assertEqual(encoded[0],encoded[1])
            self.assertEqual(len(encoded[0]),3)

    def test_missing_replica_reuses_existing_encoding_without_compression(self):
        body=b'already encoded'*100000
        with self.pair() as pair:
            with pair.stores[0].db:pair.stores[0].put(io.BytesIO(body),len(body))
            with patch('evidence_blocks.zlib.compress',side_effect=AssertionError('encoded twice')):pair.bytes(body)
            self.verify(pair,body)

    def test_bounded_pool_overlaps_encoding_and_keeps_sqlite_on_caller(self):
        data=b''.join(bytes([i])*CHUNK for i in range(12))
        active=0;peak=0;threads=set();lock=threading.Lock();original=zlib.compress;writer=threading.get_ident()
        def compress(raw,level):
            nonlocal active,peak
            self.assertEqual(level,6)
            with lock:active+=1;peak=max(peak,active);threads.add(threading.get_ident())
            try:time.sleep(0.015);return original(raw,level)
            finally:
                with lock:active-=1
        with self.pair(3) as pair,patch('evidence_blocks.zlib.compress',side_effect=compress):
            for store in pair.stores:
                store.db.set_trace_callback(lambda sql:self.assertEqual(threading.get_ident(),writer))
            pair.bytes(data)
            self.assertGreater(peak,1);self.assertLessEqual(peak,3)
            self.assertGreater(len(threads),1)
            self.assertLessEqual(pair.compression.peak_pending,6)
            self.verify(pair,data)

    def test_compression_failure_and_truncation_publish_no_object(self):
        with self.pair() as pair:
            with patch('evidence_blocks.zlib.compress',side_effect=OSError('encode failed')):
                with self.assertRaisesRegex(OSError,'encode failed'):pair.bytes(b'a'*CHUNK)
            with self.assertRaisesRegex(ValueError,'Truncated'):pair.put(io.BytesIO(b'short'),100)
            for s in pair.stores:self.assertEqual(s.db.execute('SELECT count(*) FROM objects').fetchone(),(0,))
            pair.bytes(b'retry');self.verify(pair,b'retry')

    def test_replica_insert_failure_and_retry_keep_identity(self):
        body=b'unique capture'*10000;original=Store.put
        def fail(s,*a,**k):
            if s.path.name=='replica.sqlite3':raise OSError('replica unavailable')
            return original(s,*a,**k)
        with self.pair() as pair:
            with patch.object(Store,'put',fail),self.assertRaisesRegex(OSError,'replica unavailable'):pair.bytes(body)
            self.assertTrue(pair.stores[0].has(hashlib.sha256(body).hexdigest(),len(body)))
            pair.bytes(body);self.verify(pair,body)

    def test_corrupt_existing_block_refuses_replica_repair(self):
        body=b'old capture'*1000
        with self.pair() as pair:
            s=pair.stores[0]
            with s.db:s.put(io.BytesIO(body),len(body))
            s.db.execute('DROP TRIGGER blocks_no_update');s.db.execute('UPDATE blocks SET payload=?',(b'bad',));s.db.commit()
            with self.assertRaises((ValueError,zlib.error)):pair.bytes(body)
            self.assertEqual(pair.stores[1].db.execute('SELECT count(*) FROM objects').fetchone(),(0,))

    def test_reserve_refusal_precedes_new_rows(self):
        with self.pair() as pair:
            pair.stores[0].minimum_free=2**60
            with self.assertRaisesRegex(ValueError,'reserve'):pair.bytes(b'held evidence')
            for s in pair.stores:self.assertEqual(s.db.execute('SELECT count(*) FROM objects').fetchone(),(0,))

    def test_standalone_shared_store_and_old_reader_roundtrip(self):
        s=Store(self.root/'shared.sqlite3',shared=True);self.addCleanup(s.close)
        data=b'z'*(CHUNK*2+17)
        with s.db:sha=s.put(io.BytesIO(data),len(data))
        with patch('evidence_blocks.zlib.compress',side_effect=AssertionError('duplicate compressed')):
            with s.db:s.put(io.BytesIO(data),len(data))
        raw=b''.join(zlib.decompress(v) for v, in s.db.execute('SELECT payload FROM chunks WHERE sha256=? ORDER BY ordinal',(sha,)))
        self.assertEqual(raw,data)
        self.assertEqual(s.db.execute('SELECT count(*) FROM blocks').fetchone(),(2,))

    def test_small_files_share_pool_and_encode_once(self):
        path=self.root/'working/run';path.mkdir(parents=True)
        for i in range(8):(path/str(i)).write_bytes(bytes([i//2])*100000)
        original=zlib.compress;active=0;peak=0;lock=threading.Lock();calls=0
        def compress(data,level):
            nonlocal active,peak,calls
            with lock:active+=1;peak=max(peak,active);calls+=1
            try:time.sleep(.015);return original(data,level)
            finally:
                with lock:active-=1
        before=scan(path)[0]
        with self.pair() as pair,patch('evidence_blocks.zlib.compress',side_effect=compress):
            with pair.batch():rows,_,_=scan(path,pair)
            self.assertEqual(rows,before)
            self.assertEqual(calls,4);self.assertGreater(peak,1)
            self.assertLessEqual(peak,4);self.assertLessEqual(pair.compression.peak_pending,8)

    def test_small_file_failure_keeps_original_workspace(self):
        path=self.root/'working/run';path.mkdir(parents=True);(path/'unique').write_bytes(b'x'*100000)
        with self.pair() as pair,patch('evidence_blocks.zlib.compress',side_effect=OSError('small encode failed')):
            with self.assertRaisesRegex(OSError,'small encode failed'):seal(self.root,path,pair,retire=True)
        self.assertEqual((path/'unique').read_bytes(),b'x'*100000);self.assertFalse((path/MARKER).exists())

    def test_invalid_worker_limits_refuse(self):
        for n in (0,9,True,1.5):
            with self.assertRaisesRegex(ValueError,'workers'):CompressionPool(n)

    def test_large_first_keeps_device_priority_and_eligibility_metadata(self):
        rows=[dict(path='small',device=1,allocated_bytes=1,pin=True),dict(path='external',device=2,allocated_bytes=1000),dict(path='large',device=1,allocated_bytes=100)]
        result=order_runs(rows,1)
        self.assertEqual([r['path'] for r in result],['large','small','external'])
        self.assertEqual(rows[0]['path'],'small');self.assertTrue(result[1]['pin'])
        with self.assertRaisesRegex(ValueError,'allocation'):order_runs([dict(path='bad',device=1,allocated_bytes=-1)],1)


if __name__=='__main__':unittest.main()
