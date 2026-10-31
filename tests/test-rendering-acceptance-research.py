#!/usr/bin/env python3
"""Provenance diagnostics never promote source matches or typed near-misses."""
from copy import deepcopy
from pathlib import Path
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_claims import behavior_contract
from rendering_acceptance_research import candidate_diagnostics, claim_diagnostics


class DiagnosticTests(unittest.TestCase):
    def setUp(self):
        self.behavior=dict(id='test',kind='native_policy',scope='Supplied research guard only',
            original=[],implementation=['code.cpp'],tests=['test.cpp'],validation_dependencies=['dependency.hpp'])
        self.behaviors={'test':self.behavior};self.builds={}
        self.claim=dict(behavior='test',contract_sha256=behavior_contract(self.behavior,{}))
        self.sources={n:'a'*64 for n in ['code.cpp','test.cpp','dependency.hpp']}
        self.evidence=dict(id='synthetic',record='research/runtime/synthetic.json',sha256='b'*64,
            scope='Synthetic guard only',kind='native_integration',sources_pointer='/sources',claims_pointer='/claims')
        self.verified=dict(payload_and_assertions_verified=True,
            current_source_binding_state='matching_recorded_sources',changed_sources=[])
        self.payload=dict(success=True,sources=self.sources,claims=[self.claim])

    def diagnose(self):
        return candidate_diagnostics(self.evidence,self.payload,self.verified,self.behaviors,self.builds)

    def test_complete_current_claim_is_still_not_player_admission(self):
        value=self.diagnose()
        self.assertEqual(value['claims'][0]['contract_state'],'matching')
        self.assertEqual(value['claims'][0]['missing_sources'],[])
        self.assertFalse(value['admitted_as_player_receipt'])
        self.assertFalse(value['closes_requirement'])

    def test_matching_source_bytes_do_not_rebind_changed_contract(self):
        self.claim['contract_sha256']='0'*64
        self.assertIn('execution_contract_changed_or_retired',self.diagnose()['blockers'])

    def test_matching_contract_does_not_supply_missing_dependency(self):
        self.sources.pop('dependency.hpp')
        value=self.diagnose()
        self.assertEqual(value['claims'][0]['missing_sources'],['dependency.hpp'])
        self.assertIn('execution_claim_source_closure_incomplete',value['blockers'])

    def test_success_one_is_not_boolean_execution_success(self):
        self.payload['success']=1
        self.assertIn('execution_success_not_true',self.diagnose()['blockers'])

    def test_negative_verified_assertion_cannot_become_positive_outcome(self):
        self.payload['success']=False
        self.assertIn('execution_success_not_true',self.diagnose()['blockers'])

    def test_static_evidence_and_unbound_execution_remain_candidates(self):
        self.evidence['kind']='static';self.evidence.pop('claims_pointer')
        value=self.diagnose()
        self.assertIn('not_execution_evidence',value['blockers'])
        self.assertIn('no_scope_bound_execution_claim',value['blockers'])

    def test_source_alias_preserves_complete_closure(self):
        self.payload['sources']={'historical.cpp':'a'*64,'test.cpp':'a'*64,'dependency.hpp':'a'*64}
        self.evidence['source_bindings']={'historical.cpp':'code.cpp','test.cpp':'test.cpp','dependency.hpp':'dependency.hpp'}
        self.assertEqual(self.diagnose()['claims'][0]['missing_sources'],[])

    def test_duplicate_and_retired_claims_cannot_supply_current_proof(self):
        with self.assertRaisesRegex(ValueError,'Duplicate'):
            claim_diagnostics([self.claim,deepcopy(self.claim)],self.behaviors,{},self.sources)
        self.behaviors.clear()
        self.assertIn('execution_contract_changed_or_retired',self.diagnose()['blockers'])


if __name__=='__main__':unittest.main()
