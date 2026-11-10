#!/usr/bin/env python3
"""Actual-data controls for the finite 21-operation conjunction."""
import copy
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
import operations_gate as gate

class Proxy:
    def __init__(self,reader,reports):self.reader,self.reports=reader,reports
    def digest(self,name):return self.reader.digest(name)
    def json(self,name):return copy.deepcopy(self.reports[name])if name in self.reports else self.reader.json(name)
    def read_bytes(self,name):return self.reader.read_bytes(name)

class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader=Reader(ROOT);cls.parent=cls.reader.json(gate.PREFIX+'basis-01.json')
        cls.bindings=cls.reader.json(gate.PREFIX+'bindings-01.json');cls.plan=cls.reader.json('docs/native-rendering-closure.json')
        cls.row=next(v for v in cls.parent['requirements']if v['configuration']=='player-default'and v['id']=='operation.copy-opaque')
        cls.pair=gate.pair_of(cls.row['admitted_execution_evidence'][0]);cls.size=cls.row['admitted_execution_evidence'][0]['actual_world_extents'][0]
        cls.a=cls.reader.json(cls.pair['acquisition']['record']);cls.c=cls.reader.json(cls.pair['comparison']['record'])
        cls.proof=gate.analyze(cls.reader,cls.parent,cls.bindings)
    @classmethod
    def tearDownClass(cls):cls.reader.close()
    def bad_basis(self,change):
        p=copy.deepcopy(self.parent);change(p)
        with self.assertRaises(ValueError):gate.basis(self.plan,p,self.bindings)
    def bad_pixels(self,change):
        c=copy.deepcopy(self.c);change(c)
        with self.assertRaises(ValueError):gate.comparison_pixels(c)
    def bad_pair(self,change):
        a,c=copy.deepcopy(self.a),copy.deepcopy(self.c);change(a,c)
        r=Proxy(self.reader,{self.pair['acquisition']['record']:a,self.pair['comparison']['record']:c})
        with self.assertRaises(ValueError):gate.Join(r).normal_pair(self.pair,'player-default',self.size,'operation.copy-opaque')
    def case(self,leaf):
        q=self.reader.json('research/runtime/'+gate.SUPPORT[leaf])
        return copy.deepcopy(q.get('partial_cases'if leaf=='partial-write'else'cases',q.get('executions'))[0])
    def bad_witness(self,leaf,change):
        c=self.case(leaf);change(c)
        with self.assertRaises(ValueError):gate.witnesses('operation.'+leaf,c)
    def test_01_all_operation_cells(self):
        self.assertEqual(self.proof['operation_cells'],42)
        self.assertEqual(len(self.proof['linked_requirements']),42)
        self.assertEqual(self.proof['original_game_runs'],0)
        self.assertEqual(self.proof['new_native_executions'],0)
        self.assertTrue(all(not v['current_native_execution_claimed']for v in self.proof['retained_execution_source_phases'].values()))
    def test_02_exact_two_gate_cells(self):
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),self.proof)
        self.assertEqual(sum(v['state']=='passed'for v in rows),self.parent['passed_requirements']+2)
        self.assertEqual([v for v in rows if v['id']!=gate.GATE],[v for v in self.parent['requirements']if v['id']!=gate.GATE])
    def test_03_missing_operation(self):self.bad_basis(lambda p:p['requirements'].pop())
    def test_04_duplicate_operation(self):self.bad_basis(lambda p:p['requirements'].__setitem__(1,copy.deepcopy(p['requirements'][0])))
    def test_05_false_counts(self):self.bad_basis(lambda p:p.update(passed_requirements=156))
    def test_06_changed_condition(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(pending_reason='assume success'))
    def test_07_wrong_owner(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']=='operation.copy-keyed').update(owner_task='other'))
    def test_08_missing_receipt(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']=='operation.copy-keyed').update(admitted_execution_evidence=[]))
    def test_09_changed_leaf_state(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']=='operation.copy-keyed').update(state='pending'))
    def test_10_final_decision(self):self.bad_basis(lambda p:p['decisions'].update({'supported-player':'passed'}))
    def test_11_gate_already_passed(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(state='passed'))
    def test_12_wrong_evidence_hash(self):
        with self.assertRaises(ValueError):gate.Join(self.reader).checked(dict(self.pair['acquisition'],sha256='0'*64))
    def test_13_wrong_acquisition(self):self.bad_pair(lambda a,c:c['acquisition'].update(sha256='0'*64))
    def test_14_profile_mixed(self):self.bad_pair(lambda a,c:a['configuration'].update(software_flag=True))
    def test_15_unknown_extent(self):
        with self.assertRaises(ValueError):gate.Join(self.reader).normal_pair(self.pair,'player-default',[1,1],'operation.copy-opaque')
    def test_16_original_not_stopped(self):self.bad_pair(lambda a,c:a.update(private_wine_stopped=False))
    def test_17_forced_stop(self):self.bad_pair(lambda a,c:a.update(forced_private_prefix_stop=True))
    def test_18_missing_manifest(self):self.bad_pair(lambda a,c:a.update(original_manifest_verified_before_after=False))
    def test_19_original_pixel_input(self):self.bad_pair(lambda a,c:a.update(original_pixels_used_as_native_inputs=True))
    def test_20_compiled_driver(self):self.bad_pair(lambda a,c:a['configuration'].update(compiled_test_driver_invoked=True))
    def test_21_native_owner_leak(self):self.bad_pair(lambda a,c:a['retirement']['graphics_retirement']['counts'].update(test_survivor=1))
    def test_22_process_not_retired(self):self.bad_pair(lambda a,c:a['retirement']['graphics_retirement'].update(process_exit=False))
    def test_23_error_exit(self):self.bad_pair(lambda a,c:a.update(exit_code=1))
    def test_24_non_normal_quit(self):self.bad_pair(lambda a,c:a['resident'].update(normal_quit_retired=False))
    def test_25_unknown_comparison_kind(self):self.bad_pair(lambda a,c:c.update(kind='synthetic_comparison'))
    def test_26_bad_independent_oracle(self):self.bad_pixels(lambda c:c['original_reference'].update(native_player_pixels_read=True))
    def test_27_original_destination_input(self):self.bad_pixels(lambda c:c['original_reference'].update(original_destination_capture_read=True))
    def test_28_no_owned_initial_history(self):self.bad_pixels(lambda c:c['original_reference'].update(continuous_from_owned_initial_history=False))
    def test_29_hidden_pixel_difference(self):self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(actual_sha256='0'*64))
    def test_30_missing_comparison_row(self):self.bad_pixels(lambda c:c['comparison']['comparisons'].pop())
    def test_31_retained_surface(self):self.bad_pixels(lambda c:c['comparison'].update(remaining_surfaces=1))
    def test_32_incorrect_world_count(self):self.bad_pixels(lambda c:c['comparison'].update(world_presentations=0))
    def test_33_empty_pixels(self):self.bad_pixels(lambda c:c['comparison'].update(comparisons=[]))
    def test_34_wrong_world_extent(self):self.bad_pixels(lambda c:c['world_domain'].update(extents=[[1,1]]))
    def test_35_copy_without_changed_output(self):self.bad_witness('copy-opaque',lambda c:c['proof']['witnesses'][0]['copy'].update(changed_pixels=0))
    def test_36_keyed_without_output(self):self.bad_witness('copy-keyed',lambda c:c['keyed']['witnesses'][0]['copy'].update(changed_words=0))
    def test_37_destination_without_history(self):self.bad_witness('destination-dependency',lambda c:c['proof'].update(destination_history_verified=False))
    def test_38_direct_wrong_pixels(self):self.bad_witness('sprite-direct',lambda c:c['samples'][0].update(actual_rgba=[255,0,255,255]))
    def test_39_auxiliary_wrong_route(self):self.bad_witness('sprite-auxiliary',lambda c:next(v for v in c['witnesses']if v['kind']=='auxiliary:shadow')['source'].update(route='wrong'))
    def test_40_partial_exterior_modified(self):self.bad_witness('partial-write',lambda c:c['witnesses'][0].update(after_exterior_sha256='0'*64))
    def test_41_cpu_ticket_reused(self):self.bad_witness('cpu-write-readback',lambda c:c['reads'][1].update(ticket=c['reads'][0]['ticket']))
    def test_42_full_fill_no_effect(self):self.bad_witness('fill-full',lambda c:c['witnesses'][0].update(changed_words=0))
    def test_43_clipped_fill_wrong_plane(self):self.bad_witness('fill-clipped',lambda c:c['witness']['native_replay']['proof'].update(actual_sha256='0'*64))
    def test_44_stretch_no_gpu_plane(self):self.bad_witness('copy-stretched',lambda c:c.update(actual_gpu_planes=False))
    def test_45_caller_state_not_accounted(self):self.bad_witness('flags-and-return-values',lambda c:c.update(source_accounted=False))
    def test_46_font_registration_not_retired(self):self.bad_witness('text-font-dc-lifetime',lambda c:c['native_fonts'].update(normal_retirement=False))
    def test_47_palette_repeated_generation(self):self.bad_witness('palette-cycling',lambda c:c['native'][1].update(generation=c['native'][0]['generation']))
    def test_48_missing_controlled_overlap_approval(self):
        p=copy.deepcopy(self.plan);p['acceptance_scope_amendments'][0]['approval']['response']='no'
        with self.assertRaises(ValueError):gate.module('close-rendering-alias-bounded').policy(p)
    def test_49_overlap_obligation_preserved(self):
        rows=[v for v in self.proof['linked_requirements']if v['requirement']=='operation.copy-alias-overlap']
        self.assertEqual(len(rows),2)
        self.assertTrue(all(v['natural_player_reachability']=='pending'and v['retained_obligation']['state']=='pending'for v in rows))
    def test_50_false_proof_cell_count(self):
        p=copy.deepcopy(self.proof);p['operation_cells']=41
        with self.assertRaises(ValueError):gate.close_rows(self.parent,{},p)
    def test_51_missing_profile_operation(self):
        p=copy.deepcopy(self.proof);p['linked_requirements'].pop()
        with self.assertRaises(ValueError):gate.close_rows(self.parent,{},p)
    def test_52_duplicate_profile_operation(self):
        p=copy.deepcopy(self.proof);p['linked_requirements'][1]=copy.deepcopy(p['linked_requirements'][0])
        with self.assertRaises(ValueError):gate.close_rows(self.parent,{},p)

if __name__=='__main__':unittest.main()
