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
    install_filter, mapped_commit, pack_git, pointer, safe_path, verify_push,
    Reader, read_bytes, read_text, read_json, evidence_digest, write_report)
from evidence_sweep import inventory, prepare as prepare_sweep, apply as apply_sweep


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

    def test_direct_reads_do_not_expand_the_report(self):
        data = b'{"success":false,"sources":{"a":"historical"}}\n'
        path = self.report(data)
        reference = path.read_bytes()
        with patch('coverage_evidence.hydrate', side_effect=AssertionError('Unexpected full restore')):
            with Reader(self.root) as reader:
                self.assertEqual(reader.read_bytes(path), data)
                self.assertFalse(reader.json(path)['success'])
                self.assertEqual(reader.digest(path), identity(reference)[0])
            self.assertEqual(read_bytes(path), data)
            self.assertEqual(read_json(path)['sources'], {'a': 'historical'})
            self.assertEqual(evidence_digest(path), identity(reference)[0])
        self.assertEqual(path.read_bytes(), reference)

    def test_nested_reader_scopes_reuse_connections_without_leaking_a_cache(self):
        path = self.report(b'{"success":false}')
        with Reader(self.root):
            with patch('coverage_evidence.Reader', side_effect=AssertionError('Extra connection')):
                self.assertFalse(read_json(path)['success'])
            with Reader(self.root):
                self.assertFalse(read_json(path)['success'])
            self.assertFalse(read_json(path)['success'])
        path.write_bytes(b'{"success":true}')
        self.assertTrue(read_json(path)['success'])

    def test_resident_profile_hashes_original_payload_without_hydration(self):
        from profile_native_resident_helpers import sha
        data = b'{"success":false,"inputs_stable":true,"inputs":{}}\n'
        path = self.report(data)
        reference = path.read_bytes()
        self.assertEqual(sha(path), self.put(data))
        self.assertEqual(path.read_bytes(), reference)
        ordinary = self.root/'ordinary.bin'
        ordinary.write_bytes(data)
        self.assertEqual(sha(ordinary), digest_file(ordinary))

    def test_text_adapter_preserves_path_newline_and_encoding_semantics(self):
        path = self.root/'ordinary.txt'
        path.write_bytes(b'a\r\nb\rc\n\xff')
        self.assertEqual(read_text(path, encoding='latin-1'), path.read_text(encoding='latin-1'))
        self.assertEqual(read_text(path, encoding='latin-1', newline=''),
                         path.read_text(encoding='latin-1', newline=''))
        self.assertEqual(read_bytes(path), path.read_bytes())

    def test_direct_reads_notice_changed_files_and_keep_stale_fingerprints(self):
        path = self.report(b'{"sources":{"a":"old"}}')
        with Reader(self.root) as reader:
            self.assertEqual(reader.json(path)['sources']['a'], 'old')
            path.write_bytes(b'{"sources":{"a":"new"}}')
            self.assertEqual(reader.json(path)['sources']['a'], 'new')

    def test_direct_reader_refuses_corruption_without_changing_reference(self):
        path = self.report(b'{"negative_evidence":true}')
        before = path.read_bytes()
        self.store.db.execute('DROP TRIGGER chunks_no_update')
        self.store.db.execute("UPDATE chunks SET payload=x'00'")
        self.store.db.commit()
        with self.assertRaises(ValueError):
            read_json(path)
        self.assertEqual(path.read_bytes(), before)

    def test_reader_refuses_escaping_paths_and_non_report_pointers(self):
        data = b'{"success":true}'
        sha = self.put(data)
        path = self.root/'settings.json'
        path.write_bytes(pointer(sha, len(data)))
        with Reader(self.root) as reader:
            with self.assertRaisesRegex(ValueError, 'outside report storage policy'):
                reader.json(path)
            with self.assertRaisesRegex(ValueError, 'Escaping evidence read'):
                reader.read_bytes(self.root.parent/'unrelated.json')

    def test_local_pending_objects_are_readable_before_publication(self):
        path = self.root/'research/runtime/pending.json'
        path.parent.mkdir(parents=True)
        output = io.BytesIO()
        clean(self.root, str(path.relative_to(self.root)), io.BytesIO(b'{"success":false}'), output)
        path.write_bytes(output.getvalue())
        with patch.dict(os.environ, {'MM_EVIDENCE_DB': ''}):
            self.assertFalse(read_json(path)['success'])
        self.assertIsNotNone(identity(path.read_bytes()))

    def test_targeted_legacy_export_preserves_other_references(self):
        first = self.report(b'{"first":true}')
        second = first.with_name('second.json')
        sha = self.put(b'{"second":true}')
        second.write_bytes(pointer(sha, len(b'{"second":true}')))
        self.assertEqual(hydrate(self.root, paths=[str(first.relative_to(self.root))]), 1)
        self.assertIsNone(identity(first.read_bytes()))
        self.assertIsNotNone(identity(second.read_bytes()))

    def test_new_research_exports_write_directly_to_sqlite(self):
        path = self.root/'research/runtime/coverage/source-index-new.json'
        data = b'{"schema":1,"files":[]}'
        sha = write_report(self.root, path, data)
        self.assertEqual(identity(path.read_bytes()), (sha, len(data)))
        self.assertEqual(read_bytes(path), data)
        self.assertEqual(write_report(self.root, path, data), sha)
        with self.assertRaisesRegex(ValueError, 'overwrite immutable report'):
            write_report(self.root, path, b'other report')
        self.assertEqual(read_bytes(path), data)

    def test_working_exports_and_existing_expanded_files_keep_their_interface(self):
        path = self.root/'working/source-index.json'
        data = b'{"schema":1}'
        write_report(self.root, path, data)
        self.assertEqual(path.read_bytes(), data)
        existing, original = self.sweep_report()
        write_report(self.root, existing, original)
        self.assertEqual(existing.read_bytes(), original)
        with self.assertRaisesRegex(ValueError, 'Unsafe report output'):
            write_report(self.root, self.root/'original/never.json', data)

    def test_originals_are_protected_through_parent_directory_aliases(self):
        original = self.root/'original'
        original.mkdir()
        payload = original/'input.json'; payload.write_bytes(b'{"immutable":true}')
        alias = self.root/'research/runtime/original-alias'
        alias.parent.mkdir(parents=True); alias.symlink_to(original, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'Escaping evidence path'):
            safe_path(self.root, 'research/runtime/original-alias/input.json')
        with self.assertRaisesRegex(ValueError, 'Unsafe report output'):
            write_report(self.root, alias/'new.json', b'new')
        with self.assertRaisesRegex(ValueError, 'outside research/ and original/'):
            prepare_sweep(self.root, alias/'backup')
        self.assertEqual(payload.read_bytes(), b'{"immutable":true}')
        self.assertFalse((original/'new.json').exists())

    def test_sweep_classifies_and_retains_configuration_fixtures_and_locks(self):
        names = ['protocols/schemas/wire.json', 'tests/fixtures/case.json',
                 'apps/coverage-ui/importance.json', 'research/formats/mmsprite-source-lock.json',
                 'research/runtime/coverage/register.json', 'research/runtime/subsystem-coverage.json']
        for name in names:
            p = self.root/name; p.parent.mkdir(parents=True, exist_ok=True); p.write_text('{}')
        self.report(b'{"report":true}')
        rows = {r['path']:r['role'] for r in inventory(self.root)['files']}
        for name in names:
            self.assertNotEqual(rows[name], 'generated_research')
        self.assertEqual(rows['research/runtime/result.json'], 'generated_research')
        self.assertTrue(artifact('research/formats/ani-inventory.json'))

    def sweep_report(self):
        data = b'{"success":false,"scope":"retained negative evidence","padding":"'+b'x'*12000+b'"}\n'
        path = self.root/'research/runtime/new report.json'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path, data

    def test_sweep_verifies_cold_restore_before_compaction_and_is_resumable(self):
        path, data = self.sweep_report()
        plan = prepare_sweep(self.root, self.root/'cold')
        self.assertEqual(path.read_bytes(), data)
        self.assertEqual((self.root/'cold/verified-restore.payload').read_bytes(), data)
        for receipt in plan['receipts']:
            self.assertEqual(json.loads(Path(receipt).read_text())['state'], 'prepared')
        result = apply_sweep(self.root/'cold/plan.json')
        self.assertEqual(result['state'], 'complete')
        self.assertIsNotNone(identity(path.read_bytes()))
        self.assertEqual(read_bytes(path), data)
        self.assertEqual(apply_sweep(self.root/'cold/plan.json')['state'], 'complete')
        db = sqlite3.connect(plan['backup'])
        try:
            self.assertEqual(db.execute('SELECT path,success FROM json_documents').fetchone(),
                             ('research/runtime/new report.json', 0))
        finally:
            db.close()

    def test_sweep_refuses_concurrent_source_change_before_any_replacement(self):
        path, data = self.sweep_report()
        other = path.with_name('other.json'); other.write_bytes(data)
        prepare_sweep(self.root, self.root/'cold')
        path.write_bytes(b'new unique evidence')
        with self.assertRaisesRegex(ValueError, 'Source changed since'):
            apply_sweep(self.root/'cold/plan.json')
        self.assertEqual(path.read_bytes(), b'new unique evidence')
        self.assertEqual(other.read_bytes(), data)

    def test_sweep_requires_untampered_backup_and_both_catalog_receipts(self):
        path, data = self.sweep_report()
        plan = prepare_sweep(self.root, self.root/'cold')
        receipt = Path(plan['receipts'][1]); value = json.loads(receipt.read_text())
        value['backup_sha256'] = '0'*64; receipt.write_text(json.dumps(value))
        with self.assertRaisesRegex(ValueError, 'Both durable catalog receipts'):
            apply_sweep(self.root/'cold/plan.json')
        self.assertEqual(path.read_bytes(), data)

    def test_sweep_cold_backup_corruption_preserves_expanded_json(self):
        path, data = self.sweep_report()
        plan = prepare_sweep(self.root, self.root/'cold')
        with Path(plan['backup']).open('ab') as output:output.write(b'tampered')
        with self.assertRaisesRegex(ValueError, 'Cold backup changed'):
            apply_sweep(self.root/'cold/plan.json')
        self.assertEqual(path.read_bytes(), data)

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

    def test_archive_verification_reuses_only_an_unchanged_file_identity(self):
        self.put(b'archive verification')
        spec = freeze(self.store.path, self.root/'frozen', 'test-evidence')['archives'][0]
        def download(url, **kwargs):
            return (self.root/'frozen'/url.rsplit('/', 1)[1]).open('rb')
        with patch('urllib.request.urlopen', side_effect=download):
            path = fetch_archive(self.root, spec)
        with patch('coverage_evidence.digest_file', wraps=digest_file) as hashing:
            self.assertEqual(fetch_archive(self.root, spec), path)
            self.assertEqual(fetch_archive(self.root, spec), path)
            self.assertEqual(hashing.call_count, 1)
        before = path.stat()
        data = bytearray(path.read_bytes()); data[-1] ^= 1; path.write_bytes(data)
        os.utime(path, ns=(before.st_atime_ns, before.st_mtime_ns))
        with patch('urllib.request.urlopen', return_value=io.BytesIO(b'bad replacement')):
            with self.assertRaisesRegex(ValueError, 'download hash mismatch'):
                fetch_archive(self.root, spec)

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

    def test_pre_push_allows_unpublished_evidence_without_opening_stores(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.report(b'local evidence')
        self.git('add', CONFIG, 'research'); self.git('commit', '-qm', 'unpublished pointer')
        commit = self.git('rev-parse', 'HEAD').decode().strip()
        with patch('coverage_evidence.Resolver', side_effect=AssertionError('No store needed')), \
             patch('coverage_evidence.fetch_archive', side_effect=AssertionError('No network needed')):
            self.assertEqual(verify_push(self.root,
                f'refs/heads/master {commit} refs/heads/master '+ '0'*40), 1)

    def test_real_pre_push_hook_allows_pointers_without_payloads(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        path = self.root/'research/runtime/unavailable.json'
        path.parent.mkdir(parents=True)
        path.write_bytes(pointer('a'*64, 42))
        self.git('add', CONFIG, 'research'); self.git('commit', '-qm', 'Identity only')
        remote = self.root/'remote.git'
        self.git('init', '--bare', '-q', str(remote))
        self.git('config', 'core.hooksPath', str(ROOT/'.githooks'))
        self.git('push', str(remote), 'HEAD:refs/heads/master')
        self.assertEqual(self.git('--git-dir='+str(remote), 'rev-parse', 'master'),
                         self.git('rev-parse', 'HEAD'))

    def test_pre_push_rejects_raw_and_malformed_evidence(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        path = self.root/'research/runtime/raw.json'; path.parent.mkdir(parents=True)
        for payload, message in ((b'x'*161, 'raw evidence artifact'),
                                 (b'{"raw":true}', 'unconverted evidence artifact'),
                                 (b'magic-and-mayhem-evidence-v1 invalid\n', 'Malformed evidence pointer')):
            with self.subTest(message=message):
                path.write_bytes(payload)
                self.git('add', CONFIG, 'research'); self.git('commit', '-qm', message)
                commit = self.git('rev-parse', 'HEAD').decode().strip()
                with self.assertRaisesRegex(ValueError, message):
                    verify_push(self.root,
                        f'refs/heads/master {commit} refs/heads/master '+ '0'*40)
                # Exclude this version so the next case checks only its new blob.
                self.git('update-ref', 'refs/remotes/origin/master', commit)

    def test_pre_push_checks_evidence_removed_from_head(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        path = self.root/'research/runtime/raw.json'; path.parent.mkdir(parents=True)
        path.write_bytes(b'{"raw":true}')
        self.git('add', CONFIG, 'research'); self.git('commit', '-qm', 'Raw historical report')
        self.git('rm', str(path)); self.git('commit', '-qm', 'Remove report')
        commit = self.git('rev-parse', 'HEAD').decode().strip()
        with self.assertRaisesRegex(ValueError, 'unconverted evidence artifact'):
            verify_push(self.root, f'refs/heads/master {commit} refs/heads/master '+ '0'*40)

    def test_pre_push_allows_branch_deletion_without_evidence(self):
        with patch('coverage_evidence.subprocess.check_output', side_effect=AssertionError('No history needed')):
            self.assertEqual(verify_push(self.root,
                'refs/heads/master '+ '0'*40 +' refs/heads/master '+ 'a'*40), 0)

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
        self.git('add', '.gitattributes')
        # Adopting a filter must re-clean unchanged tracked files even when
        # their cached stat information is still valid.
        self.git('add', '--renormalize', 'research')
        self.git('commit', '-qm', 'pointers')
        self.assertIsNotNone(identity(self.git('show', 'HEAD:research/runtime/result.json')))
        self.assertEqual(p.read_bytes(), b'current version')
        self.assertEqual(self.git('status', '--porcelain', '--untracked-files=no'), b'')


if __name__ == '__main__':
    unittest.main()
