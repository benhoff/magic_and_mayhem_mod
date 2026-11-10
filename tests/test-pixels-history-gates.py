#!/usr/bin/env python3
"""Adversarial joins against retained observations, without new scene fixtures."""
import copy
import json
import struct
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
from retained_artifacts import read_artifact
import pixels_history_gate as gate


class Reports:
    def __init__(self,reader,values):self.reader,self.values=reader,values
    def digest(self,path):return self.reader.digest(path)
    def json(self,path):return copy.deepcopy(self.values[path]) if path in self.values else self.reader.json(path)


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.parent = cls.reader.json(gate.PREFIX+'basis-01.json')
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        cls.pair = gate.attachments(next(v for v in cls.parent['requirements'] if v['id']=='operation.destination-dependency' and v['configuration']=='player-default'))[0]
        cls.a = cls.reader.json(cls.pair['acquisition']['record']); cls.c = cls.reader.json(cls.pair['comparison']['record'])
        item = next(v for v in cls.a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('viewport') and v['path'].endswith('.bin'))
        cls.capture = read_artifact(cls.reader, cls.a, item['path'])
        cls.observed = gate.viewport_index(cls.capture, cls.a['session_identity'])

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def bad_parent(self, change):
        p=copy.deepcopy(self.parent); change(p)
        with self.assertRaises(ValueError): gate.basis(self.plan,p)

    def bad_pixels(self, change):
        c=copy.deepcopy(self.c); change(c)
        with self.assertRaises(ValueError): gate.pixel_rows(c,self.observed)

    def bad_capture(self, change):
        raw=self.capture;at=8;parts=[raw[:8]];changed=False
        while at<len(raw):
            m,n=struct.unpack_from('<II',raw,at);at+=8
            row=json.loads(raw[at:at+m]);at+=m;body=raw[at:at+n];at+=n
            if not changed and row['kind']=='paint' and int(row['lease']):change(row);changed=True
            encoded=json.dumps(row,separators=(',',':')).encode()
            parts.extend((struct.pack('<II',len(encoded),n),encoded,body))
        self.assertTrue(changed)
        with self.assertRaises(ValueError):gate.viewport_index(b''.join(parts),self.a['session_identity'])

    def test_01_parent(self): self.assertEqual(len(gate.basis(self.plan,self.parent)),156)
    def test_02_complete_native_swaps(self): self.assertGreater(gate.pixel_rows(self.c,self.observed)['world_swaps'],0)
    def test_03_false_count(self): self.bad_parent(lambda p:p.update(passed_requirements=156))
    def test_04_duplicate_row(self): self.bad_parent(lambda p:p['requirements'].__setitem__(0,p['requirements'][1]))
    def test_05_missing_primary(self): self.bad_parent(lambda p:next(v for v in p['requirements'] if v['id']=='operation.cpu-write-readback').update(admitted_execution_evidence=[]))
    def test_06_changed_condition(self): self.bad_parent(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(pending_reason='assume pixels'))
    def test_07_final_decision(self): self.bad_parent(lambda p:p['decisions'].update({'supported-player':'passed'}))
    def test_08_dependent_native_oracle(self): self.bad_pixels(lambda c:c['original_reference'].update(native_player_pixels_read=True))
    def test_09_original_destination_seed(self): self.bad_pixels(lambda c:c['original_reference'].update(original_destination_capture_read=True))
    def test_10_original_input(self): self.bad_pixels(lambda c:c.update(original_pixels_used_as_native_inputs=True))
    def test_11_pixel_difference(self): self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(actual_sha256='0'*64))
    def test_12_missing_swap(self): self.bad_pixels(lambda c:c['comparison']['comparisons'].pop())
    def test_13_duplicate_swap(self): self.bad_pixels(lambda c:c['comparison']['comparisons'].append(c['comparison']['comparisons'][0]))
    def test_14_wrong_lease(self): self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(lease=999999))
    def test_15_wrong_generation(self): self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(world=999999))
    def test_16_partial_viewport(self): self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(pixels=1))
    def test_17_false_pixel_total(self): self.bad_pixels(lambda c:c['comparison'].update(pixels_compared=1))
    def test_18_false_world_total(self): self.bad_pixels(lambda c:c['observations'].update(world_swaps=0))
    def test_19_false_swap_total(self): self.bad_pixels(lambda c:c['observations'].update(swaps=1))
    def test_20_surviving_surfaces(self): self.bad_pixels(lambda c:c['comparison'].update(remaining_surfaces=1))
    def test_21_foreign_capture(self):
        with self.assertRaises(ValueError): gate.viewport_index(b'MNMACE01','foreign')
    def test_22_wrong_reference_hash(self):
        with self.assertRaises(ValueError):gate.Join(self.reader).checked(dict(self.pair['comparison'],sha256='0'*64))
    def test_23_duplicate_attachments(self):
        row=copy.deepcopy(next(v for v in self.parent['requirements'] if v['id']=='operation.destination-dependency'))
        row['admitted_execution_evidence'].append(row['admitted_execution_evidence'][0])
        with self.assertRaises(ValueError):gate.attachments(row)
    def test_24_incomplete_caller_state(self):
        cases=copy.deepcopy(self.reader.json(gate.SUPPORT['state'])['cases']);cases[0]['state_cases']=0
        with self.assertRaises(ValueError):gate.module('close-rendering-caller-state').validate_cases(cases)
    def test_25_missing_history_words(self):
        cpu=self.reader.json(gate.SUPPORT['cpu']);p=self.reader.json(cpu['analysis']['record']);c=copy.deepcopy(p['cases'][0]);c['witnesses'][0]['equal_nonzero_history_words']=0
        with self.assertRaises(ValueError):gate.module('close-rendering-operations').check_case(c)
    def test_26_exact_gate_join_and_preservation(self):
        proof=gate.analyze(self.reader,self.parent)
        self.assertEqual(len(proof['cases']),12)
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),proof)
        self.assertEqual(sum(v['state']=='passed'for v in rows),self.parent['passed_requirements']+2)
        self.assertEqual([v for v in rows if v['id']!=gate.GATE],[v for v in self.parent['requirements']if v['id']!=gate.GATE])
    def test_27_gpu_read_incomplete(self):self.bad_capture(lambda r:r.update(gpu_read_complete=False))
    def test_28_queued_only_frame(self):self.bad_capture(lambda r:r.update(native_gpu_frame=False))
    def test_29_pixels_not_captured(self):self.bad_capture(lambda r:r.update(pixels_captured=False))
    def test_30_unknown_frame_owner(self):self.bad_capture(lambda r:r.update(presentation_owner='unknown'))
    def test_31_lost_paint_history(self):self.bad_capture(lambda r:r.update(paint=str(int(r['paint'])+1)))
    def test_32_wrong_full_extent(self):self.bad_capture(lambda r:r.update(width=1))
    def test_33_unowned_world(self):self.bad_capture(lambda r:r.update(lease='0',world='1'))
    def test_34_failed_ancestor_not_whole_success(self):
        path='research/runtime/rendering-interaction-results-20261107/default-800-06-comparison.json'
        ref=dict(record=path,sha256=self.reader.digest(path))
        with self.assertRaises(ValueError):gate.Join(self.reader).checked(ref)
        self.assertIs(gate.Join(self.reader).checked(ref,pixels_only=True)['success'],False)
    def test_35_changed_pixel_ancestry_partition(self):
        c=self.reader.json('research/runtime/rendering-interaction-results-20261107/default-800-06-comparison-05.json')
        c['comparison']['comparisons'][0]['lease']+=1
        with self.assertRaises(ValueError):gate.Join(self.reader).pixel_origin(c)
    def test_36_changed_native_ancestry(self):
        c=self.reader.json('research/runtime/rendering-interaction-results-20261107/default-800-06-comparison-05.json')
        path=c['retained_pixel_comparison']['record'];old=self.reader.json(path)
        n=next(n for n in old['sources']if n.startswith(('renderer/','runtime/'))and n in c['sources'])
        old['sources'][n]='0'*64
        with self.assertRaises(ValueError):gate.Join(Reports(self.reader,{path:old})).pixel_origin(c)
    def test_37_successful_pixel_ancestry_keeps_failure(self):
        c=self.reader.json('research/runtime/rendering-interaction-results-20261107/default-800-06-comparison-05.json')
        join=gate.Join(self.reader);origin=join.pixel_origin(c)
        self.assertIs(origin['success'],False)
        self.assertTrue(any(v['whole_report_success'] is False and v['consumed_scope']=='independent-pixel-component-only' for v in join.refs.values()))
    def test_38_protected_ui_pixel_phase_keeps_raw_instability(self):
        c=self.reader.json('research/runtime/rendering-ui-results-20261107/default800-08-comparison-05.json')
        join=gate.Join(self.reader);origin=join.pixel_origin(c)
        self.assertIs(origin['sources_stable'],False)
        self.assertTrue(any(v['recorded_sources_stable'] is False and v['protected_pixel_sources_review'] is True for v in join.phases.values()))
    def test_39_ui_pixel_phase_needs_exact_review(self):
        c=self.reader.json('research/runtime/rendering-ui-results-20261107/default800-08-comparison-05.json')
        c['retained_pixel_phase']['protected_pixel_sources_exact']=False
        with self.assertRaises(ValueError):gate.Join(self.reader).pixel_origin(c)
    def test_40_all_primary_schema_profiles(self):
        pairs,singles,mechanisms=gate.primary_cohort(gate.Join(self.reader),self.parent['requirements'])
        self.assertEqual((len(pairs),len(singles),len(mechanisms)),(72,14,2))


if __name__=='__main__': unittest.main()
