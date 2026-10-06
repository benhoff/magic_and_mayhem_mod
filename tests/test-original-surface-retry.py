#!/usr/bin/env python3
import copy,gzip,hashlib,importlib.util,json,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('retry',ROOT/'tools/check-original-surface-retry.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r)
class Retry(unittest.TestCase):
    def test_original(self):self.assertTrue(r.check()['success'])
    def rejected(self,change,rows_change=None):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp);m=json.loads(r.DEFAULT.read_text());data=(r.DEFAULT.parent/m['path']).read_bytes()
            if rows_change:
                rows=json.loads(gzip.decompress(data));rows_change(rows);raw=json.dumps(rows).encode();data=gzip.compress(raw,mtime=0);m.update(sha256=hashlib.sha256(data).hexdigest(),raw_sha256=hashlib.sha256(raw).hexdigest())
            (p/m['path']).write_bytes(data);change(m);file=p/'corpus.json';file.write_text(json.dumps(m))
            with self.assertRaises(ValueError):
                rows,_=r.load(file)
                for row in rows:r.require(r.model(row['input'])==row['original'],'Changed trace')
    def test_hash(self):self.rejected(lambda m:m.update(sha256='0'*64))
    def test_build(self):self.rejected(lambda m:m.update(original_sha256='0'*64))
    def test_path(self):self.rejected(lambda m:m.update(path='../escape'))
    def test_bad_restore(self):self.rejected(lambda m:None,lambda rows:next(row for row in rows if any(e[0]==3 for e in row['original']['events']))['original']['events'].reverse())
    def test_consumption(self):self.rejected(lambda m:None,lambda rows:rows[0]['original'].update(draws_consumed=2))
    def test_key_failure_still_reloads(self):
        rows,_=r.load();row=next(row for row in rows if row['input']['entry']==0x58c4a0 and row['input']['label']=='lost-success' and row['input']['key_result'] and row['input']['reload_bits']==3 and row['input']['key_enabled'] and not row['input']['source_restore'] and not row['input']['destination_restore'])
        self.assertEqual([e[0] for e in row['original']['events']],[1,3,4,5,3,4,5,1])
    def test_finite_schedule_refusal(self):
        rows,_=r.load();c=copy.deepcopy(next(row['input'] for row in rows if row['input']['entry']==0x58c4a0));c['draw_results']=[r.LOST]
        with self.assertRaises(ValueError):r.model(c)
if __name__=='__main__':unittest.main()
