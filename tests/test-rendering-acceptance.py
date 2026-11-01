#!/usr/bin/env python3
"""Acceptance records preserve pending proof, immutable events and separate verdicts."""
from copy import deepcopy
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sqlite3
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_claims import fingerprint
from rendering_acceptance import Workspace, assemble, requirements, validate, verify_evidence


class AcceptanceTests(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        self.spec = self.plan['acceptance_record']
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.workspace = Workspace(self.root, self.plan)
        self.addCleanup(self.workspace.close)
        self.prerequisite = dict(evidence_applicability_seed=[dict(id='bounded', kind='static',
            current_source_binding_state='matching_recorded_sources', closes_requirement=False)])

    def record(self):
        return assemble(self.plan, self.prerequisite, self.workspace, [], dict(scope='Synthetic host only'))

    def attachment(self):
        row = self.workspace.rows[0]
        return dict(configuration=row['configuration'], requirement=row['id'], reason='Bounded fixture lead only',
                    evidence=dict(id='TEST.static', record='research/runtime/fixture.json', sha256='a'*64))

    def test_complete_denominator_and_both_configurations(self):
        rows = requirements(self.plan)
        self.assertEqual(len(rows), 156)
        for config in ['player-default', 'player-software']:
            subset = [r for r in rows if r['configuration'] == config]
            self.assertEqual(len(subset), 78)
            self.assertEqual(sum(r['kind'] == 'journey' for r in subset), 47)
            self.assertEqual(sum(r['kind'] == 'operation' for r in subset), 21)
            self.assertEqual(sum(r['kind'] == 'gate' for r in subset), 10)

    def test_initial_record_is_valid_but_every_acceptance_row_pending(self):
        report = self.record()
        self.assertTrue(report['record_valid'])
        self.assertEqual(report['counts']['pending_rows'], 156)
        self.assertEqual({r['state'] for r in report['requirements']}, {'pending'})
        for name in ['supported_player_accepted', 'original_recovery_complete', 'engine_acceptance_assessed',
                     'engine_execution_performed', 'original_media_consumed', 'complete_replacement']:
            self.assertIs(report[name], False)
        self.assertEqual(report['status_promotions'], [])

    def test_all_three_prerequisite_fingerprints_are_required(self):
        for name in ['completion_contract', 'required_gap_register', 'execution_plan']:
            original = deepcopy(self.spec['bindings'][name]);self.spec['bindings'][name]['sha256'] = '0'*64
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'input binding'):validate(self.plan)
            self.spec['bindings'][name] = original

    def test_static_mode_and_positive_admission_cannot_be_promoted(self):
        self.spec['mode'] = 'accepted'
        with self.assertRaisesRegex(ValueError, 'cannot certify'):validate(self.plan)
        self.spec['mode'] = 'opening_pending_record'
        self.spec['positive_execution_admission']['state'] = 'passed'
        with self.assertRaisesRegex(ValueError, 'real pilot'):validate(self.plan)

    def test_policy_and_workspace_boundary_cannot_weaken(self):
        self.spec['evidence_policy']['no_pass_from_attachment'] = False
        with self.assertRaisesRegex(ValueError, 'rule weakened'):validate(self.plan)
        self.spec['evidence_policy']['no_pass_from_attachment'] = True
        self.spec['workspace'] = '../outside.sqlite3'
        with self.assertRaisesRegex(ValueError, 'Unsafe acceptance workspace'):validate(self.plan)

    def test_player_and_original_recovery_decisions_stay_separate(self):
        report = self.record()
        self.assertEqual([d['id'] for d in report['decisions']], ['supported-player', 'original-recovery'])
        self.assertEqual([d['state'] for d in report['decisions']], ['pending', 'pending'])
        self.assertEqual(len(report['original_recovery']), 15)
        self.spec['decision_ids'] = ['combined-completion']
        with self.assertRaisesRegex(ValueError, 'remain separate'):validate(self.plan)

    def test_source_review_only_starts_ep01(self):
        self.workspace.append('source_applicability_review', dict(task='EP01', sources={'code.cpp':'a'*64},
            evidence_identities=[dict(id='lead',sha256='b'*64)], limits='Synthetic source observation; no build or player identity'))
        report = self.record()
        self.assertEqual(report['tasks'][0]['state'], 'in_progress')
        self.assertFalse(report['tasks'][0]['completed'])
        self.assertFalse(report['ep01_review']['complete'])
        self.assertEqual(report['ep01_review']['actual_player_build_identity'], 'pending')
        self.assertTrue(all(t['state'] == 'not_started' for t in report['tasks'][1:]))

    def test_guard_or_source_pass_cannot_complete_ep01(self):
        self.spec['first_work']['complete'] = True
        with self.assertRaisesRegex(ValueError, 'cannot close execution-input'):validate(self.plan)

    def test_attachments_do_not_pass_even_source_matching_static_evidence(self):
        self.workspace.append('candidate_attachment', self.attachment())
        report = self.record()
        self.assertEqual(report['requirements'][0]['attached_candidates'], ['TEST.static'])
        self.assertEqual(report['requirements'][0]['state'], 'pending')
        self.assertEqual(report['counts']['passed_rows'], 0)

    def test_duplicate_attachment_is_idempotent(self):
        first = self.workspace.append('candidate_attachment', self.attachment())
        second = self.workspace.append('candidate_attachment', self.attachment())
        self.assertEqual(first, second)
        self.assertEqual(len(self.workspace.events()), 1)

    def test_unknown_configuration_or_requirement_refuses(self):
        for field in ['configuration', 'requirement']:
            payload = self.attachment();payload[field] = 'invented'
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'Unknown configuration'):self.workspace.append('candidate_attachment', payload)

    def test_attachment_needs_reason_and_cannot_carry_pass_flag(self):
        payload = self.attachment();payload['reason'] = ' '
        with self.assertRaisesRegex(ValueError, 'review reason'):self.workspace.append('candidate_attachment', payload)
        payload = self.attachment();payload['accepted'] = True
        with self.assertRaisesRegex(ValueError, 'cannot carry acceptance'):self.workspace.append('candidate_attachment', payload)

    def test_unknown_event_kind_refuses(self):
        with self.assertRaisesRegex(ValueError, 'Unsupported acceptance event'):
            self.workspace.append('set_pass', dict(accepted=True))

    def test_sqlite_events_and_definitions_cannot_be_rewritten(self):
        self.workspace.append('candidate_attachment', self.attachment())
        for sql in ['UPDATE events SET payload=?', 'DELETE FROM events',
                    'UPDATE dossiers SET definition=?', 'DELETE FROM dossiers']:
            with self.subTest(sql=sql), self.assertRaisesRegex(sqlite3.IntegrityError, 'Immutable'):
                self.workspace.db.execute(sql, ('changed',) if '?' in sql else ())
            self.workspace.db.rollback()

    def test_pending_scopes_and_negative_history_do_not_disappear(self):
        report = self.record()
        self.assertEqual(len(report['scope_decisions']), 8)
        self.assertTrue(all(r['state'] == 'unresolved' for r in report['scope_decisions']))
        self.assertEqual(len(report['historical_observations']), 2)
        self.assertEqual(report['counts']['failed_current_rows'], 0)

    def test_evidence_verification_checks_hash_and_assertions_without_accepting_scope(self):
        class FakeReader:
            def digest(self, path):return 'a'*64
            def json(self, path):return dict(success=True, sources={'code.cpp':'b'*64})
        register=dict(evidence=[dict(id='fixture',record='research/runtime/fixture.json',sha256='a'*64,
            kind='static',scope='Synthetic static fixture',sources_pointer='/sources',
            assertions=[dict(pointer='/success',equals=True)])])
        result=verify_evidence(self.root,FakeReader(),register,'fixture')
        self.assertFalse(result['closes_requirement'])
        self.assertEqual(result['current_source_binding_state'],'changed')
        register['evidence'][0]['sha256']='c'*64
        with self.assertRaisesRegex(ValueError,'payload hash'):verify_evidence(self.root,FakeReader(),register,'fixture')
        register['evidence'][0]['sha256']='a'*64
        register['evidence'][0]['assertions'][0]['equals']=False
        with self.assertRaisesRegex(ValueError,'assertion failed'):verify_evidence(self.root,FakeReader(),register,'fixture')

    def test_unregistered_candidate_refuses(self):
        with self.assertRaisesRegex(ValueError, 'Unknown registered evidence'):
            verify_evidence(self.root, None, dict(evidence=[]), 'invented')

    def test_durable_backup_restores_exact_ledger(self):
        self.workspace.append('candidate_attachment', self.attachment())
        receipt = self.workspace.backup()
        restored = sqlite3.connect(receipt['path'])
        try:
            self.assertEqual(restored.execute('SELECT id,definition FROM dossiers').fetchall(),
                             self.workspace.db.execute('SELECT id,definition FROM dossiers').fetchall())
            self.assertEqual(restored.execute('SELECT id,payload FROM events').fetchall(),
                             self.workspace.db.execute('SELECT id,payload FROM events').fetchall())
        finally:restored.close()

    def test_tampered_event_identity_refuses(self):
        self.workspace.db.execute('INSERT INTO events VALUES (?,?,?,?,?)',
            ('bad',self.workspace.identity,'candidate_attachment',json.dumps(self.attachment()),'fixture'))
        self.workspace.db.commit()
        with self.assertRaisesRegex(ValueError,'event identity'):self.workspace.events()

    def test_exported_snapshot_restores_events_idempotently(self):
        self.workspace.append('candidate_attachment', self.attachment())
        snapshot = self.record()
        with tempfile.TemporaryDirectory() as target:
            restored = Workspace(target, self.plan)
            try:
                self.assertEqual(restored.restore_snapshot(snapshot), 1)
                restored.restore_snapshot(snapshot)
                self.assertEqual(restored.events(), self.workspace.events())
            finally:restored.close()

    def test_restore_cannot_change_dossier_or_import_passed_outcomes(self):
        snapshot = self.record();snapshot['dossier_identity'] = 'wrong'
        with self.assertRaisesRegex(ValueError, 'different acceptance dossier'):self.workspace.restore_snapshot(snapshot)
        snapshot = self.record();snapshot['decisions'][0]['state'] = 'pass'
        with self.assertRaisesRegex(ValueError, 'accepted outcomes'):self.workspace.restore_snapshot(snapshot)
        snapshot = self.record();snapshot['requirements'].pop()
        with self.assertRaisesRegex(ValueError, 'denominator'):self.workspace.restore_snapshot(snapshot)

    def test_acceptance_cli_never_invokes_original_or_player_commands(self):
        spec = importlib.util.spec_from_file_location('acceptance_cli', ROOT/'tools/reconcile-rendering-dependencies.py')
        cli = importlib.util.module_from_spec(spec);spec.loader.exec_module(cli)
        report=self.record();report['sources']={}
        with patch.object(cli, 'ROOT', self.root), patch('rendering_acceptance.review', return_value=report), \
             patch('evidence_local.local_evidence', side_effect=lambda _: contextlib.nullcontext()), \
             patch.object(cli.subprocess, 'run', side_effect=AssertionError('Original/player command invoked')), \
             patch.object(cli, 'write_report', return_value='fixture'), \
             patch.object(sys, 'argv', ['reconcile','--acceptance-only','--report','research/runtime/check.json']), \
             patch('sys.stdout', new_callable=io.StringIO):
            self.assertEqual(cli.main(),0)


if __name__ == '__main__':
    unittest.main()
