#!/usr/bin/env python3
"""Integration admission, conservative conflict union, history and fresh reads."""
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
from unittest.mock import patch
import zlib

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import MAGIC, Reader, Store
from integration_queue import integration_lock, finish_lock, lock_path, status
from integration_review import reconcile, review_history, merge_json


class Repository(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mm-integration-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.git('init', '-qb', 'master')
        self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.git('config', 'core.hooksPath', '/dev/null')
        (self.root/'.gitignore').write_text('working/\n')
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}\n')
        self.database = self.root/'working/coverage/evidence-pending.sqlite3'
        Store(self.database).close()
        environment = patch.dict(os.environ, MM_EVIDENCE_DB=str(self.database), MM_EVIDENCE_EXTRA_DB='[]',
                                  MM_EVIDENCE_CACHE=str(self.root/'working/evidence-cache'))
        environment.start();self.addCleanup(environment.stop)
        self.register = 'research/runtime/coverage/register.json'
        self.journal = 'research/runtime/coverage/change-reviews.json'
        self.ledger = 'research/runtime/coverage-ledger.md'
        self.write(self.register, dict(schema=1,behaviors=[dict(id='base',scope='unchanged')],evidence=[],code_index=dict(path='base',sha256='a'*64)))
        self.write(self.journal, dict(schema=1,reviews=[]))
        self.write(self.ledger, 'baseline\n')
        self.git('add', '.');self.git('commit', '-qm', 'base')

    def git(self, *args, check=True):
        value = subprocess.run(['git', '-C', str(self.root), *args], capture_output=True)
        if check and value.returncode:
            raise RuntimeError(value.stderr.decode())
        return value.stdout if check else value

    def write(self, name, data):
        path = self.root/name;path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data if isinstance(data, str) else json.dumps(data)+'\n')

    def conflicts(self):
        self.git('switch', '-qc', 'feature')
        for branch in ('feature', 'master'):
            if branch == 'master': self.git('switch', '-q', 'master')
            self.write(self.register, dict(schema=1,behaviors=[dict(id='base',scope='unchanged'),dict(id=branch,scope='retained')],evidence=[],code_index=dict(path=branch,sha256='b'*64)))
            self.write(self.journal, dict(schema=1,reviews=[dict(id=branch,files=[])]))
            self.write(self.ledger, 'baseline\n'+branch+' tail\n')
            self.git('add', '.');self.git('commit', '-qm', branch)
        self.assertEqual(self.git('merge', '--no-ff', '--no-commit', 'feature', check=False).returncode, 1)

    def test_queue_status_is_dry_and_reentrant_child_inherits_lease(self):
        self.assertFalse(status(self.root)['busy'])
        self.assertFalse(lock_path(self.root).exists())
        code = "import sys;sys.path.insert(0,sys.argv[1]);from integration_queue import integration_lock;\nwith integration_lock(sys.argv[2]):print('admitted')"
        with integration_lock(self.root) as lease:
            self.assertTrue(status(self.root)['busy'])
            result = lease.run([sys.executable, '-B', '-c', code, str(ROOT/'tools'), str(self.root)],
                               capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), 'admitted')
        self.assertFalse(status(self.root)['busy'])

    def test_another_integrator_waits_and_then_acquires(self):
        code = "import sys;sys.path.insert(0,sys.argv[1]);from integration_queue import integration_lock;print('waiting',flush=True);\nwith integration_lock(sys.argv[2]):print('acquired',flush=True)"
        with integration_lock(self.root):
            # Deliberately do not pass the lease: this is an independent integrator.
            env = dict(os.environ);env.pop('MM_INTEGRATION_LOCK_FD', None)
            child = subprocess.Popen([sys.executable, '-B', '-c', code, str(ROOT/'tools'), str(self.root)],
                                     env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            self.addCleanup(lambda: child.kill() if child.poll() is None else None)
            self.assertEqual(child.stdout.readline().strip(), 'waiting')
            self.assertIsNone(child.poll())
            self.assertTrue(status(self.root)['busy'])
        out, err = child.communicate(timeout=10)
        self.assertEqual(child.returncode, 0, err)
        self.assertEqual(out.strip(), 'acquired')

    def test_stale_master_refuses_before_running_and_releases(self):
        old = self.git('rev-parse', 'HEAD').decode().strip()
        self.write('new.txt', 'new head');self.git('add', '.');self.git('commit', '-qm', 'new')
        with self.assertRaisesRegex(ValueError, 'Master changed'):
            with integration_lock(self.root, expected_master=old):
                self.fail('stale admission entered')
        self.assertFalse(status(self.root)['busy'])

    def test_bad_inherited_descriptor_is_refused(self):
        with patch.dict(os.environ, MM_INTEGRATION_LOCK_FD='not-a-descriptor'):
            with self.assertRaisesRegex(ValueError, 'not inherited'):
                with integration_lock(self.root):pass

    def test_master_finish_locks_but_feature_finish_is_independent(self):
        with finish_lock(self.root):
            self.assertTrue(status(self.root)['busy'])
        self.git('switch','-qc','feature')
        with finish_lock(self.root) as lease:
            self.assertIsNone(lease)
            self.assertFalse(status(self.root)['busy'])

    def test_metadata_dry_then_union_preserves_both_tails_and_receipt_bodies(self):
        self.conflicts()
        before = {name:(self.root/name).read_bytes() for name in (self.register,self.journal,self.ledger)}
        preview = reconcile(self.root)
        self.assertTrue(preview['dry_run'])
        self.assertEqual(before, {name:(self.root/name).read_bytes() for name in before})
        value = reconcile(self.root, execute=True, reason='Reviewed both disjoint additions and append-only tails',
                          report='research/runtime/integration-test/resolution.json')
        self.assertTrue(value['needs_census_refresh'])
        self.assertFalse(self.git('diff', '--name-only', '--diff-filter=U'))
        register = json.loads((self.root/self.register).read_text())
        self.assertEqual([b['id'] for b in register['behaviors']], ['base','master','feature'])
        self.assertEqual(register['code_index']['path'], 'master')
        self.assertEqual(json.loads((self.root/self.journal).read_text())['reviews'],
                         [dict(id='master',files=[]),dict(id='feature',files=[])])
        self.assertIn('master tail', (self.root/self.ledger).read_text())
        self.assertIn('feature tail', (self.root/self.ledger).read_text())
        self.assertTrue((self.root/'research/runtime/integration-test/resolution.json').read_bytes().startswith(MAGIC))

    def test_same_record_semantic_changes_are_not_synthesized(self):
        b = [dict(id='same',scope='base',confidence='old')]
        o = [dict(id='same',scope='ours',confidence='old')]
        t = [dict(id='same',scope='base',confidence='theirs')]
        with self.assertRaisesRegex(ValueError, 'Both parents'):
            merge_json(b, o, t)
        with self.assertRaises(ValueError):
            merge_json([dict(id='keep')], [], [dict(id='keep'),dict(id='new')])

    def test_invalid_report_path_cannot_modify_conflict_inputs(self):
        self.conflicts()
        before = (self.root/self.register).read_bytes()
        with self.assertRaises(ValueError):
            reconcile(self.root, execute=True, reason='reviewed', report='working/export.json')
        self.assertEqual(before, (self.root/self.register).read_bytes())

    def test_unknown_conflict_is_held(self):
        self.write('code.py','base\n');self.git('add','.');self.git('commit','-qm','code base')
        self.git('switch','-qc','feature');self.write('code.py','feature\n');self.git('add','.');self.git('commit','-qm','feature')
        self.git('switch','-q','master');self.write('code.py','master\n');self.git('add','.');self.git('commit','-qm','master')
        self.git('merge','--no-ff','--no-commit','feature',check=False)
        before=(self.root/'code.py').read_bytes()
        with self.assertRaisesRegex(ValueError,'Nonmetadata'):
            reconcile(self.root, execute=True, reason='reviewed',report='research/runtime/integration-test/no.json')
        self.assertEqual(before,(self.root/'code.py').read_bytes())

    def test_history_preview_then_exact_retrospective_record(self):
        base = self.git('rev-parse','HEAD').decode().strip()
        self.write('code.py','changed\n');self.git('add','.');self.git('commit','-qm','changed')
        tip = self.git('rev-parse','HEAD').decode().strip()
        before=(self.root/self.journal).read_bytes()
        value=review_history(self.root,base,[tip])
        self.assertEqual(value['missing_receipt_count'],1)
        self.assertEqual(before,(self.root/self.journal).read_bytes())
        value=review_history(self.root,base,[tip],record=True,reason='Reviewed exact committed source bytes',
            database=Path('working/coverage/history.sqlite3'),report='research/runtime/integration-test/history.json',
            behaviors=['base'])
        self.assertEqual(len(value['exported_receipts']),1)
        self.assertEqual(review_history(self.root,base,[tip])['missing_receipt_count'],0)

    def test_unreceipted_pointer_requires_payload_before_history_record(self):
        base=self.git('rev-parse','HEAD').decode().strip()
        name='research/runtime/history-test/missing.json'
        self.write(name,(MAGIC+b'a'*64+b' 12\n').decode())
        self.git('add','.');self.git('commit','-qm','unavailable pointer')
        tip=self.git('rev-parse','HEAD').decode().strip()
        self.assertEqual(review_history(self.root,base,[tip])['missing_receipt_count'],1)
        before=(self.root/self.journal).read_bytes()
        with self.assertRaisesRegex(ValueError,'Evidence unavailable'):
            review_history(self.root,base,[tip],record=True,reason='reviewed pointer custody',behaviors=['base'],
                database=Path('working/coverage/history.sqlite3'),report='research/runtime/history-test/report.json')
        self.assertEqual(before,(self.root/self.journal).read_bytes())

    def test_cached_pointer_blob_cannot_escape_report_path_policy(self):
        base=self.git('rev-parse','HEAD').decode().strip()
        payload=(MAGIC+b'a'*64+b' 12\n').decode()
        self.write('research/runtime/history-test/missing.json',payload)
        self.write('z-code.py',payload)
        self.git('add','.');self.git('commit','-qm','same pointer outside policy')
        with self.assertRaisesRegex(ValueError,'outside report storage policy'):
            review_history(self.root,base,[self.git('rev-parse','HEAD').decode().strip()])


class Reads(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix='mm-reader-epoch-');self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}')
        self.db=self.root/'working/coverage/evidence-pending.sqlite3'
        env=patch.dict(os.environ,MM_EVIDENCE_DB=str(self.db),MM_EVIDENCE_EXTRA_DB='[]',MM_EVIDENCE_CACHE=str(self.root/'working/cache'))
        env.start();self.addCleanup(env.stop)
        self.body=b'{"original":true,"value":"retained"}'
        store=Store(self.db)
        with store.db:self.sha=store.put(io.BytesIO(self.body),len(self.body))
        store.close()
        self.path=self.root/'research/runtime/reader.json';self.path.parent.mkdir(parents=True)
        self.path.write_bytes(MAGIC+self.sha.encode()+b' '+str(len(self.body)).encode()+b'\n')

    def test_audit_epoch_shares_verified_digest_and_bytes_but_not_mutable_json(self):
        with Reader(self.root,memoize=True) as reader:
            with patch.object(reader.resolver,'copy',wraps=reader.resolver.copy) as copies:
                self.assertEqual(reader.digest(self.path),self.sha)
                value=reader.json(self.path);value['value']='changed by caller'
                self.assertEqual(reader.json(self.path)['value'],'retained')
                self.assertEqual(reader.digest(self.path),self.sha)
                self.assertEqual(copies.call_count,1)

    def test_default_reader_still_verifies_each_read(self):
        with Reader(self.root) as reader:
            with patch.object(reader.resolver,'copy',wraps=reader.resolver.copy) as copies:
                reader.digest(self.path);reader.json(self.path)
                self.assertEqual(copies.call_count,2)

    def test_new_epoch_detects_corrupt_payload_after_previous_success(self):
        with Reader(self.root,memoize=True) as reader:reader.json(self.path)
        with closing(sqlite3.connect(self.db)) as db, db:
            for name, in db.execute("SELECT name FROM sqlite_master WHERE type='trigger' AND tbl_name='chunks'").fetchall():
                db.execute('DROP TRIGGER "'+name+'"')
            db.execute('UPDATE chunks SET payload=? WHERE sha256=?',(b'bad',self.sha))
        with Reader(self.root,memoize=True) as reader,self.assertRaises((ValueError,zlib.error)):
            reader.digest(self.path)

    def test_file_change_invalidates_even_when_mtime_is_restored(self):
        p=self.root/'plain.txt';p.write_bytes(b'first');old=p.stat()
        with Reader(self.root,memoize=True) as reader:
            before=reader.digest(p)
            p.write_bytes(b'other');os.utime(p,ns=(old.st_atime_ns,old.st_mtime_ns))
            self.assertNotEqual(reader.digest(p),before)
            self.assertEqual(reader.read_bytes(p),b'other')

    def test_body_cache_is_bounded_and_close_discards_observations(self):
        reader=Reader(self.root,memoize=True,cache_bytes=64)
        for n in range(5):
            p=self.root/str(n);p.write_bytes(bytes([n])*40);reader.digest(p)
        self.assertLessEqual(reader._body_bytes,64)
        self.assertEqual(len(reader._bodies),1)
        reader.close()
        self.assertFalse(reader._fingerprints)
        self.assertFalse(reader._bodies)


if __name__=='__main__':unittest.main()
