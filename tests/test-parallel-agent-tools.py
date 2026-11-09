#!/usr/bin/env python3
"""Independent-process controls for shared budgets, guarded refs and queued batches."""
from contextlib import closing
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from verification_budget import workers, directory, DIRECTORY
from integration_queue import integration_lock, require_lease, FD_VARIABLE
from integration_batch import execute, preview
from coverage_evidence import Reader, shared_cache
from coverage_scratch import check_capacity, scratch


class Concurrent(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mm-parallel-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.git('init', '-qb', 'master')
        self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        (self.root/'source').write_text('base')
        self.git('add', '.'); self.git('commit', '-qm', 'base')
        env = patch.dict(os.environ)
        env.start(); self.addCleanup(env.stop)
        for key in (FD_VARIABLE, DIRECTORY):
            os.environ.pop(key, None)

    def git(self, *args, check=True):
        result = subprocess.run(['git', '-C', str(self.root), *args], capture_output=True, text=True)
        if check:
            self.assertEqual(result.returncode, 0, result.stderr)
        return result

    def child(self, code, *args):
        child = subprocess.Popen([sys.executable, '-B', '-c',
            'import sys;sys.path.insert(0,sys.argv[1]);'+code, str(ROOT/'tools'), str(self.root), *args],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, text=True)
        def cleanup():
            if child.poll() is None:
                child.kill()
            child.communicate(timeout=10)
        self.addCleanup(cleanup)
        return child

    def test_six_processes_share_four_slots_and_all_make_progress(self):
        db = self.root/'counter.sqlite3'
        with closing(sqlite3.connect(db)) as connection, connection:
            connection.execute('CREATE TABLE counts(active,peak,finished)')
            connection.execute('INSERT INTO counts VALUES(0,0,0)')
        code = ("from verification_budget import workers;import sqlite3,time;"
                "db=sqlite3.connect(sys.argv[3],timeout=10);print('ready',flush=True);sys.stdin.readline();\n"
                "with workers(sys.argv[2]):\n"
                " with db:db.execute('UPDATE counts SET active=active+1,peak=max(peak,active+1)')\n"
                " time.sleep(0.15)\n"
                " with db:db.execute('UPDATE counts SET active=active-1,finished=finished+1')\n"
                "db.close()")
        children = [self.child(code, str(db)) for _ in range(6)]
        for child in children:
            self.assertEqual(child.stdout.readline().strip(), 'ready')
        for child in children:
            child.stdin.write('go\n'); child.stdin.flush()
        for child in children:
            out, err = child.communicate(timeout=15)
            self.assertEqual(child.returncode, 0, err)
        with closing(sqlite3.connect(db)) as connection:
            active, peak, finished = connection.execute('SELECT * FROM counts').fetchone()
        self.assertEqual((active, finished), (0, 6))
        self.assertLessEqual(peak, 4)
        self.assertGreater(peak, 1)

    def test_killed_owner_releases_slots_without_cleanup_records(self):
        child = self.child("from verification_budget import workers;\nwith workers(sys.argv[2],4) as n:print(n,flush=True);sys.stdin.readline()")
        self.assertEqual(child.stdout.readline().strip(), '4')
        child.kill(); child.communicate(timeout=10)
        with workers(self.root, 4) as count:
            self.assertEqual(count, 4)

    def test_nested_prefetch_uses_only_available_extras(self):
        with workers(self.root, 3) as count:
            self.assertEqual(count, 3)
            child = self.child("from verification_budget import workers;\nwith workers(sys.argv[2]) as n:print(n,flush=True);sys.stdin.readline()")
            self.assertEqual(child.stdout.readline().strip(), '1')
            with workers(self.root, 4) as nested:
                self.assertEqual(nested, 3)
            child.communicate('release\n', timeout=10)
        with workers(self.root, 4) as count:
            self.assertEqual(count, 4)

    def test_linked_worktree_and_materialized_tree_share_budget(self):
        linked = self.root/'linked'
        self.git('worktree', 'add', '-qb', 'other', str(linked))
        self.assertEqual(directory(linked), directory(self.root))
        with shared_cache(self.root):
            self.assertEqual(directory(self.root/'materialized'), directory(self.root))
        self.assertNotIn(DIRECTORY, os.environ)

    def test_master_ref_guard_rejects_unadmitted_and_accepts_inherited_lease(self):
        old = self.git('rev-parse', 'HEAD').stdout.strip()
        self.git('switch', '-qc', 'feature')
        (self.root/'source').write_text('feature')
        self.git('commit', '-qam', 'feature')
        tip = self.git('rev-parse', 'HEAD').stdout.strip()
        self.git('config', 'core.hooksPath', str(ROOT/'.githooks'))
        denied = self.git('update-ref', 'refs/heads/master', tip, old, check=False)
        self.assertNotEqual(denied.returncode, 0)
        self.assertIn('integration admission', denied.stderr)
        self.assertEqual(self.git('rev-parse', 'master').stdout.strip(), old)
        with integration_lock(self.root) as lease:
            result = lease.run(['git', '-C', str(self.root), 'update-ref', 'refs/heads/master', tip, old],
                               capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.git('rev-parse', 'master').stdout.strip(), tip)

    def test_environment_without_locked_descriptor_does_not_authorize(self):
        path = self.root/'.git/mm-integration.lock'
        fd = os.open(path, os.O_CREAT | os.O_RDWR, 0o600)
        self.addCleanup(os.close, fd)
        os.environ[FD_VARIABLE] = str(fd)
        with self.assertRaisesRegex(ValueError, 'integration admission'):
            require_lease(self.root)

    def test_storage_guard_forwards_the_active_lease_to_its_real_child(self):
        (self.root/'working').mkdir()
        policy=json.loads((ROOT/'tools/working-retention.json').read_bytes())
        policy['minimum_free_gib']=0
        target=self.root/'guard-policy.json';target.write_text(json.dumps(policy))
        code="import sys;sys.path.insert(0,sys.argv[1]);from integration_queue import require_lease;require_lease(sys.argv[2]);print('guarded admission inherited')"
        command=[sys.executable,'-B',str(ROOT/'tools/working-retention.py'),'--root',str(self.root),
                 '--policy',str(target),'run','--reserve-gib','0','--',sys.executable,'-B','-c',code,
                 str(ROOT/'tools'),str(self.root)]
        with integration_lock(self.root) as lease:
            result=lease.run(command,capture_output=True,text=True,timeout=10)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('guarded admission inherited',result.stdout)

    def test_scratch_checks_bytes_inodes_and_write_access(self):
        values = dict(f_bavail=100, f_frsize=4096, f_files=100, f_favail=2)
        from types import SimpleNamespace
        with patch('coverage_scratch.os.statvfs', return_value=SimpleNamespace(**values)):
            with self.assertRaisesRegex(ValueError, 'free bytes'):
                check_capacity(self.root, 1000000, 1)
            with self.assertRaisesRegex(ValueError, 'free inodes'):
                check_capacity(self.root, 1, 3)
        with patch('coverage_scratch.tempfile.TemporaryFile', side_effect=PermissionError('quota')):
            with self.assertRaises(PermissionError):
                check_capacity(self.root, 1, 1)

    def test_scratch_falls_back_before_materializing_and_removes_only_its_tree(self):
        with patch('coverage_scratch.check_capacity', side_effect=[ValueError('inode limit'), None]):
            with scratch(self.root, 'bounded-', revision='HEAD') as target:
                self.assertTrue(target.is_relative_to(self.root/'working/coverage/scratch'))
                (target/'disposable').write_text('temporary tracked copy')
            self.assertFalse(target.exists())
        self.assertEqual((self.root/'source').read_text(), 'base')


class Batch(unittest.TestCase):
    def setUp(self):
        spec = importlib.util.spec_from_file_location('parallel_gate_fixture', ROOT/'tests/test-coverage-gate.py')
        module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
        self.fixture = module.GateTests(); self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.root = self.fixture.root
        from coverage_gate import BASELINE, snapshot
        for path in (ROOT/'tools').glob('*.py'):
            self.fixture.write('tools/'+path.name, path.read_text())
        name='git-merge-mm-coverage.sh'
        self.fixture.write('tools/'+name, (ROOT/'tools'/name).read_text())
        (self.root/'tools'/name).chmod(0o755)
        for name in ('pre-commit', 'reference-transaction'):
            self.fixture.write('.githooks/'+name, (ROOT/'.githooks'/name).read_text())
            (self.root/'.githooks'/name).chmod(0o755)
        self.fixture.write('.gitignore', 'working/\n')
        self.fixture.write('research/runtime/coverage-ledger.md', 'Fixture ledger baseline\n')
        self.fixture.json('research/runtime/coverage/change-reviews.json', dict(schema=1, reviews=[]))
        self.fixture.before = self.fixture.current()[0]
        self.fixture.json(BASELINE, self.fixture.before)
        self.fixture.init_git()
        self.git('branch', '-M', 'master')
        self.base = self.git('rev-parse', 'HEAD')
        for branch in ('first', 'second'):
            self.git('switch', '-qc', branch, self.base)
            self.fixture.write('docs/'+branch+'.md', 'Independent authored note '+branch+'\n')
            self.git('add', '.'); self.git('commit', '-qm', branch)
        self.git('switch', '-qc', 'task', self.base)
        self.git('config', 'core.hooksPath', str(self.root/'.githooks'))
        environment = patch.dict(os.environ, MM_EVIDENCE_DB='', MM_EVIDENCE_EXTRA_DB='[]',
                                 MM_EVIDENCE_CACHE=str(self.root/'working/cache'))
        environment.start(); self.addCleanup(environment.stop)
        for key in (FD_VARIABLE, DIRECTORY):
            os.environ.pop(key, None)

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE, text=True).strip()

    def run_batch(self):
        return execute(self.root, ['first', 'second'], branch='integrate/batch',
            report='research/runtime/batch/preparation.json', database=Path('working/review.sqlite3'),
            reason='Reviewed disjoint authored notes and complete retained fixture history',
            validation_reason='Host fixture only; no engine validation or milestone promotion',
            index_reason='Reviewed current fixture sources and existing behavior ownership',
            history_reason='Reviewed exact fixture committed transitions with pending custody only',
            behaviors=getattr(self, 'history_behaviors', ['MV.test']), message='Integrate compatible fixture branches')

    def test_preview_does_not_create_admission_or_change_source(self):
        before = self.git('status', '--porcelain')
        value = preview(self.root, ['first', 'second'])
        self.assertTrue(value['dry_run'])
        self.assertEqual(self.git('status', '--porcelain'), before)
        self.assertFalse((self.root/'.git/mm-integration.lock').exists())

    def test_two_ready_tips_pass_real_finish_backups_hook_and_guarded_promotion(self):
        value = self.run_batch()
        self.assertEqual(value['master'], self.git('rev-parse', 'master'))
        self.assertEqual(self.git('show', '-s', '--format=%P', 'HEAD').split(),
                         [self.base, self.git('rev-parse', 'first'), self.git('rev-parse', 'second')])
        self.assertEqual(self.git('status', '--porcelain'), '')
        with Reader(self.root) as reader:
            self.assertTrue(reader.json('research/runtime/batch/preparation.json')['prepared'])
        self.assertTrue((self.root/'working/backups/catalog.sqlite3').exists())

    def test_checked_batch_fast_forwards_a_separate_clean_master_checkout(self):
        main = self.root/'working/master-checkout'
        self.git('worktree', 'add', str(main), 'master')
        value = self.run_batch()
        actual = subprocess.check_output(['git', '-C', str(main), 'rev-parse', 'HEAD'], text=True).strip()
        self.assertEqual(actual, value['master'])
        self.assertEqual(subprocess.check_output(['git', '-C', str(main), 'status', '--porcelain']), b'')

    def test_ready_branches_union_disjoint_register_journal_and_ledger_additions(self):
        self.git('config', 'core.hooksPath', '/dev/null')
        self.history_behaviors=['MV.test','POLICY.first','POLICY.second']
        for branch in ('first','second'):
            self.git('switch', branch)
            p=self.root/'research/runtime/coverage/register.json'
            register=json.loads(p.read_bytes())
            register['behaviors'].append(dict(id='POLICY.'+branch,subsystem='tooling',kind='native_policy',
                title='Authored fixture '+branch,scope='Deferred authored fixture policy '+branch,confidence='provisional',
                original=[],documents=['docs/'+branch+'.md'],implementation=[],tests=[],evidence=[],
                status=dict(understanding='partial',implementation='none',comparison='none',integration='none',replacement='none'),
                remaining_gaps=['Fixture policy remains deferred']))
            p.write_text(json.dumps(register)+'\n')
            p=self.root/'research/runtime/coverage/change-reviews.json'
            journal=json.loads(p.read_bytes());journal['reviews'].append(dict(id='authored-'+branch,
                reason='Deferred authored fixture metadata',files=[],behaviors=[],pending_gaps=[],
                validation=dict(status='pending',reason='No validation promotion',evidence=[])))
            p.write_text(json.dumps(journal)+'\n')
            p=self.root/'research/runtime/coverage-ledger.md';p.write_text(p.read_text()+branch+' append\n')
            self.git('add','.');self.git('commit','-qm','Authored fixture metadata '+branch)
        self.git('switch','task');self.git('config','core.hooksPath',str(self.root/'.githooks'))
        self.run_batch()
        register=json.loads((self.root/'research/runtime/coverage/register.json').read_bytes())
        self.assertTrue({'POLICY.first','POLICY.second'} <= {b['id'] for b in register['behaviors']})
        ledger=(self.root/'research/runtime/coverage-ledger.md').read_text()
        self.assertIn('first append',ledger);self.assertIn('second append',ledger)
        journal=json.loads((self.root/'research/runtime/coverage/change-reviews.json').read_bytes())
        self.assertTrue({'authored-first','authored-second'} <= {v['id'] for v in journal['reviews']})

    def test_dirty_master_checkout_is_preserved_before_preparation(self):
        main = self.root/'working/master-checkout'
        self.git('worktree', 'add', str(main), 'master')
        (main/'held.txt').write_text('another agent owns this')
        with self.assertRaisesRegex(ValueError, 'Master checkout has other work'):
            self.run_batch()
        self.assertEqual(self.git('branch', '--show-current'), 'task')
        self.assertEqual(self.git('rev-parse', 'master'), self.base)
        self.assertEqual((main/'held.txt').read_text(), 'another agent owns this')

    def test_dirty_task_is_preserved_before_branch_or_merge(self):
        self.fixture.write('docs/held.md', 'someone else owns this')
        with self.assertRaisesRegex(ValueError, 'must be clean'):
            self.run_batch()
        self.assertEqual(self.git('branch', '--show-current'), 'task')
        self.assertEqual(self.git('rev-parse', 'master'), self.base)
        self.assertEqual((self.root/'docs/held.md').read_text(), 'someone else owns this')

    def test_failed_finish_preserves_pending_merge_and_cannot_promote(self):
        from integration_queue import Lease
        original = Lease.run
        def fail_finish(lease, command, **kwargs):
            if any(str(part).endswith('/tools/finish-work.py') for part in command):
                raise subprocess.CalledProcessError(1, command)
            return original(lease, command, **kwargs)
        with patch.object(Lease, 'run', fail_finish):
            with self.assertRaises(subprocess.CalledProcessError):
                self.run_batch()
        self.assertEqual(self.git('rev-parse', 'master'), self.base)
        self.assertTrue(self.git('rev-parse', 'MERGE_HEAD'))
        self.assertTrue((self.root/'docs/first.md').is_file())
        self.assertTrue((self.root/'docs/second.md').is_file())

    def test_conflicting_batch_refuses_without_moving_master_or_resetting_sources(self):
        self.git('config', 'core.hooksPath', '/dev/null')
        self.git('switch', '-qc', 'conflicting', self.base)
        self.fixture.write('docs/first.md', 'Different authored fixture branch\n')
        self.git('add', '.'); self.git('commit', '-qm', 'conflicting fixture input')
        self.git('switch', 'task')
        self.git('config', 'core.hooksPath', str(self.root/'.githooks'))
        with self.assertRaisesRegex(ValueError, 'conflict review'):
            execute(self.root, ['first', 'conflicting'], branch='integrate/conflict',
                report='research/runtime/batch/conflict.json', database=Path('working/review.sqlite3'),
                reason='Review authored fixture conflict refusal', validation_reason='No new validation',
                index_reason='No acceptance promotion', message='Must not promote')
        self.assertEqual(self.git('rev-parse', 'master'), self.base)
        with Reader(self.root) as reader:
            self.assertFalse(reader.json('research/runtime/batch/conflict.json')['prepared'])

    def test_overlapping_behavior_records_are_not_combined_by_batch_strategy(self):
        self.git('config', 'core.hooksPath', '/dev/null')
        for branch in ('first', 'second'):
            self.git('switch', branch)
            p=self.root/'research/runtime/coverage/register.json'
            register=json.loads(p.read_bytes());register['behaviors'][0]['scope']='Competing fixture scope '+branch
            p.write_text(json.dumps(register)+'\n')
            self.git('add','.');self.git('commit','-qm','Competing fixture record '+branch)
        self.git('switch','task');self.git('config','core.hooksPath',str(self.root/'.githooks'))
        with self.assertRaisesRegex(ValueError,'conflict review'):
            self.run_batch()
        self.assertEqual(self.git('rev-parse','master'),self.base)


if __name__ == '__main__':
    unittest.main()
