#!/usr/bin/env python3
"""Actual-data refusal controls for independent pending artifact5 verdicts."""
import copy,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
import rendering_final_decisions as gate

class Proxy:
    def __init__(self,r,reports):self.reader,self.reports=r,reports;self.root=r.root
    def digest(self,n):return self.reader.digest(n)
    def json(self,n):return copy.deepcopy(self.reports[n])if n in self.reports else self.reader.json(n)
    def read_bytes(self,n):return self.reader.read_bytes(n)

class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.r=Reader(ROOT);cls.parent=cls.r.json(gate.PREFIX+'basis-01.json');cls.bindings=cls.r.json(gate.PREFIX+'bindings-01.json');cls.plan=cls.r.json('docs/native-rendering-closure.json');cls.proof=gate.analyze(cls.r,cls.parent,cls.bindings)
    @classmethod
    def tearDownClass(cls):cls.r.close()
    def bad_basis(self,change):
        p=copy.deepcopy(self.parent);change(p)
        with self.assertRaises(ValueError):gate.basis(self.plan,p,self.bindings)
    def bad_proof(self,change):
        p=copy.deepcopy(self.proof);change(p)
        with self.assertRaises(ValueError):gate.validate_verdicts(p)
    def player(self,p):return next(v for v in p['verdicts']if v['id']=='supported-player')
    def original(self,p):return next(v for v in p['verdicts']if v['id']=='original-recovery')
    def test_two_independent_issued_verdicts(self):self.assertEqual({v['id']for v in self.proof['verdicts']},set(gate.DECISIONS))
    def test_all_four_profile_verdicts_pending(self):self.assertTrue(all(v['state']=='pending'and not v['accepted']for d in self.proof['verdicts']for v in d['profiles']))
    def test_only_two_recording_gate_rows(self):
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),self.proof)
        self.assertEqual(sum(v['state']=='passed'for v in rows),self.parent['passed_requirements']+2)
        self.assertEqual([v for v in rows if v['id']!=gate.GATE],[v for v in self.parent['requirements']if v['id']!=gate.GATE])
    def test_original_scoped_milestones_preserved(self):
        self.assertEqual([g['registered_behaviors']for g in self.original(self.proof)['contract_groups']],[g['registered_behaviors']for g in self.bindings['recovery_groups']])
    def test_historical_failure_not_current_regression(self):self.assertTrue(all(not v['current_failure_asserted']for v in self.player(self.proof)['historical_failures']))
    def test_no_new_execution(self):self.assertEqual([self.proof[n]for n in ['original_game_runs','new_native_executions','new_pixel_comparisons']],[0,0,0])
    def test_missing_row(self):self.bad_basis(lambda p:p['requirements'].pop())
    def test_duplicate_row(self):self.bad_basis(lambda p:p['requirements'].__setitem__(1,p['requirements'][0]))
    def test_changed_counts(self):self.bad_basis(lambda p:p.update(passed_requirements=156))
    def test_changed_gate_condition(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(pending_reason='accept from FPS'))
    def test_changed_gate_owner(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(owner_task='other'))
    def test_accepted_leaf_lost(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['kind']=='journey').update(admitted_execution_evidence=[]))
    def test_accepted_leaf_made_pending(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['kind']=='journey').update(state='pending'))
    def test_positive_parent_verdict(self):self.bad_basis(lambda p:p['decisions'].update({'supported-player':'passed'}))
    def test_whole_player_flag(self):self.bad_basis(lambda p:p.update(current_whole_player_proved=True))
    def test_combined_player_flag(self):self.bad_basis(lambda p:p.update(current_combined_player_proved=True))
    def test_failure_retirement_flag(self):self.bad_basis(lambda p:p.update(historical_failures_retired=True))
    def test_scope_amendment_lost(self):self.bad_basis(lambda p:p['scope_amendments'].clear())
    def test_gate_already_passed(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(state='passed'))
    def test_boundary_passed_without_receipt(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.BOUNDARY).update(state='passed'))
    def test_missing_verdict(self):self.bad_proof(lambda p:p['verdicts'].pop())
    def test_duplicate_verdict(self):self.bad_proof(lambda p:p['verdicts'].__setitem__(1,copy.deepcopy(p['verdicts'][0])))
    def test_unissued_verdict(self):self.bad_proof(lambda p:self.player(p).update(issued=False))
    def test_player_acceptance_injected(self):self.bad_proof(lambda p:self.player(p).update(state='passed',accepted=True))
    def test_recovery_acceptance_injected(self):self.bad_proof(lambda p:self.original(p).update(state='passed',accepted=True))
    def test_player_profile_accepted(self):self.bad_proof(lambda p:self.player(p)['profiles'][0].update(accepted=True))
    def test_missing_software_verdict(self):self.bad_proof(lambda p:self.player(p)['profiles'].pop())
    def test_duplicate_profile(self):self.bad_proof(lambda p:self.original(p)['profiles'].__setitem__(1,copy.deepcopy(self.original(p)['profiles'][0])))
    def test_missing_reason(self):self.bad_proof(lambda p:self.player(p).update(reason=''))
    def test_missing_exit(self):self.bad_proof(lambda p:self.original(p).update(exit_conditions=[]))
    def test_whole_player_blocker_removed(self):self.bad_proof(lambda p:self.player(p)['profiles'][0].update(blockers=[v for v in self.player(p)['profiles'][0]['blockers']if v['id']!='final-current-player-receipt']))
    def test_required_boundary_blocker_removed(self):self.bad_proof(lambda p:self.player(p)['profiles'][0].update(blockers=[v for v in self.player(p)['profiles'][0]['blockers']if v['id']!=gate.BOUNDARY]))
    def test_fps_substitutes_contract(self):self.bad_proof(lambda p:self.player(p)['profiles'][0]['response_progress_resources'].update(fixed_fps_floor_required=True))
    def test_missing_response_evidence(self):self.bad_proof(lambda p:self.player(p)['profiles'][0]['response_progress_resources'].update(response_evidence=[]))
    def test_missing_progress_evidence(self):self.bad_proof(lambda p:self.player(p)['profiles'][0]['response_progress_resources'].update(progress_evidence=[]))
    def test_missing_resource_evidence(self):self.bad_proof(lambda p:self.player(p)['profiles'][0]['response_progress_resources'].update(resource_evidence=[]))
    def test_claim_budget_promoted(self):self.bad_proof(lambda p:self.player(p)['profiles'][0]['response_progress_resources'].update(whole_claim_response_progress_resource_validation_established=True))
    def test_recovery_group_removed(self):self.bad_proof(lambda p:self.original(p)['contract_groups'].pop())
    def test_duplicate_recovery_group(self):self.bad_proof(lambda p:self.original(p)['contract_groups'].__setitem__(1,copy.deepcopy(self.original(p)['contract_groups'][0])))
    def test_original_group_promoted(self):self.bad_proof(lambda p:self.original(p)['contract_groups'][0].update(whole_group_completion_established=True))
    def test_registered_milestones_removed(self):self.bad_proof(lambda p:self.original(p)['contract_groups'][0].update(registered_behaviors=[]))
    def test_negative_observation_removed(self):self.bad_proof(lambda p:self.player(p).update(historical_failures=[]))
    def test_historical_failure_made_current(self):self.bad_proof(lambda p:self.player(p)['historical_failures'][0].update(current_failure_asserted=True))
    def test_global_player_acceptance(self):self.bad_proof(lambda p:p.update(supported_player_accepted=True))
    def test_global_recovery_completion(self):self.bad_proof(lambda p:p.update(original_recovery_complete=True))
    def test_global_whole_player_claim(self):self.bad_proof(lambda p:p.update(current_whole_player_proved=True))
    def test_controlled_overlap_remains_bounded(self):
        aliases=[v for v in self.parent['requirements']if v['id']=='operation.copy-alias-overlap']
        self.assertEqual(len(aliases),2)
        self.assertTrue(all(v['actual_player_overlap_domains']==0 and v['natural_player_reachability']=='pending'and v['retained_obligation']['state']=='pending'for v in aliases))
    def test_controlled_overlap_cannot_become_natural(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']=='operation.copy-alias-overlap').update(natural_player_reachability='passed'))
    def test_changed_input_hash(self):
        i=gate.Inputs(self.r);ref=next(iter(self.bindings['supports'].values()))
        with self.assertRaises(ValueError):i.checked(dict(ref,sha256='0'*64))
    def test_changed_registered_recovery_scope(self):
        reg=self.r.json('research/runtime/coverage/register.json');bid=self.bindings['recovery_groups'][0]['registered_behaviors'][0]['id'];next(v for v in reg['behaviors']if v['id']==bid)['scope']='broadened scope'
        r=Proxy(self.r,{'research/runtime/coverage/register.json':reg})
        with self.assertRaises(ValueError):gate.recovery_review(r,self.plan,self.bindings)
    def test_changed_historical_failure(self):
        ref=self.bindings['historical_negative_progress'];q=self.r.json(ref);q['negative_executions'][0]['resolution']='silently-retired';r=Proxy(self.r,{ref:q})
        with self.assertRaises(ValueError):gate.historical_failures(gate.Inputs(r),self.bindings)

if __name__=='__main__':unittest.main()
