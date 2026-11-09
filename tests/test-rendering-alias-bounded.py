#!/usr/bin/env python3
"""Retained-data and scope guards for the owner-approved overlap amendment."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
TOOL = ROOT/'tools/close-rendering-alias-bounded.py'
PARENT_ROOT = ROOT
PARENT = 'research/runtime/caller-state-master-results-20261108/admission-01.json'


class BoundedOverlap(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        sys.path.insert(0, str(ROOT/'tools'))
        spec = importlib.util.spec_from_file_location('alias_bounded_test_tool', TOOL)
        cls.tool = importlib.util.module_from_spec(spec); spec.loader.exec_module(cls.tool)
        cls.tool.ROOT = ROOT
        from coverage_evidence import Reader
        with Reader(PARENT_ROOT) as reader:
            cls.parent = reader.json(PARENT)
            cls.plan = reader.json('docs/native-rendering-closure.json')
        cls.plan['acceptance_scope_amendments'] = [copy.deepcopy(cls.tool.POLICY)]
        with Reader(ROOT) as reader:
            cls.proof_record = reader.json(cls.tool.PROOF)
            cls.members = cls.tool.unpack(reader, cls.proof_record)
            cls.proof = cls.tool.read_proof(reader)
        cls.native = cls.tool.module('alias_bounded_test_native', 'tools/check-native-surface-overlap.py')
        cls.rows, _ = cls.native.o.load(cls.native.o.DEFAULT)

    def test_real_raw_proof_and_exact_two_row_amendment(self):
        rows, amendment = self.tool.close_rows(self.plan, self.parent, self.proof)
        self.assertEqual(sum(r['state'] == 'passed' for r in rows), self.parent['passed_requirements']+2)
        self.assertEqual(sum(a != b for a,b in zip(rows, self.parent['requirements'])), 2)
        self.assertEqual(self.proof['cases'], 1536)
        self.assertTrue(all(self.proof['overlap_directions'].values()))
        self.assertEqual(amendment, self.tool.POLICY)
        for row in rows:
            if row['id'] == self.tool.LEAF:
                self.assertEqual(row['natural_player_reachability'], 'pending')
                self.assertEqual(row['actual_player_overlap_domains'], 0)
                self.assertEqual(row['admitted_execution_evidence'], [])
            elif row['id'] == 'gate.required-boundaries':
                self.assertEqual(row['state'], 'pending')

    def reject_policy(self, mutate):
        plan = copy.deepcopy(self.plan); mutate(plan)
        with self.assertRaises(ValueError): self.tool.policy(plan)

    def test_owner_approval_required(self):
        self.reject_policy(lambda p: p['acceptance_scope_amendments'][0]['approval'].update(response='no'))

    def test_scope_cannot_widen(self):
        self.reject_policy(lambda p: p['acceptance_scope_amendments'][0]['scope'].update(format='all'))

    def test_natural_scope_cannot_be_resolved(self):
        self.reject_policy(lambda p: next(d for d in p['completion_contract']['scope_decisions'] if d['id']=='SC.natural-callers').update(state='resolved'))

    def reject_raw(self, name, mutate, binary=False):
        members = dict(self.members)
        value = members[name] if binary else json.loads(members[name])
        changed = mutate(value)
        if changed is not None: value = changed
        members[name] = value if binary else json.dumps(value).encode()
        with self.assertRaises(ValueError):
            self.tool.compare_raw(self.rows, members, self.proof_record, self.native)

    def test_expected_output_cannot_be_native_input(self):
        self.reject_raw('execution/inputs.json', lambda x: x[0].update(destination_after=self.rows[0]['destination_after']))

    def test_actual_native_plane_must_match(self):
        self.reject_raw('execution/backend.json', lambda x: x['cases'][0]['destination_after'].__setitem__(0, x['cases'][0]['destination_after'][0]^1))

    def test_complete_backend_domain_required(self):
        self.reject_raw('execution/backend.json', lambda x: x['cases'].__delitem__(-1))

    def test_replay_command_bytes_bound(self):
        self.reject_raw('execution/stream-0.cmd', lambda x: bytes([x[0]^1])+x[1:], True)

    def test_replay_output_bytes_bound(self):
        self.reject_raw('execution/stream-0.pixels', lambda x: bytes([x[0]^1])+x[1:], True)

    def test_replay_result_and_retirement_bound(self):
        self.reject_raw('execution/stream-0.log', lambda x: x['surface_stats'].update(surfaces=1))

    def test_negative_stream_cannot_publish(self):
        self.reject_raw('execution/bad-result.log', lambda x: x.update(rendered=True))

    def reject_parent(self, mutate):
        parent = copy.deepcopy(self.parent); mutate(parent)
        with self.assertRaises(ValueError): self.tool.close_rows(self.plan, parent, self.proof)

    def test_missing_profile_row_refused(self):
        self.reject_parent(lambda p: p['requirements'].__delitem__(-1))

    def test_duplicate_row_refused(self):
        self.reject_parent(lambda p: p['requirements'].__setitem__(0, copy.deepcopy(p['requirements'][1])))

    def test_parent_count_substitution_refused(self):
        self.reject_parent(lambda p: p.update(passed_requirements=p['passed_requirements']+1))

    def test_prior_player_execution_credit_cannot_be_relabelled(self):
        self.reject_parent(lambda p: next(r for r in p['requirements'] if r['id']==self.tool.LEAF)['admitted_execution_evidence'].append({'supplied': True}))

    def test_base_annotation_review_bound(self):
        self.reject_parent(lambda p: p['definition_review'].update(current_definition_sha256='0'*64))

    def test_missing_annotation_review_is_exactly_reconstructed(self):
        parent = copy.deepcopy(self.parent); del parent['definition_review']
        review = self.tool.definition_review(self.plan, parent)
        self.assertEqual(review['preserved_dossier_identity'], parent['dossier_identity'])
        self.assertEqual(len(review['only_added_candidate_annotations']), 2)
        self.assertTrue(review['all_acceptance_conditions_and_owners_unchanged'])

    def test_missing_review_cannot_accept_another_annotation(self):
        parent = copy.deepcopy(self.parent); del parent['definition_review']
        plan = copy.deepcopy(self.plan)
        plan['required_gap_register']['operations'][0].setdefault('evidence_leads', []).append('unreviewed')
        with self.assertRaises(ValueError): self.tool.definition_review(plan, parent)

    def test_actual_merged_font_parent_preserves_all_other_rows(self):
        from coverage_evidence import Reader
        with Reader(ROOT) as reader:
            parent = reader.json('research/runtime/font-dc-master-results-20261109/admission-02.json')
        rows, _ = self.tool.close_rows(self.plan, parent, self.proof)
        self.assertEqual(sum(a != b for a,b in zip(rows, parent['requirements'])), 2)
        self.assertEqual(sum(r['state']=='passed' for r in rows), parent['passed_requirements']+2)

    def test_no_readmission_of_already_passed_row(self):
        def change(p):
            next(r for r in p['requirements'] if r['id']==self.tool.LEAF)['state']='passed'
            p['passed_requirements']+=1; p['pending_requirements']-=1
        self.reject_parent(change)

    def test_required_boundary_gate_stays_pending(self):
        def change(p):
            next(r for r in p['requirements'] if r['id']=='gate.required-boundaries')['state']='passed'
            p['passed_requirements']+=1; p['pending_requirements']-=1
        self.reject_parent(change)

    def test_controlled_proof_cannot_be_relabelled_natural(self):
        proof = dict(self.proof, natural_player_call_proved=True)
        with self.assertRaises(ValueError): self.tool.close_rows(self.plan, self.parent, proof)

    def test_raw_archive_hash_required(self):
        class Corrupt:
            def read_bytes(self, _): return b'wrong retained bytes'
        with self.assertRaises(ValueError): self.tool.unpack(Corrupt(), self.proof_record)


if __name__ == '__main__': unittest.main()
