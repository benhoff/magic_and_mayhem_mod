#!/usr/bin/env python3
"""Synthetic change accounting, immutable history, rename and real Git-base checks."""
import copy
from contextlib import redirect_stdout
import io
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_gate import BASELINE, REGISTER, REVIEWS, check, changes, digest, draft, snapshot, source_index

spec = importlib.util.spec_from_file_location('gate_cli', ROOT/'tools/check-re-coverage.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)
spec = importlib.util.spec_from_file_location('hook_install', ROOT/'tools/install-coverage-hook.py')
hook_install = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook_install)


class GateTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for path in ('assets/model.cpp', 'tests/model.py', 'research/finding.md'):
            self.write(path, 'fixture\n')
        self.inventory = {'schema': 1, 'build': 'fixture', 'source_sha256': 'a'*64,
            'sections': [{'start': 4096, 'end': 4136, 'executable': True}],
            'functions': [{'entry': 4096, 'name': 'mapped', 'ranges': [[4096, 4106]], 'data_references': [], 'sha256': 'c'*64},
                          {'entry': 4128, 'name': 'unknown', 'ranges': [[4128, 4136]], 'data_references': [], 'sha256': 'd'*64}],
            'unassigned_executable_ranges': [[4106, 4128]], 'imports': [], 'flows': []}
        self.json('research/inventory.json', self.inventory)
        self.record = {'source_sha256': 'a'*64, 'success': True, 'sources': {'assets/model.cpp': self.sha('assets/model.cpp')}}
        self.json('research/record.json', self.record)
        ev = {'id': 'old-original', 'kind': 'isolated_original', 'scope': 'bounded helper', 'build': 'fixture',
              'record': 'research/record.json', 'sha256': self.sha('research/record.json'), 'sources_pointer': '/sources',
              'original_hash_pointer': '/source_sha256', 'assertions': [{'pointer': '/success', 'equals': True}]}
        b = {'id': 'MV.test', 'subsystem': 'movement', 'kind': 'recovered', 'title': 'Selected movement', 'scope': 'One bounded helper',
             'confidence': 'provisional', 'original': [{'build': 'fixture', 'address': 4096, 'scope': 'entry'}],
             'documents': ['research/finding.md'], 'implementation': ['assets/model.cpp'], 'tests': ['tests/model.py'],
             'evidence': ['old-original'], 'status': {'understanding': 'partial', 'implementation': 'partial',
                                                   'comparison': 'recorded', 'integration': 'none', 'replacement': 'none'}}
        self.register = {'schema': 1, 'builds': [{'id': 'fixture', 'source_sha256': 'a'*64, 'inventory': 'research/inventory.json',
                                                'inventory_sha256': self.sha('research/inventory.json')}],
                         'behaviors': [b], 'evidence': [ev], 'scenarios': [], 'dispatch_tables': [],
                         'classifications': [], 'api_bindings': [], 'recovered_ranges': []}
        self.sync()
        self.before, self.audit_before = snapshot(self.root)

    def write(self, path, value):
        target = self.root/path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(value)

    def json(self, path, value):
        self.write(path, json.dumps(value, sort_keys=True))

    def sha(self, path):
        return hashlib.sha256((self.root/path).read_bytes()).hexdigest()

    def sync(self):
        self.json('research/index.json', source_index.index(self.root, self.register))
        self.register['code_index'] = {'path': 'research/index.json', 'sha256': self.sha('research/index.json')}
        self.json(REGISTER, self.register)

    def current(self):
        self.sync()
        return snapshot(self.root)

    def receipt(self, before=None, after=None):
        after = after or self.current()[0]
        value = draft(changes(before or self.before, after))
        value['reviews'][0]['reason'] = 'Reviewed this bounded source and contract change.'
        value['reviews'][0]['validation']['reason'] = 'Original comparison has not been rerun; current validation is pending.'
        return value

    def result(self, reviews=None, before=None):
        after, report = self.current()
        return check(before or self.before, after, reviews or {'schema': 1, 'reviews': []}, report)

    def assert_error(self, result, text):
        self.assertTrue(any(text in e for e in result['errors']), result['errors'])

    def test_unchanged_backlog_passes_and_is_retained(self):
        result = self.result()
        self.assertEqual(result['errors'], [])
        self.assertGreater(result['counts']['existing_gaps'], 0)
        self.assertEqual(result['counts']['changed_files'], 0)

    def test_output_is_deterministic(self):
        self.assertEqual(self.result(), self.result())
        self.assertEqual(snapshot(self.root)[0], snapshot(self.root)[0])

    def test_index_refresh_cannot_hide_changed_code(self):
        self.write('assets/model.cpp', 'different\n')
        result = self.result()
        self.assert_error(result, 'Unreviewed source change: assets/model.cpp')
        self.assert_error(result, 'Unreviewed behavior change: MV.test')
        self.assert_error(result, 'New gap needs explicit pending accounting')

    def test_research_changes_identify_their_behavior(self):
        self.write('research/finding.md', 'Changed contract finding\n')
        result = self.result()
        self.assert_error(result, 'Unreviewed source change: research/finding.md')
        self.assert_error(result, 'Unreviewed behavior change: MV.test')

    def test_behavior_review_cannot_be_reused_for_a_later_source_edit(self):
        self.write('assets/model.cpp', 'first\n')
        old_review = self.receipt()
        baseline, _ = self.current()
        self.write('assets/model.cpp', 'second\n')
        new_review = self.receipt(before=baseline)
        new_review['reviews'][0]['behaviors'] = []
        new_review['reviews'] += old_review['reviews']
        self.assert_error(self.result(new_review, before=baseline), 'Unreviewed behavior change: MV.test')

    def test_change_review_history_cannot_be_dropped(self):
        self.write('assets/model.cpp', 'first\n')
        reviews = self.receipt()
        self.json('research/runtime/coverage/change-reviews.json', reviews)
        baseline, _ = self.current()
        self.assert_error(self.result({'schema': 1, 'reviews': []}, before=baseline), 'Historical change review removed or rewritten')

    def test_pending_hash_bound_review_passes_without_clearing_staleness(self):
        self.write('assets/model.cpp', 'different\n')
        result = self.result(self.receipt())
        self.assertEqual(result['errors'], [])
        self.assertTrue(any(g['kind'] == 'stale_evidence' for g in result['changes']['new_gaps'].values()))
        self.assertEqual(self.register['evidence'][0]['sha256'], self.before['evidence']['old-original']['sha256'])

    def test_empty_draft_reasons_do_not_approve_anything(self):
        self.write('assets/model.cpp', 'different\n')
        after, _ = self.current()
        result = self.result(draft(changes(self.before, after)))
        self.assert_error(result, 'review reason missing')
        self.assert_error(result, 'Unreviewed source change')

    def test_old_receipt_cannot_cover_a_second_edit(self):
        self.write('assets/model.cpp', 'first\n')
        receipt = self.receipt()
        self.write('assets/model.cpp', 'second\n')
        self.assert_error(self.result(receipt), 'Unreviewed source change')

    def test_preexisting_unlinked_file_requires_review_when_changed(self):
        self.write('tools/unlinked.py', 'before\n')
        baseline, _ = self.current()
        self.write('tools/unlinked.py', 'after\n')
        result = self.result(before=baseline)
        self.assert_error(result, 'Unreviewed source change: tools/unlinked.py')
        self.assertEqual(self.result(self.receipt(before=baseline), before=baseline)['errors'], [])

    def test_new_unlinked_file_needs_explicit_gap_accounting(self):
        self.write('tools/new.py', 'new\n')
        receipt = self.receipt()
        receipt['reviews'][0]['pending_gaps'] = []
        self.assert_error(self.result(receipt), 'New gap needs explicit pending accounting')
        self.assertEqual(self.result(self.receipt())['errors'], [])

    def test_source_links_cannot_change_without_review(self):
        self.register['behaviors'][0]['tests'].append('assets/model.cpp')
        self.assert_error(self.result(), 'Unreviewed source change')

    def test_behavior_metadata_changes_need_review_without_code_edit(self):
        self.register['behaviors'][0]['scope'] += ' Narrower clarified scope.'
        self.assert_error(self.result(), 'Unreviewed behavior change')
        self.assertEqual(self.result(self.receipt())['errors'], [])

    def test_removed_behavior_stays_explicitly_accounted(self):
        self.register['behaviors'] = []
        self.assert_error(self.result(), 'Unreviewed behavior change: MV.test')
        self.assertEqual(self.result(self.receipt())['errors'], [])

    def test_rename_can_rebind_same_historical_source_key(self):
        (self.root/'assets/model.cpp').rename(self.root/'assets/renamed.cpp')
        self.register['behaviors'][0]['implementation'] = ['assets/renamed.cpp']
        self.register['evidence'][0]['source_bindings'] = {'assets/model.cpp': 'assets/renamed.cpp'}
        result = self.result(self.receipt())
        self.assertEqual(result['errors'], [])
        self.assertEqual({f['change'] for f in result['changes']['files'] if f['path'].startswith('assets/')}, {'added', 'removed'})

    def test_removing_historical_evidence_is_rejected_even_with_review(self):
        self.register['evidence'] = []
        self.register['behaviors'][0]['evidence'] = []
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        self.assert_error(self.result(self.receipt()), 'Historical evidence removed')

    def test_overwriting_historical_report_is_rejected_even_with_new_hash(self):
        self.record['extra'] = 'overwritten'
        self.json('research/record.json', self.record)
        self.register['evidence'][0]['sha256'] = self.sha('research/record.json')
        self.assert_error(self.result(self.receipt()), 'Historical evidence rewritten')

    def test_source_provenance_cannot_be_narrowed_to_hide_staleness(self):
        self.record['sources']['tests/model.py'] = self.sha('tests/model.py')
        self.json('research/record.json', self.record)
        self.register['evidence'][0]['sha256'] = self.sha('research/record.json')
        baseline, _ = self.current()
        self.register['evidence'][0]['source_bindings'] = {'assets/model.cpp': 'assets/model.cpp'}
        self.assert_error(self.result(self.receipt(before=baseline), before=baseline), 'Historical provenance keys removed')

    def test_pinned_input_keys_remain_in_historical_provenance(self):
        self.record['sources']['working/game.exe'] = 'a'*64
        self.json('research/record.json', self.record)
        self.register['evidence'][0].update(sha256=self.sha('research/record.json'), recorded_inputs=['working/game.exe'])
        after, _ = self.current()
        self.assertIn('working/game.exe', after['evidence_keys']['old-original'])
        self.assertNotIn('working/game.exe', after['evidence_sources']['old-original'])

    def test_shared_file_identifies_every_affected_behavior(self):
        second = copy.deepcopy(self.register['behaviors'][0])
        second['id'] = 'MV.second'
        self.register['behaviors'].append(second)
        baseline, _ = self.current()
        self.write('assets/model.cpp', 'changed\n')
        receipt = self.receipt(before=baseline)
        receipt['reviews'][0]['behaviors'] = [b for b in receipt['reviews'][0]['behaviors'] if b['id'] == 'MV.test']
        self.assert_error(self.result(receipt, before=baseline), 'Unreviewed behavior change: MV.second')

    def test_recorded_validation_rejects_stale_and_static_evidence(self):
        self.write('assets/model.cpp', 'changed\n')
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['old-original'])
        self.assert_error(self.result(receipt), 'validation evidence missing, static or stale')
        self.register['evidence'][0]['kind'] = 'static'
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['old-original'])
        self.assert_error(self.result(receipt), 'validation evidence missing, static or stale')

    def test_fresh_execution_evidence_can_validate_a_change(self):
        self.write('assets/model.cpp', 'changed\n')
        new = {**self.register['evidence'][0], 'id': 'new-original', 'record': 'research/new-record.json'}
        record = {**self.record, 'sources': {'assets/model.cpp': self.sha('assets/model.cpp')}}
        self.json(new['record'], record)
        new['sha256'] = self.sha(new['record'])
        self.register['evidence'].append(new)
        self.register['behaviors'][0]['evidence'].append(new['id'])
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['new-original'])
        self.assertEqual(self.result(receipt)['errors'], [])

    def test_pending_review_cannot_promote_validation_status(self):
        self.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.assert_error(self.result(self.receipt()), 'Status promotion requires fresh recorded validation')

    def test_not_required_cannot_exempt_engine_changes(self):
        self.write('assets/model.cpp', 'changed\n')
        receipt = self.receipt()
        receipt['reviews'][0]['validation']['status'] = 'not_required'
        self.assert_error(self.result(receipt), 'engine/protocol code requires')

    def test_existing_stale_evidence_does_not_block_unrelated_work(self):
        self.write('assets/model.cpp', 'old already stale\n')
        baseline, _ = self.current()
        self.write('tools/new.py', 'support tool\n')
        receipt = self.receipt(before=baseline)
        receipt['reviews'][0]['validation']['status'] = 'not_required'
        self.assertEqual(self.result(receipt, before=baseline)['errors'], [])

    def test_no_wildcard_gap_waiver(self):
        self.write('tools/new.py', 'new\n')
        receipt = self.receipt()
        receipt['reviews'][0]['pending_gaps'] = ['*']
        self.assert_error(self.result(receipt), 'New gap needs explicit pending accounting')

    def test_fresh_native_evidence_cannot_validate_original_comparison(self):
        new = {**self.register['evidence'][0], 'id': 'native-only', 'kind': 'native_integration'}
        self.register['evidence'].append(new)
        self.register['behaviors'][0]['evidence'].append(new['id'])
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['native-only'])
        self.assert_error(self.result(receipt), 'fresh evidence kind does not support')

    def test_original_inventory_cannot_be_overwritten_even_with_review(self):
        self.inventory['functions'][1]['name'] = 'changed-discovery'
        self.json('research/inventory.json', self.inventory)
        self.register['builds'][0]['inventory_sha256'] = self.sha('research/inventory.json')
        self.assert_error(self.result(self.receipt()), 'Pinned original inventory removed or overwritten')

    def test_new_inventory_snapshot_preserves_previous_discovery(self):
        self.json('research/new-inventory.json', self.inventory)
        self.register['builds'][0].update(inventory='research/new-inventory.json', inventory_sha256=self.sha('research/new-inventory.json'))
        self.assertEqual(self.result(self.receipt())['errors'], [])

    def test_inactive_historical_review_does_not_require_rerunning_evidence(self):
        self.register['behaviors'][0]['scope'] += ' Clarified.'
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['old-original'])
        baseline, _ = self.current()
        self.write('assets/model.cpp', 'now changed\n')
        latest = self.receipt(before=baseline)
        latest['reviews'] = receipt['reviews'] + latest['reviews']
        self.assertEqual(self.result(latest, before=baseline)['errors'], [])

    def test_obsolete_recorded_contract_cannot_reactivate_through_unchanged_file(self):
        self.write('research/finding.md', 'First clarified scope\n')
        older = self.receipt()
        older['reviews'][0]['id'] = 'first-recorded-version'
        older['reviews'][0]['validation'].update(status='recorded', evidence=['old-original'])
        self.assertEqual(self.result(older)['errors'], [])
        # The original base stays fixed: the research file still matches the
        # earlier receipt, but its behavior contract and evidence are obsolete.
        self.write('assets/model.cpp', 'Later model version\n')
        only_old = self.result(older)
        self.assert_error(only_old, 'Unreviewed source change: research/finding.md')
        self.assert_error(only_old, 'Unreviewed behavior change: MV.test')
        latest = self.receipt()
        latest['reviews'][0]['id'] = 'current-pending-version'
        latest['reviews'] = older['reviews'] + latest['reviews']
        self.assertEqual(self.result(latest)['errors'], [])
        self.assertTrue(any(g['kind'] == 'stale_evidence' for g in self.result(latest)['changes']['new_gaps'].values()))
        # An applicable current receipt still cannot claim stale execution.
        latest['reviews'][-1]['validation'].update(status='recorded', evidence=['old-original'])
        self.assert_error(self.result(latest), 'validation evidence missing, static or stale')

    def test_controls_are_watched(self):
        self.write('AGENTS.md', 'Changed workflow\n')
        self.assert_error(self.result(), 'Unreviewed source change: AGENTS.md')

    def test_unreconciled_census_fails_before_approval(self):
        self.write('assets/model.cpp', 'changed\n')
        with self.assertRaisesRegex(ValueError, 'Reconcile source'):
            snapshot(self.root)

    def test_unknown_functions_becoming_unmapped_are_a_new_gap(self):
        self.register['behaviors'][0]['original'] = []
        self.register['behaviors'][0]['status']['understanding'] = 'unknown'
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        result = self.result()
        self.assertTrue(any(g['kind'] == 'unclassified_functions' for g in result['changes']['new_gaps'].values()))

    def init_git(self):
        def run(*args):
            return subprocess.run(['git', '-C', str(self.root), *args], check=True, capture_output=True).stdout
        run('init', '-q')
        run('config', 'user.email', 'coverage-fixture@example.invalid')
        run('config', 'user.name', 'Coverage fixture')
        self.json(BASELINE, self.before)
        run('add', '.')
        run('commit', '-qm', 'reviewed baseline')
        return run('rev-parse', 'HEAD').decode().strip()

    def test_real_git_base_cannot_be_reset_by_current_baseline_edit(self):
        revision = self.init_git()
        self.write('assets/model.cpp', 'changed\n')
        after, report = self.current()
        self.json(BASELINE, after)
        git_before, commit = cli.git_snapshot(self.root, revision)
        self.assertEqual(commit, revision)
        self.assertEqual(git_before['sources']['assets/model.cpp']['sha256'], self.before['sources']['assets/model.cpp']['sha256'])
        self.assert_error(check(git_before, after, {'schema': 1, 'reviews': []}, report), 'Unreviewed source change')

    def test_cli_refuses_overwrite_and_bootstrap_after_adoption(self):
        revision = self.init_git()
        self.json('reviews.json', {'schema': 1, 'reviews': []})
        with redirect_stdout(io.StringIO()), patch.object(cli, 'ROOT', self.root), patch.object(sys, 'argv', ['gate', '--freeze-baseline', str(self.root/BASELINE)]):
            self.assertEqual(cli.main(), 1)
        with redirect_stdout(io.StringIO()), patch.object(cli, 'ROOT', self.root), patch.object(sys, 'argv', ['gate', '--base', revision, '--bootstrap', '--reviews', str(self.root/'reviews.json')]):
            self.assertEqual(cli.main(), 1)

    def test_git_base_rejects_unsafe_symlinks_without_following_them(self):
        self.init_git()
        (self.root/'assets/link.cpp').symlink_to('/etc/passwd')
        subprocess.run(['git', '-C', str(self.root), 'add', 'assets/link.cpp'], check=True, capture_output=True)
        subprocess.run(['git', '-C', str(self.root), 'commit', '-qm', 'unsafe symlink fixture'], check=True, capture_output=True)
        with self.assertRaisesRegex(ValueError, 'unsupported symlink'):
            cli.git_snapshot(self.root, 'HEAD')

    def test_staged_tree_ignores_unstaged_changes_and_game_artifacts(self):
        self.write('original/never-read', 'immutable input')
        self.write('working/never-read', 'disposable input')
        self.init_git()
        self.write('assets/model.cpp', 'unstaged fix\n')
        with cli.staged_tree(self.root) as tree:
            self.assertEqual((tree/'assets/model.cpp').read_text(), 'fixture\n')
            self.assertFalse((tree/'original').exists())
            self.assertFalse((tree/'working').exists())
        self.assertEqual((self.root/'assets/model.cpp').read_text(), 'unstaged fix\n')

    def test_staged_tree_rejects_symlink(self):
        self.init_git()
        (self.root/'assets/link.cpp').symlink_to('/etc/passwd')
        cli.git(self.root, 'add', 'assets/link.cpp')
        with self.assertRaisesRegex(ValueError, 'Unsupported staged file mode'):
            with cli.staged_tree(self.root):
                pass

    def test_staged_gate_cannot_use_unstaged_census_fix(self):
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        self.init_git()
        self.write('assets/model.cpp', 'changed\n')
        cli.git(self.root, 'add', 'assets/model.cpp')
        self.sync()  # Unstaged repaired metadata must not repair the commit.
        with redirect_stdout(io.StringIO()) as out, patch.object(cli, 'ROOT', self.root), patch.object(sys, 'argv', ['gate', '--staged', '--base', 'HEAD']):
            self.assertEqual(cli.main(), 1)
        self.assertIn('Reconcile source', out.getvalue())

    def prepare_hook(self):
        for name in ('check-re-coverage.py', 'coverage_gate.py', 'coverage_audit.py', 'index-coverage-sources.py'):
            self.write('tools/' + name, (ROOT/'tools'/name).read_text())
        self.write('.githooks/pre-commit', (ROOT/'.githooks/pre-commit').read_text())
        (self.root/'.githooks/pre-commit').chmod(0o755)
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        self.before = self.current()[0]
        self.init_git()

    def test_hook_blocks_commit_and_accepts_exact_staged_review(self):
        self.prepare_hook()
        hook_install.install(self.root)
        self.write('assets/model.cpp', 'changed\n')
        cli.git(self.root, 'add', 'assets/model.cpp')
        commit = subprocess.run(['git', '-C', str(self.root), 'commit', '-qm', 'must fail'], capture_output=True)
        self.assertNotEqual(commit.returncode, 0)
        self.assertIn(b'Coverage accounting blocked', commit.stderr)
        self.json(REVIEWS, self.receipt())
        cli.git(self.root, 'add', '.')
        cli.git(self.root, 'commit', '-qm', 'exact reviewed change')

    def test_hook_install_is_local_idempotent_and_reversible(self):
        self.prepare_hook()
        hook_install.install(self.root)
        hook_install.install(self.root)
        self.assertEqual(cli.git(self.root, 'config', '--local', '--get', 'core.hooksPath').strip(), b'.githooks')
        hook_install.install(self.root, uninstall=True)
        self.assertNotEqual(subprocess.run(['git', '-C', str(self.root), 'config', '--local', '--get', 'core.hooksPath'], capture_output=True).returncode, 0)

    def test_hook_install_preserves_custom_path(self):
        self.prepare_hook()
        cli.git(self.root, 'config', '--local', 'core.hooksPath', 'custom-hooks')
        with self.assertRaisesRegex(ValueError, 'Existing core.hooksPath'):
            hook_install.install(self.root)
        self.assertEqual(cli.git(self.root, 'config', '--local', '--get', 'core.hooksPath').strip(), b'custom-hooks')

    def test_hook_install_preserves_active_default_hook(self):
        self.prepare_hook()
        path = self.root/'.git/hooks/pre-push'
        path.write_text('#!/bin/sh\nexit 0\n')
        path.chmod(0o755)
        with self.assertRaisesRegex(ValueError, 'Existing active hooks'):
            hook_install.install(self.root)
        self.assertTrue(path.exists())


if __name__ == '__main__':
    unittest.main()
