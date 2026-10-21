#!/usr/bin/env python3
"""Synthetic change accounting, immutable history, rename and real Git-base checks."""
import copy
from contextlib import contextmanager, redirect_stdout
import io
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import tarfile
import unittest
from unittest.mock import Mock, patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_gate import BASELINE, REGISTER, REVIEWS, check, changes, digest, draft, snapshot, source_index, file_digest
from coverage_claims import behavior_contract, scenario_contract
from coverage_reviews import ARCHIVES, load_journal, read_journal, split_journal

spec = importlib.util.spec_from_file_location('gate_cli', ROOT/'tools/check-re-coverage.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)
spec = importlib.util.spec_from_file_location('hook_install', ROOT/'tools/install-coverage-hook.py')
hook_install = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook_install)
spec = importlib.util.spec_from_file_location('claim_draft', ROOT/'tools/draft-coverage-claims.py')
claim_draft = importlib.util.module_from_spec(spec)
spec.loader.exec_module(claim_draft)


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
        self.record = {'source_sha256': 'a'*64, 'success': True, 'sources': {
            'assets/model.cpp': self.sha('assets/model.cpp'), 'tests/model.py': self.sha('tests/model.py')}}
        self.json('research/record.json', self.record)
        ev = {'id': 'old-original', 'kind': 'isolated_original', 'scope': 'bounded helper', 'build': 'fixture',
              'record': 'research/record.json', 'sha256': self.sha('research/record.json'), 'sources_pointer': '/sources',
              'original_hash_pointer': '/source_sha256', 'assertions': [{'pointer': '/success', 'equals': True}]}
        b = {'id': 'MV.test', 'subsystem': 'movement', 'kind': 'recovered', 'title': 'Selected movement', 'scope': 'One bounded helper',
             'confidence': 'provisional', 'original': [{'build': 'fixture', 'address': 4096, 'scope': 'entry'}],
             'documents': ['research/finding.md'], 'implementation': ['assets/model.cpp'], 'tests': ['tests/model.py'],
             'evidence': ['old-original'], 'status': {'understanding': 'partial', 'implementation': 'partial',
                                                   'comparison': 'recorded', 'integration': 'none', 'replacement': 'none'}}
        self.register = {'schema': 1, 'claim_bindings_required': True, 'builds': [{'id': 'fixture', 'source_sha256': 'a'*64, 'inventory': 'research/inventory.json',
                                                'inventory_sha256': self.sha('research/inventory.json')}],
                         'behaviors': [b], 'evidence': [ev], 'scenarios': [], 'dispatch_tables': [],
                         'classifications': [], 'api_bindings': [], 'recovered_ranges': []}
        self.record['claims'] = [{'behavior': b['id'],
                                 'contract_sha256': behavior_contract(b, {'fixture': self.register['builds'][0]})}]
        self.json('research/record.json', self.record)
        ev.update(sha256=self.sha('research/record.json'), claims_pointer='/claims')
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

    def test_snapshot_fingerprints_shared_paths_once_without_cross_call_reuse(self):
        with patch('coverage_gate.file_digest', wraps=file_digest) as fingerprints:
            first, _ = snapshot(self.root)
        paths = [call.args[0] for call in fingerprints.call_args_list]
        self.assertEqual(paths.count(self.root/'assets/model.cpp'), 1)
        # Research participates both in source and immutable-artifact accounting.
        self.assertEqual(paths.count(self.root/'research/record.json'), 1)
        self.write('assets/model.cpp', 'changed after snapshot\n')
        second, report = self.current()
        self.assertNotEqual(first['behaviors'], second['behaviors'])
        self.assertTrue(report['findings']['stale_evidence'])

    def test_snapshot_reuses_audited_provenance_with_bound_and_input_keys(self):
        self.record['sources'] = {'old-model.cpp': self.sha('assets/model.cpp'), 'input.bin': 'b'*64}
        self.json('research/record.json', self.record)
        evidence = self.register['evidence'][0]
        evidence.update(sha256=self.sha('research/record.json'),
                        source_bindings={'old-model.cpp':'assets/model.cpp', 'input.bin':'input.bin'},
                        recorded_inputs=['input.bin'])
        result, _ = self.current()
        self.assertEqual(result['evidence_keys']['old-original'], ['input.bin', 'old-model.cpp'])
        self.assertEqual(result['evidence_sources']['old-original'],
                         {'assets/model.cpp':self.sha('assets/model.cpp')})

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

    def test_sharded_journal_preserves_receipts_and_append_only_enforcement(self):
        self.write('assets/model.cpp', 'first\n')
        reviews = self.receipt()
        # Force two shards, keeping complete receipts and their order intact.
        extra = copy.deepcopy(reviews['reviews'][0])
        extra['id'] += '-second'
        reviews['reviews'].append(extra)
        self.json(REVIEWS, reviews)
        before, _ = self.current()
        size = len(json.dumps(extra, separators=(',', ':'))) + 1200
        split_journal(self.root, max_bytes=size)
        loaded, paths = load_journal(self.root)
        self.assertEqual(loaded['reviews'], reviews['reviews'])
        self.assertGreater(len(paths), 1)
        after, report = self.current()
        self.assertEqual(changes(before, after)['files'], [])
        self.assertEqual(check(before, after, loaded, report)['errors'], [])
        self.assert_error(check(before, after, dict(loaded, reviews=loaded['reviews'][1:]), report),
                          'Historical change review removed or rewritten')
        # The same byte reader is used for historical Git trees.
        historical, _ = read_journal(lambda name: (self.root/name).read_bytes())
        self.assertEqual(historical['reviews'], loaded['reviews'])
        revision = self.init_git()
        historical, _ = read_journal(lambda name: cli.git(self.root, 'show', revision + ':' + name))
        self.assertEqual(historical['reviews'], loaded['reviews'])
        with cli.staged_tree(self.root) as staged:
            self.assertEqual(load_journal(staged)[0]['reviews'], loaded['reviews'])
        archive = next(p for p in paths if p != REVIEWS)
        self.write(archive, '{}\n')
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            self.current()

    def test_sharded_journal_rejects_escaping_duplicate_and_missing_archives(self):
        sha = 'a' * 64
        entry = {'path': ARCHIVES + sha + '.json', 'sha256': sha}
        self.json(REVIEWS, {'schema': 1, 'reviews': [], 'archives': [entry]})
        with self.assertRaises(FileNotFoundError):
            load_journal(self.root)
        entry['path'] = '../outside.json'
        self.json(REVIEWS, {'schema': 1, 'reviews': [], 'archives': [entry]})
        with self.assertRaisesRegex(ValueError, 'Invalid review archive path'):
            load_journal(self.root)
        data = b'{"schema":1,"reviews":[]}'
        sha = hashlib.sha256(data).hexdigest()
        entry = {'path': ARCHIVES + sha + '.json', 'sha256': sha}
        self.write(entry['path'], data.decode())
        self.json(REVIEWS, {'schema': 1, 'reviews': [], 'archives': [entry, entry]})
        with self.assertRaisesRegex(ValueError, 'Duplicate review archive'):
            load_journal(self.root)

    def test_sharding_rejects_oversized_receipt_without_changing_journal(self):
        self.json(REVIEWS, {'schema': 1, 'reviews': [{'id': 'large', 'reason': 'x' * 1000}]})
        original = (self.root/REVIEWS).read_bytes()
        with self.assertRaisesRegex(ValueError, 'One review exceeds'):
            split_journal(self.root, max_bytes=500)
        self.assertEqual((self.root/REVIEWS).read_bytes(), original)

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
        self.register['evidence'][0]['source_bindings'] = {'assets/model.cpp': 'assets/renamed.cpp',
                                                        'tests/model.py': 'tests/model.py'}
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
        self.register['evidence'][0].pop('claims_pointer')
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=['old-original'])
        self.assert_error(self.result(receipt), 'validation evidence missing, static or stale')

    def test_fresh_execution_evidence_can_validate_a_change(self):
        self.write('assets/model.cpp', 'changed\n')
        new = {**self.register['evidence'][0], 'id': 'new-original', 'record': 'research/new-record.json'}
        record = {**self.record, 'sources': {'assets/model.cpp': self.sha('assets/model.cpp'),
                                           'tests/model.py': self.sha('tests/model.py')}}
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

    def stage_native_receipt(self, stages):
        self.write('assets/model.cpp', 'new native implementation\n')
        behavior = self.register['behaviors'][0]
        behavior['status']['implementation'] = 'scoped'
        record = dict(self.record, sources={'assets/model.cpp': self.sha('assets/model.cpp'),
                                           'tests/model.py': self.sha('tests/model.py')})
        self.json('research/stage-native.json', record)
        evidence = dict(self.register['evidence'][0], id='stage-native', kind='native_integration',
                        record='research/stage-native.json', sha256=self.sha('research/stage-native.json'))
        self.register['evidence'].append(evidence)
        behavior['evidence'].append(evidence['id'])
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=[evidence['id']])
        if stages is not None:
            receipt['reviews'][0]['validation']['stages'] = stages
        return receipt

    def test_implementation_receipt_does_not_require_renewing_historical_comparison(self):
        receipt = self.stage_native_receipt(['implementation'])
        self.assertEqual(self.result(receipt)['errors'], [])
        _, audit = self.current()
        validation = audit['behaviors'][0]['current_validation']
        self.assertEqual(validation['implementation']['state'], 'current')
        self.assertEqual(validation['comparison']['state'], 'pending')

    def test_default_recorded_receipt_still_requires_all_declared_milestones(self):
        self.assert_error(self.result(self.stage_native_receipt(None)),
                          'fresh evidence kind does not support current validation levels')

    def test_implementation_receipt_cannot_promote_integration(self):
        receipt = self.stage_native_receipt(['implementation'])
        self.register['behaviors'][0]['status']['integration'] = 'headless'
        self.register['scenarios'].append({'id': 'native-case', 'scope': 'Bounded headless case',
                                          'level': 'headless', 'behaviors': ['MV.test'],
                                          'tests': ['tests/model.py'], 'evidence': ['stage-native']})
        scenario = self.register['scenarios'][-1]
        record = json.loads((self.root/'research/stage-native.json').read_text())
        record['scenarios'] = [{'scenario': scenario['id'], 'contract_sha256': scenario_contract(scenario)}]
        self.json('research/stage-native.json', record)
        self.register['evidence'][-1].update(sha256=self.sha('research/stage-native.json'),
                                             scenarios_pointer='/scenarios')
        # Refresh the exact delta, preserving the deliberately insufficient stage credit.
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', stages=['implementation'],
                                                   evidence=['stage-native'])
        self.assert_error(self.result(receipt), 'Status promotion requires fresh recorded validation')

    def test_implementation_receipt_cannot_promote_original_comparison(self):
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        baseline, _ = self.current()
        self.stage_native_receipt(['implementation'])
        self.register['behaviors'][0]['status']['comparison'] = 'recorded'
        receipt = self.receipt(before=baseline)
        receipt['reviews'][0]['validation'].update(status='recorded', stages=['implementation'],
                                                   evidence=['stage-native'])
        self.assert_error(self.result(receipt, before=baseline),
                          'Status promotion requires fresh recorded validation')

    def test_explicit_comparison_validation_still_needs_original_evidence(self):
        self.assert_error(self.result(self.stage_native_receipt(['implementation', 'comparison'])),
                          'fresh evidence kind does not support current validation levels')

    def test_invalid_validation_stages_refuse(self):
        receipt = self.stage_native_receipt(['implementation'])
        for stages in ([], ['implementation', 'implementation'], ['understanding'], 'implementation', [None]):
            with self.subTest(stages=stages):
                receipt['reviews'][0]['validation']['stages'] = stages
                self.assert_error(self.result(receipt), 'stages requires a nonempty unique list')

    def test_pending_receipt_cannot_claim_stage_credit(self):
        receipt = self.receipt()
        receipt['reviews'][0]['validation']['stages'] = ['implementation']
        self.assert_error(self.result(receipt), 'stages requires a nonempty unique list')

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

    def test_artifact_only_recorded_receipt_requires_later_exact_pending_review(self):
        self.json('research/new-record.json', self.record)
        new = {**self.register['evidence'][0], 'id': 'new-original',
               'record': 'research/new-record.json', 'sha256': self.sha('research/new-record.json')}
        self.register['evidence'].append(new)
        self.register['behaviors'][0]['evidence'].append(new['id'])
        older = self.receipt()
        entry = older['reviews'][0]
        entry['id'] = 'artifact-recorded'
        entry['files'] = [f for f in entry['files'] if f['path'] == new['record']]
        entry['behaviors'] = []
        entry['validation'].update(status='recorded', evidence=[new['id']])
        self.write('assets/model.cpp', 'Later source closure\n')
        self.assert_error(self.result(older), 'validation evidence missing, static or stale')
        latest = self.receipt()
        latest['reviews'][0]['id'] = 'exact-current-pending'
        latest['reviews'] = older['reviews'] + latest['reviews']
        self.assertEqual(self.result(latest)['errors'], [])
        self.assertTrue(any(g['kind'] == 'stale_evidence' for g in self.result(latest)['changes']['new_gaps'].values()))
        # A prior pending receipt cannot retire a subsequent recorded claim.
        reversed_reviews = {**latest, 'reviews': list(reversed(latest['reviews']))}
        self.assert_error(self.result(reversed_reviews), 'validation evidence missing, static or stale')
        # Code and current contract claims still require fresh execution proof.
        code_claim = copy.deepcopy(latest)
        code_claim['reviews'][0]['files'].extend(f for f in latest['reviews'][1]['files'] if f['path'] == 'assets/model.cpp')
        self.assert_error(self.result(code_claim), 'validation evidence missing, static or stale')
        contract_claim = copy.deepcopy(latest)
        contract_claim['reviews'][0]['behaviors'] = latest['reviews'][1]['behaviors']
        self.assert_error(self.result(contract_claim), 'validation evidence missing, static or stale')

    def test_root_build_and_dependency_files_are_watched(self):
        self.write('CMakeLists.txt', 'add_subdirectory(assets)\n')
        self.write('Makefile', 'all:\n\tfalse\n')
        self.write('tools/compiler.cmake', 'set(CMAKE_C_FLAGS unsafe)\n')
        result = self.result()
        for path in ('CMakeLists.txt', 'Makefile', 'tools/compiler.cmake'):
            self.assert_error(result, 'Unreviewed source change: ' + path)

    def test_execution_claim_policy_cannot_be_disabled_after_adoption(self):
        self.register['claim_bindings_required'] = False
        self.assert_error(self.result(self.receipt()), 'Execution-claim policy cannot be disabled')

    def test_pre_adoption_history_keeps_its_accounting_contract(self):
        self.register['claim_bindings_required'] = False
        before, _ = self.current()
        self.register['behaviors'][0]['scope'] += ' Historical scope clarification.'
        after, report = self.current()
        self.assertTrue(report['findings']['validation_gaps'])
        self.assertFalse(any(g['kind'] == 'validation_gaps' for g in changes(before, after)['new_gaps'].values()))
        self.assertEqual(self.result(self.receipt(before=before), before=before)['errors'], [])

    def test_claim_declaration_names_exact_scope_without_success_assertions(self):
        value = claim_draft.declaration(self.root, self.register, ['MV.test'], [])
        self.assertNotIn('success', value)
        self.assertEqual(value['sources'], {'assets/model.cpp': self.sha('assets/model.cpp'),
                                           'tests/model.py': self.sha('tests/model.py')})
        self.assertEqual(value['claims'][0]['contract_sha256'], self.record['claims'][0]['contract_sha256'])
        with self.assertRaisesRegex(ValueError, 'Unknown behavior'):
            claim_draft.declaration(self.root, self.register, ['unknown'], [])

    def test_parent_receipts_compose_without_a_new_branch_receipt(self):
        self.write('assets/model.cpp', 'middle\n')
        middle, _ = self.current()
        first = self.receipt(after=middle)
        self.write('assets/model.cpp', 'final\n')
        second = self.receipt(before=middle)
        result = self.result({'schema': 1, 'reviews': first['reviews'] + second['reviews']})
        self.assertEqual(result['errors'], [])
        self.assertIn(first['reviews'][0]['id'], result['receipt_chains'][second['reviews'][0]['id']])

    def test_broken_or_unreviewed_receipt_chain_cannot_approve(self):
        self.write('assets/model.cpp', 'middle\n')
        middle, _ = self.current()
        first = self.receipt(after=middle)
        self.write('assets/model.cpp', 'final\n')
        second = self.receipt(before=middle)
        self.assert_error(self.result(second), 'Unreviewed source change: assets/model.cpp')
        first['reviews'][0]['reason'] = ''
        self.assert_error(self.result({'schema': 1, 'reviews': first['reviews'] + second['reviews']}),
                          'Unreviewed source change: assets/model.cpp')

    def test_recorded_receipt_requires_bound_scope_even_with_fresh_sources(self):
        self.write('assets/model.cpp', 'new implementation\n')
        record = copy.deepcopy(self.record)
        record['sources']['assets/model.cpp'] = self.sha('assets/model.cpp')
        self.json('research/unbound.json', record)
        ev = {**self.register['evidence'][0], 'id': 'fresh-unbound', 'record': 'research/unbound.json',
              'sha256': self.sha('research/unbound.json')}
        ev.pop('claims_pointer')
        self.register['evidence'].append(ev)
        self.register['behaviors'][0]['evidence'].append(ev['id'])
        receipt = self.receipt()
        receipt['reviews'][0]['validation'].update(status='recorded', evidence=[ev['id']])
        self.assert_error(self.result(receipt), 'requires scope-bound execution evidence')

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

    def test_inherited_milestone_still_requires_exact_source_and_gap_reviews(self):
        self.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.write('assets/model.cpp', 'merged implementation\n')
        after, audit_report = self.current()
        history = {'statuses': {'MV.test': {'commit': 'fixture-parent'}}, 'reviews': [], 'parents': []}
        result = check(self.before, after, {'schema': 1, 'reviews': []}, audit_report, history)
        self.assert_error(result, 'Unreviewed source change')
        self.assertTrue(any(e.startswith('New gap needs explicit pending accounting') for e in result['errors']))
        self.assertFalse(any(e.startswith('Status promotion') for e in result['errors']))
        self.assertEqual(check(self.before, after, self.receipt(), audit_report, history)['errors'], [])

    def test_imported_recorded_receipt_grants_no_current_validation_credit(self):
        self.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.write('assets/model.cpp', 'merged implementation\n')
        receipt = self.receipt()
        r = receipt['reviews'][0]
        r['validation'].update(status='recorded', evidence=['old-original'])
        after, audit_report = self.current()
        history = {'statuses': {}, 'reviews': [r['id']], 'parents': []}
        result = check(self.before, after, receipt, audit_report, history)
        self.assert_error(result, 'Unreviewed source change')
        self.assert_error(result, 'Status promotion requires fresh recorded validation')
        self.assertFalse(any('validation evidence missing, static or stale' in e for e in result['errors']))

    def test_real_git_base_cannot_be_reset_by_current_baseline_edit(self):
        revision = self.init_git()
        self.write('assets/model.cpp', 'changed\n')
        after, report = self.current()
        self.json(BASELINE, after)
        git_before, commit = cli.git_snapshot(self.root, revision)
        self.assertEqual(commit, revision)
        self.assertEqual(git_before['sources']['assets/model.cpp']['sha256'], self.before['sources']['assets/model.cpp']['sha256'])
        self.assert_error(check(git_before, after, {'schema': 1, 'reviews': []}, report), 'Unreviewed source change')

    def test_git_archive_stream_preserves_the_complete_snapshot(self):
        revision = self.init_git()
        expected, _ = snapshot(self.root)
        with patch.object(cli, 'git', wraps=cli.git) as calls:
            actual, commit = cli.git_snapshot(self.root, revision)
        self.assertEqual(actual, expected)
        self.assertEqual(commit, revision)
        self.assertFalse(any('archive' in call.args for call in calls.call_args_list))

    def test_complete_tar_does_not_hide_a_failed_git_export(self):
        archive = io.BytesIO()
        with tarfile.open(fileobj=archive, mode='w'):
            pass
        export = Mock(stdout=io.BytesIO(archive.getvalue()), returncode=7)
        export.wait.return_value = 7
        export.poll.return_value = 7
        with patch.object(cli, 'git', return_value=b'a'*40+b'\n'), patch.object(
                cli.subprocess, 'Popen', return_value=export):
            with self.assertRaises(subprocess.CalledProcessError) as failure:
                cli.git_snapshot(self.root, 'HEAD')
        self.assertEqual(failure.exception.returncode, 7)
        self.assertTrue(export.stdout.closed)

    def test_malformed_archive_stops_the_exporter(self):
        export = Mock(stdout=io.BytesIO(b'invalid tar'))
        export.poll.return_value = None
        with patch.object(cli, 'git', return_value=b'a'*40+b'\n'), patch.object(
                cli.subprocess, 'Popen', return_value=export):
            with self.assertRaises(tarfile.ReadError):
                cli.git_snapshot(self.root, 'HEAD')
        export.terminate.assert_called_once_with()
        export.wait.assert_called_once_with()
        self.assertTrue(export.stdout.closed)

    def test_cli_optional_phase_timings_are_recorded_without_changing_checks(self):
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        revision = self.init_git()
        output = self.root/'timed-report.json'
        with redirect_stdout(io.StringIO()), patch.object(cli, 'ROOT', self.root), patch.object(
                sys, 'argv', ['gate', '--base', revision, '--timings', '--json', str(output)]):
            self.assertEqual(cli.main(), 0)
        report = json.loads(output.read_text())
        self.assertEqual(report['errors'], [])
        self.assertEqual(set(report['timings_seconds']),
                         {'current_snapshot', 'git_base_snapshot', 'merge_history', 'review_check'})
        self.assertTrue(all(seconds >= 0 for seconds in report['timings_seconds'].values()))

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

    def test_staged_copy_released_before_git_base_and_reviews_remain_frozen(self):
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        revision = self.init_git()
        self.write('assets/model.cpp', 'reviewed staged change\n')
        self.json(REVIEWS, self.receipt())
        cli.git(self.root, 'add', '.')
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        materialized = []
        original_tree, original_base = cli.staged_tree, cli.git_snapshot

        @contextmanager
        def staged(root):
            with original_tree(root) as tree:
                materialized.append(tree)
                yield tree

        def base(root, rev):
            self.assertTrue(materialized)
            self.assertFalse(materialized[-1].exists())
            return original_base(root, rev)

        with redirect_stdout(io.StringIO()) as out, patch.object(cli, 'ROOT', self.root), patch.object(cli, 'staged_tree', staged), patch.object(cli, 'git_snapshot', base), patch.object(sys, 'argv', ['gate', '--staged', '--base', revision]):
            self.assertEqual(cli.main(), 0, out.getvalue())
        self.assertTrue(all(not p.exists() for p in materialized))

    def test_staged_local_baseline_is_captured_before_tree_release(self):
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        self.init_git()
        self.write('assets/model.cpp', 'reviewed staged change\n')
        self.json(REVIEWS, self.receipt())
        cli.git(self.root, 'add', '.')
        self.json(BASELINE, {'invalid': 'unstaged baseline'})
        self.json(REVIEWS, {'schema': 1, 'reviews': []})
        with redirect_stdout(io.StringIO()) as out, patch.object(cli, 'ROOT', self.root), patch.object(sys, 'argv', ['gate', '--staged']):
            self.assertEqual(cli.main(), 0, out.getvalue())

    def test_freeze_baseline_does_not_require_reviews(self):
        target = self.root/'frozen.json'
        with redirect_stdout(io.StringIO()) as out, patch.object(cli, 'ROOT', self.root), patch.object(sys, 'argv', ['gate', '--freeze-baseline', str(target)]):
            self.assertEqual(cli.main(), 0, out.getvalue())
        self.assertEqual(json.loads(target.read_text())['sources'], self.before['sources'])

    def prepare_hook(self):
        for name in ('check-re-coverage.py', 'coverage_gate.py', 'coverage_audit.py', 'coverage_claims.py', 'coverage_evidence.py', 'coverage_merge_history.py', 'coverage_reviews.py', 'index-coverage-sources.py'):
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
        self.assertIn(b'your staged files are intact', commit.stderr)
        self.assertIn(b'--draft-review', commit.stderr)
        self.assertIn(b'--staged --base HEAD', commit.stderr)
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

    def test_shared_absolute_hooks_keep_current_tooling_and_other_root(self):
        self.prepare_hook()
        path = str((self.root/'.githooks').resolve())
        cli.git(self.root, 'config', '--local', 'core.hooksPath', path)
        hook_install.install(self.root)
        self.assertEqual(cli.git(self.root, 'config', '--local', '--get', 'core.hooksPath').decode().strip(),path)
        with redirect_stdout(io.StringIO()) as out, patch.object(cli,'ROOT',ROOT), patch.object(sys,'argv',['gate','--root',str(self.root),'--base','HEAD']):
            self.assertEqual(cli.main(),0,out.getvalue())

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
