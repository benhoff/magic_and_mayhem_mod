#!/usr/bin/env python3
"""Mutation controls over actual retained journey receipts and terminal bytes."""
import copy
import json
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
import journeys_gate as g


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT,memoize=True)
        cls.parent = cls.reader.json(g.PREFIX+'basis-01.json')
        cls.bindings = cls.reader.json(g.PREFIX+'bindings-01.json')
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        cls.admission,cls.pixels = g.accepted_pixels(cls.reader)
        cls.case = cls.pixels['cases'][0]; cls.primary = cls.case['pair']
        cls.ref = cls.primary['acquisition']; cls.profile = cls.case['configuration']
        cls.output = g.completed_output(cls.pixels,cls.primary,cls.profile)
        cls.a,_ = g.scoped_artifacts(cls.reader.json(cls.ref['record']),label='journey controls')
        names = ('native-only-session.json','native-output-admission.json','original-process-result.json',
                 'render-input.bin','render-commands.bin','render-commands.bin.control',
                 'native-cursor-lifetime.bin','native-draw-retirement.bin','events.bin',
                 'world-continuous.bin','world-terminal-0001.bin')
        cls.raw = {n:g.artifact(cls.reader,cls.a,n) for n in names}

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def bad_basis(self,change,binding_change=None):
        p = copy.deepcopy(self.parent); b = copy.deepcopy(self.bindings); change(p)
        if binding_change: binding_change(b)
        with self.assertRaises(ValueError): g.basis(self.plan,p,b)

    def bad_output(self,change):
        p = copy.deepcopy(self.pixels); change(p)
        with self.assertRaises(ValueError): g.completed_output(p,self.primary,self.profile)

    def bad_terminal(self,name,change):
        raw = dict(self.raw); raw[name] = change(raw[name])
        with patch.object(g,'artifact',side_effect=lambda reader,a,n:raw[n]):
            with self.assertRaises(ValueError): g.lifetime(self.reader,self.ref,self.profile,self.output)

    @staticmethod
    def word(raw,at,value):
        body = bytearray(raw); struct.pack_into('<I',body,at,value); return bytes(body)

    def test_01_exact_94_semantic_bindings(self):
        self.assertEqual(len(g.basis(self.plan,self.parent,self.bindings)),94)
    def test_02_missing_leaf(self): self.bad_basis(lambda p:p['requirements'].pop(0))
    def test_03_duplicate_leaf(self): self.bad_basis(lambda p:p['requirements'].__setitem__(1,p['requirements'][0]))
    def test_04_changed_owner(self): self.bad_basis(lambda p:p['requirements'][0].update(owner_task='EP11'))
    def test_05_changed_condition(self): self.bad_basis(lambda p:p['requirements'][0].update(pending_reason='waived'))
    def test_06_false_count(self): self.bad_basis(lambda p:p.update(passed_requirements=156))
    def test_07_final_decision(self): self.bad_basis(lambda p:p['decisions'].update(**{'supported-player':'passed'}))
    def test_08_missing_binding(self): self.bad_basis(lambda p:None,lambda b:b['rows'].pop())
    def test_09_duplicate_binding(self): self.bad_basis(lambda p:None,lambda b:b['rows'].__setitem__(1,b['rows'][0]))
    def test_10_pending_leaf(self): self.bad_basis(lambda p:p['requirements'][0].update(state='pending'))
    def test_11_changed_primary(self): self.bad_basis(lambda p:p['requirements'][0]['admitted_execution_evidence'].pop())
    def test_12_changed_scope(self): self.bad_basis(lambda p:p.update(scope_amendments=[]))
    def test_13_complete_output(self): self.assertEqual(self.output['case'],self.case)
    def test_14_wrong_profile(self):
        with self.assertRaises(ValueError): g.completed_output(self.pixels,self.primary,g.PROFILES[1])
    def test_15_missing_paint_history(self): self.bad_output(lambda p:p.update(cases=[],additional_presented_cases=[]))
    def test_16_missing_import_history(self): self.bad_output(lambda p:p['cases'][0]['command_history'].update(imports=0))
    def test_17_no_swaps(self): self.bad_output(lambda p:p['cases'][0]['completed_output'].update(swaps=0))
    def test_18_no_pixels(self): self.bad_output(lambda p:p['cases'][0]['completed_output'].update(pixels=0))
    def test_19_semantic_widgets_have_distinct_role(self):
        v = next(v for v in self.pixels['semantic_primary_cases'] if v['kind'] == 'public_results_return_acquisition')
        out = g.completed_output(self.pixels,v['execution'],v['configuration'])
        self.assertEqual(out['role'],'semantic-native-widget-policy')
        self.assertIs(out['case']['whole_viewport_pixel_credit'],False)
    def test_20_invented_widget_pixels(self):
        p = copy.deepcopy(self.pixels);v = p['semantic_primary_cases'][0];v['whole_viewport_pixel_credit'] = True
        with self.assertRaises(ValueError): g.completed_output(p,v['execution'],v['configuration'])
    def test_21_movie_mismatch(self):
        p = copy.deepcopy(self.pixels);v = p['movie_cases'][0];v['output']['mismatching_pixels'] = 1
        with self.assertRaises(ValueError): g.completed_output(p,v['execution'],g.PROFILES[0])
    def test_22_direct_attachment_projection(self):
        row = next(v for v in self.parent['requirements'] if v['id'] == 'journey.cursor-interaction.cursor-motion')
        values = g.expand(g.Join(self.reader),row)
        self.assertEqual(set(values[0]),{'acquisition','comparison'})
    def test_23_duplicate_attachment(self):
        row = copy.deepcopy(self.parent['requirements'][0]);row['admitted_execution_evidence'] *= 2
        with self.assertRaises(ValueError): g.expand(g.Join(self.reader),row)
    def test_24_normal_terminal(self):
        with patch.object(g,'artifact',side_effect=lambda reader,a,n:self.raw[n]):
            p = g.lifetime(self.reader,self.ref,self.profile,self.output)
        self.assertTrue(p['proof']['normal_quit']);self.assertEqual(p['proof']['graphics_owners'],0)
    def test_25_active_input(self): self.bad_terminal('render-input.bin',lambda v:self.word(v,20,1))
    def test_26_unended_commands(self): self.bad_terminal('render-commands.bin',lambda v:self.word(v,24,0))
    def test_27_pending_stop(self): self.bad_terminal('render-commands.bin.control',lambda v:self.word(v,32,0))
    def test_28_graphics_owner(self): self.bad_terminal('native-draw-retirement.bin',lambda v:self.word(v,20,1))
    def test_29_original_process_exit(self):
        self.bad_terminal('original-process-result.json',lambda v:json.dumps(dict(json.loads(v),original_exit_code=1)).encode())
    def test_30_forced_stop(self):
        self.bad_terminal('native-only-session.json',lambda v:json.dumps(dict(json.loads(v),force_stop_verified=True)).encode())
    def test_31_fallback(self):
        self.bad_terminal('native-only-session.json',lambda v:json.dumps(dict(json.loads(v),fallback=True)).encode())
    def test_32_cursor_work(self):
        self.bad_terminal('native-cursor-lifetime.bin',lambda v:self.word(v,len(v)-64+16,1))
    def test_33_foreign_callback_thread(self):
        old = g.module('close-rendering-retained-rows')
        rows = old.menu_events.decode(self.raw['events.bin']);i = next(i for i,v in enumerate(rows) if v['event'] == 3)
        self.bad_terminal('events.bin',lambda v:self.word(v,16+64*i+8,0))
    def test_34_wrong_retirement_profile(self):
        with self.assertRaises(ValueError): g.lifetime(self.reader,self.ref,g.PROFILES[1],self.output)
    def test_35_complete_gate_preserves_154_rows(self):
        links = [dict(configuration=v['row']['configuration'],requirement=v['row']['id'],journey=v['row']['journey']) for v in self.bindings['rows']]
        proof = dict(success=True,journey_cells=94,journeys=14,linked_requirements=links)
        rows = g.close_rows(self.parent,dict(record='controlled-projection',sha256='a'*64),proof)
        self.assertEqual([v for v in rows if v['id'] != g.GATE],[v for v in self.parent['requirements'] if v['id'] != g.GATE])
        self.assertEqual(sum(v['state'] == 'passed' for v in rows),150)
    def test_36_missing_gate_leaf(self):
        proof = dict(success=True,journey_cells=94,journeys=14,linked_requirements=[])
        with self.assertRaises(ValueError): g.close_rows(self.parent,{},proof)


if __name__ == '__main__': unittest.main()
