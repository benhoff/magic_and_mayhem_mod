"""Exercise lossless journal rollover with real SQLite pointer archives."""
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_evidence import MAGIC, read_bytes
from coverage_reviews import REVIEWS, load_journal, split_journal


class Shards(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.env = patch.dict(os.environ, {}, clear=False); self.env.start(); self.addCleanup(self.env.stop)
        os.environ.pop('MM_EVIDENCE_DB', None)
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}\n')
        self.path = self.root/REVIEWS; self.path.parent.mkdir(parents=True)
    def seed(self, reviews, archives=None):
        self.path.write_text(json.dumps(dict(schema=1, archives=archives or [], reviews=reviews)))
    def test_rollover_preserves_existing_shards_and_every_receipt(self):
        reviews = [dict(id=str(i), reason='x'*300) for i in range(12)]
        self.seed(reviews[:6]); split_journal(self.root, 1100)
        previous = json.loads(self.path.read_bytes())['archives']
        pinned = {v['path']: (self.root/v['path']).read_bytes() for v in previous}
        current = json.loads(self.path.read_bytes()); current['reviews'] += reviews[6:]
        self.path.write_text(json.dumps(current)); split_journal(self.root, 1800)
        final = json.loads(self.path.read_bytes())
        self.assertEqual(final['archives'][:len(previous)], previous)
        self.assertEqual(load_journal(self.root)[0]['reviews'], reviews)
        self.assertLessEqual(self.path.stat().st_size, 1800)
        for entry in final['archives']:
            target = self.root/entry['path']
            self.assertTrue(target.read_bytes().startswith(MAGIC))
            self.assertEqual(hashlib.sha256(read_bytes(target, root=self.root)).hexdigest(), entry['sha256'])
        for name, data in pinned.items(): self.assertEqual((self.root/name).read_bytes(), data)
        before = self.path.read_bytes(); split_journal(self.root, 1800)
        self.assertEqual(self.path.read_bytes(), before)
    def test_oversized_receipt_refuses_before_mutation(self):
        self.seed([dict(id='large', reason='x'*5000)])
        before = self.path.read_bytes()
        with self.assertRaisesRegex(ValueError, 'One review'): split_journal(self.root, 1000)
        self.assertEqual(self.path.read_bytes(), before)
        self.assertFalse((self.root/'working/coverage/evidence-pending.sqlite3').exists())
    def test_corrupt_old_shard_refuses_before_mutation(self):
        self.seed([dict(id=str(i), reason='x'*300) for i in range(6)])
        split_journal(self.root, 1100)
        before = self.path.read_bytes(); entry = json.loads(before)['archives'][0]
        (self.root/entry['path']).write_text('{}')
        with self.assertRaisesRegex(ValueError, 'hash mismatch'): split_journal(self.root, 1800)
        self.assertEqual(self.path.read_bytes(), before)


if __name__ == '__main__': unittest.main()
