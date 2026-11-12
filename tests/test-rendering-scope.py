#!/usr/bin/env python3
"""Scope answers must not silently close engineering proof or original recovery."""
from collections import Counter
from copy import deepcopy
import json
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_claims import fingerprint
from rendering_scope import validate,load


class ScopeTests(unittest.TestCase):
    def setUp(self):
        self.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        rows=[dict(id=d['id'],blocking_condition=d['blocking_condition'],close_when=d['close_when'],
                   user_judgment_required_now=False,answer='Synthetic guard answer only',technical_remaining=['Proof remains'],
                   owner_task='EP07',question_state='answered',original_recovery_state='unresolved',
                   player_scope_state='resolved' if d['id']=='SC.physical-compatibility' else 'unresolved')
              for d in self.plan['completion_contract']['scope_decisions']]
        self.report=dict(kind='rendering_scope_decision_review',success=True,
            contract_sha256=fingerprint(self.plan['completion_contract']),
            gap_register_sha256=fingerprint(self.plan['required_gap_register']),
            execution_plan_sha256=fingerprint(self.plan['execution_plan']),decisions=rows,
            no_engine_status_promotion=True,supported_player_accepted=False,original_recovery_complete=False,status_promotions=[],
            facts=dict(qt_fullscreen_already_exposed=True,rgb555_surfaces_are_not_rgb555_world_transport=True,
                       natural_absence_is_not_global_unreachability=True),
            acceptance_followups=[dict(id=k) for k in ['qt-fullscreen','format-production','owned-history','screen-routes','natural-callers','alternate-chain']],
            counts=dict(scope_questions=8,immediate_user_decisions=0,player_scope_closed=1,player_scope_unresolved=7,
                        original_recovery_scope_unresolved=8,question_states={'answered':8}),sources={},records=[])

    def test_one_bounded_player_scope_closure_is_not_game_acceptance(self):
        original=deepcopy(self.plan)
        report=validate(self.plan,self.report)
        self.assertEqual(report['counts']['player_scope_closed'],1)
        self.assertEqual(self.plan,original)
        self.assertFalse(report['supported_player_accepted'])
        self.assertFalse(report['original_recovery_complete'])

    def test_missing_or_duplicate_answer_refuses(self):
        for duplicate in [False,True]:
            self.setUp()
            if duplicate:self.report['decisions'].append(deepcopy(self.report['decisions'][0]))
            else:self.report['decisions'].pop()
            with self.subTest(duplicate=duplicate),self.assertRaisesRegex(ValueError,'scope answer'):validate(self.plan,self.report)

    def test_all_three_frozen_input_bindings_are_required(self):
        for name in ['contract_sha256','gap_register_sha256','execution_plan_sha256']:
            self.setUp();self.report[name]='0'*64
            with self.subTest(name=name),self.assertRaisesRegex(ValueError,'input binding'):validate(self.plan,self.report)

    def test_original_blocking_condition_or_exit_cannot_change(self):
        for field in ['blocking_condition','close_when']:
            self.setUp();self.report['decisions'][0][field]='Ignore required paths'
            with self.subTest(field=field),self.assertRaisesRegex(ValueError,'conditional scope rule'):validate(self.plan,self.report)

    def test_required_engineering_unknown_cannot_be_marked_closed(self):
        self.report['decisions'][0]['player_scope_state']='resolved'
        with self.assertRaisesRegex(ValueError,'Unproved engineering'):validate(self.plan,self.report)

    def test_player_boundary_cannot_close_original_recovery(self):
        self.report['decisions'][-1]['original_recovery_state']='resolved'
        with self.assertRaisesRegex(ValueError,'original recovery'):validate(self.plan,self.report)

    def test_technical_question_cannot_become_an_immediate_user_blocker(self):
        self.report['decisions'][0]['user_judgment_required_now']=True
        with self.assertRaisesRegex(ValueError,'Incomplete scope answer'):validate(self.plan,self.report)

    def test_source_absence_or_format_conflation_cannot_erase_required_work(self):
        for field in self.report['facts']:
            self.setUp();self.report['facts'][field]=False
            with self.subTest(field=field),self.assertRaisesRegex(ValueError,'technical distinctions'):validate(self.plan,self.report)

    def test_followups_and_counters_cannot_silently_shrink(self):
        self.report['acceptance_followups'].pop()
        with self.assertRaisesRegex(ValueError,'followup'):validate(self.plan,self.report)
        self.setUp();self.report['counts']['player_scope_closed']=8
        with self.assertRaisesRegex(ValueError,'counters'):validate(self.plan,self.report)

    def test_static_review_cannot_accept_player_or_promote_engine(self):
        for field,value in [('supported_player_accepted',True),('original_recovery_complete',True),('status_promotions',['fake'])]:
            self.setUp();self.report[field]=value
            with self.subTest(field=field),self.assertRaisesRegex(ValueError,'promote engine'):validate(self.plan,self.report)

    def test_unsafe_result_path_is_rejected_before_any_read(self):
        self.plan['scope_review']['record']='original/Chaos.exe'
        with self.assertRaisesRegex(ValueError,'Unsafe scope result'):load(ROOT,None,self.plan)

    def test_reader_requires_registered_identity_and_current_sources(self):
        test=self
        class Reader:
            def json(self,name):
                if name=='research/runtime/coverage/register.json':
                    return dict(evidence=[dict(id=test.plan['scope_review']['evidence'],kind='static',
                         record=test.plan['scope_review']['record'],sha256='a'*64,
                         assertions=[dict(pointer='/success',equals=True)])])
                return test.report
            def digest(self,name):return 'a'*64
        value=load(ROOT,Reader(),self.plan)
        self.assertEqual(value['counts']['player_scope_unresolved'],7)
        self.report['sources']={'code.cpp':'b'*64}
        with self.assertRaisesRegex(ValueError,'source changed'):load(ROOT,Reader(),self.plan)


if __name__=='__main__':unittest.main()
