#!/usr/bin/env python3
"""Exact payload recovery, corruption rejection, and Git pointer workflow checks."""
import io
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import (CONFIG, COMMIT_MAP, CHUNK, Store, artifact, clean,
    decode_bytes, digest_file, fetch_archive, freeze, hydrate, identity,
    install_filter, mapped_commit, pack_git, pointer, safe_path, verify_push)


class EvidenceTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.store = Store(self.root/'objects.sqlite3')
        self.addCleanup(self.store.close)
        self.environment = patch.dict(os.environ, {'MM_EVIDENCE_DB': str(self.store.path),
                'MM_EVIDENCE_CACHE': str(self.root/'cache')})
        self.environment.start()
        self.addCleanup(self.environment.stop)
        (self.root/CONFIG).write_text(json.dumps({'schema': 1, 'archives': []}))

    def put(self, data):
        with self.store.db:
            return self.store.put(io.BytesIO(data), len(data))

    def report(self, data):
        sha = self.put(data)
        path = self.root/'research/runtime/result.json'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(pointer(sha, len(data)))
        return path

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE)

    def test_exact_binary_and_empty_multichunk_roundtrip(self):
        for data in (b'', b'\0\xffnegative evidence\n', b'z'*(CHUNK*2+17)):
            sha = self.put(data)
            self.assertEqual(decode_bytes(self.root, pointer(sha, len(data))), data)
        self.assertEqual(self.store.verify(), 3)

    def test_duplicate_bytes_are_deduplicated(self):
        self.put(b'same'); self.put(b'same')
        self.assertEqual(self.store.db.execute('SELECT COUNT(*) FROM objects').fetchone()[0], 1)

    def test_hydration_preserves_original_bytes_and_unrelated_edits(self):
        path = self.report(b'{"success":false,"sources":{"a":"old"}}\n')
        self.assertEqual(hydrate(self.root), 1)
        self.assertEqual(path.read_bytes(), b'{"success":false,"sources":{"a":"old"}}\n')
        path.write_bytes(b'changed evidence')
        self.assertEqual(hydrate(self.root), 0)
        self.assertEqual(path.read_bytes(), b'changed evidence')

    def test_corrupt_payload_never_replaces_pointer(self):
        path = self.report(b'original')
        previous = path.read_bytes()
        self.store.db.execute('DROP TRIGGER chunks_no_update')
        self.store.db.execute("UPDATE chunks SET payload=x'00'")
        self.store.db.commit()
        with self.assertRaises(Exception):
            hydrate(self.root)
        self.assertEqual(path.read_bytes(), previous)

    def test_deleted_chunk_is_rejected(self):
        path = self.report(b'original')
        self.store.db.execute('DROP TRIGGER chunks_no_delete')
        self.store.db.execute('DELETE FROM chunks'); self.store.db.commit()
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            hydrate(self.root)

    def test_wrong_size_and_missing_object_are_rejected(self):
        sha = self.put(b'payload')
        for p in (pointer(sha, 2), pointer('a'*64, 7)):
            with self.assertRaises(ValueError):
                decode_bytes(self.root, p)

    def test_immutable_records(self):
        self.put(b'payload')
        for sql in ('UPDATE objects SET size=0', 'DELETE FROM objects', 'DELETE FROM chunks'):
            with self.assertRaises(sqlite3.IntegrityError):
                self.store.db.execute(sql)

    def test_pointer_parsing_and_storage_policy(self):
        self.assertIsNone(identity(b'{"ordinary":"json"}'))
        with self.assertRaises(ValueError): identity(pointer('a'*64, 3) + b'junk')
        self.assertTrue(artifact('research/runtime/coverage/review-archives/a.json'))
        self.assertFalse(artifact('research/runtime/coverage/register.json'))
        self.assertFalse(artifact('research/runtime/effects-coverage.json'))
        self.assertFalse(artifact('original/test.bin'))
        self.assertFalse(artifact('research/runtime/results/source.py'))

    def test_unsafe_paths_and_symlink_escape(self):
        for name in ('original/x', '../research/x', '/research/x'):
            with self.assertRaises(ValueError): safe_path(self.root, name)
        (self.root/'research').symlink_to('/tmp', target_is_directory=True)
        with self.assertRaises(ValueError): safe_path(self.root, 'research/x')

    def test_clean_filter_is_idempotent_and_retains_metadata(self):
        for path, data in [('research/runtime/report.json', b'report'),
                           ('research/runtime/coverage/register.json', b'register')]:
            out = io.BytesIO(); clean(self.root, path, io.BytesIO(data), out)
            result = out.getvalue()
            if artifact(path): self.assertIsNotNone(identity(result))
            else: self.assertEqual(result, data)
            again = io.BytesIO(); clean(self.root, path, io.BytesIO(result), again)
            self.assertEqual(result, again.getvalue())

    def test_freeze_and_verified_http_restore(self):
        self.put(b'negative\x00evidence')
        manifest = freeze(self.store.path, self.root/'frozen', 'test-evidence')
        spec = manifest['archives'][0]
        def download(url, **kwargs):
            return (self.root/'frozen'/url.rsplit('/', 1)[1]).open('rb')
        with patch('urllib.request.urlopen', side_effect=download):
            path = fetch_archive(self.root, spec)
        self.assertEqual(digest_file(path), spec['sha256'])
        other = Store(path, readonly=True)
        try: self.assertEqual(other.verify(), 1)
        finally: other.close()

    def test_wrong_download_hash_never_publishes_cache(self):
        self.put(b'payload')
        spec = freeze(self.store.path, self.root/'frozen', 'test-evidence')['archives'][0]
        with patch('urllib.request.urlopen', return_value=io.BytesIO(b'bad')):
            with self.assertRaisesRegex(ValueError, 'download hash mismatch'):
                fetch_archive(self.root, spec)
        self.assertFalse((self.root/'cache'/(spec['sha256']+'.sqlite3')).exists())

    def test_old_commit_translation_preserves_unmapped_values(self):
        (self.root/COMMIT_MAP).write_text(json.dumps({'commits': {'old':'new'}}))
        self.assertEqual(mapped_commit(self.root, 'old'), 'new')
        self.assertEqual(mapped_commit(self.root, 'other'), 'other')

    def test_old_worktree_uses_current_tooling_commit_map(self):
        owner=self.root/'owner';owner.mkdir()
        (owner/COMMIT_MAP).write_text(json.dumps({'commits':{'legacy':'mapped'}}))
        other=self.root/'older-worktree';other.mkdir()
        with patch('coverage_evidence.__file__',str(owner/'tools/coverage_evidence.py')):
            self.assertEqual(mapped_commit(other,'legacy'),'mapped')

    def test_pre_push_rejects_legacy_history(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.git('add', CONFIG); self.git('commit', '-qm', 'old')
        old = self.git('rev-parse', 'HEAD').decode().strip()
        (self.root/COMMIT_MAP).write_text(json.dumps({'commits': {old:'a'*40}}))
        with self.assertRaisesRegex(ValueError, 'pre-migration Git history'):
            verify_push(self.root, f'refs/heads/master {old} refs/heads/master '+ '0'*40)

    def test_pre_push_requires_published_evidence(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.report(b'local evidence')
        self.git('add', CONFIG, 'research'); self.git('commit', '-qm', 'unpublished pointer')
        commit = self.git('rev-parse', 'HEAD').decode().strip()
        with self.assertRaisesRegex(ValueError, 'Publish evidence before pushing'):
            verify_push(self.root, f'refs/heads/master {commit} refs/heads/master '+ '0'*40)

    def test_transport_preserves_identity_normalizes_assets_and_has_a_parent(self):
        import evidence_publish as publication
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.git('add', CONFIG); self.git('commit', '-qm', 'baseline')
        self.git('remote','add','origin','git@github.com:benhoff/magic_and_mayhem_mod.git')
        anchor=self.git('rev-parse','HEAD').decode().strip()
        asset=self.root/'.manifest.json';asset.write_bytes(b'fixture manifest')
        real_git=publication.git;real_run=subprocess.run
        def local_git(root,*args,**kwargs):
            if args[0]=='ls-remote':return anchor+'\t'+args[-1]
            return real_git(root,*args,**kwargs)
        def local_run(command,*args,**kwargs):
            if command[0]=='git' and 'push' in command:return subprocess.CompletedProcess(command,0)
            return real_run(command,*args,**kwargs)
        with patch('evidence_publish.git',side_effect=local_git),patch('subprocess.run',side_effect=local_run):
            result=publication.transport(self.root,self.root/'upload',[asset],'evidence-fixture')
        self.assertEqual(result['assets'][0]['name'],'manifest.json')
        bare=self.root/'upload/transport.git'
        parents=real_git(bare,'rev-list','--parents','-n','1',result['commit']).split()
        self.assertEqual(len(parents),2)
        self.assertEqual(real_git(bare,'show',result['commit']+':assets/manifest.json'),'fixture manifest')

    def test_fresh_clone_restores_exact_reports_and_stays_clean(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.report(b'exact negative historical evidence\n')
        (self.root/'.gitattributes').write_text('research/runtime/** filter=mm-evidence\n')
        self.git('add', CONFIG, '.gitattributes', 'research'); self.git('commit', '-qm', 'pointer baseline')
        clone = self.root/'clone'
        subprocess.run(['git','clone','-q',str(self.root),str(clone)],check=True)
        self.assertIsNotNone(identity((clone/'research/runtime/result.json').read_bytes()))
        install_filter(clone, ROOT/'tools/evidence-store.py')
        self.assertEqual(hydrate(clone),1)
        self.assertEqual((clone/'research/runtime/result.json').read_bytes(),b'exact negative historical evidence\n')
        status=subprocess.check_output(['git','-C',str(clone),'status','--porcelain','--untracked-files=no'])
        self.assertEqual(status,b'')
        archive=subprocess.check_output(['git','-C',str(clone),'archive','--format=tar','HEAD'])
        with tarfile.open(fileobj=io.BytesIO(archive)) as tree:
            stored=tree.extractfile('research/runtime/result.json').read()
        self.assertIsNotNone(identity(stored))

    def test_real_git_clean_filter_and_history_import(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        p = self.root/'research/runtime/result.json'; p.parent.mkdir(parents=True)
        p.write_bytes(b'first historical version'); self.git('add', 'research')
        self.git('commit', '-qm', 'first')
        p.write_bytes(b'current version'); self.git('add', 'research'); self.git('commit', '-qm', 'second')
        report = pack_git(self.root, self.root/'history.sqlite3')
        self.assertEqual(report['git_artifact_blobs'], 2)
        install_filter(self.root, ROOT/'tools/evidence-store.py')
        (self.root/'.gitattributes').write_text('research/runtime/** filter=mm-evidence\n')
        self.git('add', '.gitattributes', 'research'); self.git('commit', '-qm', 'pointers')
        self.assertIsNotNone(identity(self.git('show', 'HEAD:research/runtime/result.json')))
        self.assertEqual(p.read_bytes(), b'current version')
        self.assertEqual(self.git('status', '--porcelain', '--untracked-files=no'), b'')


if __name__ == '__main__':
    unittest.main()
