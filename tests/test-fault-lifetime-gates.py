#!/usr/bin/env python3
"""Adversarial controls using the eight retained fault/fresh executions."""
import copy
import hashlib
import json
from pathlib import Path
import sqlite3
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
import fault_lifetime_gate as gate


class Proxy:
    def __init__(self, reader, reports=None, payloads=None):
        self.reader, self.reports, self.payloads = reader, reports or {}, payloads or {}
    def digest(self, n): return self.reader.digest(n)
    def json(self, n): return copy.deepcopy(self.reports[n]) if n in self.reports else self.reader.json(n)
    def read_bytes(self, n): return self.payloads[n] if n in self.payloads else self.reader.read_bytes(n)


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.parent = cls.reader.json(gate.PREFIX+'basis-01.json')
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        cls.pair = next(r for r in cls.parent['requirements'] if r['id']=='operation.transition-recovery'
                        and r['configuration']=='player-default')['admitted_execution_evidence'][0]
        cls.a = cls.reader.json(cls.pair['acquisition']['record'])
        cls.c = cls.reader.json(cls.pair['comparison']['record'])

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def bad_parent(self, change):
        p=copy.deepcopy(self.parent);change(p)
        with self.assertRaises(ValueError): gate.basis(self.plan,p)

    def bad_case(self, change):
        a,c=copy.deepcopy(self.a),copy.deepcopy(self.c);change(a,c)
        reader=Proxy(self.reader,{self.pair['acquisition']['record']:a,self.pair['comparison']['record']:c})
        with self.assertRaises(ValueError): gate.Join(reader).case(self.pair,'player-default','operation.transition-recovery')

    def bad_pixels(self, change):
        c=copy.deepcopy(self.c);change(c)
        with self.assertRaises(ValueError): gate.pixels(c)

    def test_01_exact_eight_case_cohort(self):
        self.assertEqual(len(gate.analyze(self.reader,self.parent)['cases']),8)

    def test_02_exact_two_cell_admission(self):
        proof=gate.analyze(self.reader,self.parent)
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),proof)
        self.assertEqual(sum(r['state']=='passed' for r in rows),self.parent['passed_requirements']+2)
        self.assertEqual([r for r in rows if r['id']!=gate.GATE],[r for r in self.parent['requirements'] if r['id']!=gate.GATE])

    def test_03_missing_prerequisite(self):
        def change(p):
            r=next(r for r in p['requirements'] if r['id']=='operation.surface-loss-restore');r['state']='pending'
            p['passed_requirements']-=1;p['pending_requirements']+=1
        self.bad_parent(change)

    def test_04_missing_leaf_receipt(self):
        self.bad_parent(lambda p:next(r for r in p['requirements'] if r['id']=='operation.failure-cleanup').update(admitted_execution_evidence=[]))

    def test_05_duplicate_requirement(self): self.bad_parent(lambda p:p['requirements'].__setitem__(1,copy.deepcopy(p['requirements'][0])))
    def test_06_false_parent_count(self): self.bad_parent(lambda p:p.update(passed_requirements=156))
    def test_07_changed_condition(self): self.bad_parent(lambda p:next(r for r in p['requirements'] if r['id']==gate.GATE).update(pending_reason='assume success'))
    def test_08_final_decision_promotion(self): self.bad_parent(lambda p:p['decisions'].update({'supported-player':'passed'}))
    def test_09_wrong_receipt_hash(self):
        with self.assertRaises(ValueError):gate.Join(self.reader).checked(dict(self.pair['acquisition'],sha256='0'*64))
    def test_10_wrong_comparison_primary(self): self.bad_case(lambda a,c:c['acquisition'].update(sha256='0'*64))
    def test_11_mixed_profile(self): self.bad_case(lambda a,c:a['configuration'].update(software_flag=True))
    def test_12_wrong_extent(self): self.bad_case(lambda a,c:a['configuration'].update(actual_size=[1,1]))
    def test_13_compiled_driver(self): self.bad_case(lambda a,c:a['configuration'].update(compiled_test_driver_invoked=True))
    def test_14_wrong_fault_class(self): self.bad_case(lambda a,c:a['configuration'].update(declared_fault='reset'))
    def test_15_surviving_prefix(self): self.bad_case(lambda a,c:a.update(private_wine_stopped=False))
    def test_16_forced_stop(self): self.bad_case(lambda a,c:a.update(forced_private_prefix_stop=True))
    def test_17_missing_original_manifest(self): self.bad_case(lambda a,c:a.update(original_manifest_verified_before_after=False))
    def test_18_dependent_native_input(self): self.bad_case(lambda a,c:a.update(original_pixels_used_as_native_inputs=True))
    def test_19_normal_credit_for_failure(self): self.bad_case(lambda a,c:a['proof'].update(failed_epoch_normal_credit=True))
    def test_20_wrong_epoch_root(self): self.bad_case(lambda a,c:c['epochs'][0].update(root='another'))
    def test_21_dependent_pixel_oracle(self): self.bad_pixels(lambda c:c['epochs'][0]['original_reference'].update(native_player_pixels_read=True))
    def test_22_hidden_pixel_difference(self): self.bad_pixels(lambda c:c['epochs'][0]['comparison']['comparisons'][0].update(actual_sha256='0'*64))
    def test_23_synthetic_end(self): self.bad_pixels(lambda c:c['epochs'][0]['comparison'].update(synthetic_end_added=True))
    def test_24_failed_normal_completion(self): self.bad_pixels(lambda c:c['epochs'][0]['comparison'].update(normal_completion_claimed=True))
    def test_25_missing_swap(self): self.bad_pixels(lambda c:c['epochs'][0]['comparison']['comparisons'].pop())
    def test_26_duplicate_swap_partition(self): self.bad_pixels(lambda c:c['partition'].update(all_swaps_uniquely_bound=False))
    def test_27_fresh_surface_leak(self): self.bad_pixels(lambda c:c['epochs'][1]['comparison'].update(remaining_surfaces=1))
    def test_28_replayed_source_generation(self): self.bad_case(lambda a,c:c['epochs'][1]['world_domain']['worlds'][1].update(generation=1))
    def test_29_reused_nonce(self): self.bad_case(lambda a,c:a['proof']['epochs'][1]['resident'].update(nonce=a['proof']['epochs'][0]['resident']['nonce']))

    def raw_change(self, suffix, modify, expected):
        a=copy.deepcopy(self.a);entry=next(v for v in a['artifacts'] if v['path'].endswith('/'+suffix))
        raw=modify(self.reader.read_bytes(entry['record']),a)
        entry.update(bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
        reader=Proxy(self.reader,{self.pair['acquisition']['record']:a},{entry['record']:raw})
        with self.assertRaisesRegex(ValueError,expected):gate.Join(reader).case(self.pair,'player-default','operation.transition-recovery')

    def test_30_exact_terminal_owner_retirement(self):
        def modify(raw,a):
            v=json.loads(raw);v['state']['timers_stopped']=False;a['proof']['epochs'][0]['terminal']=v
            return json.dumps(v).encode()
        self.raw_change('native-terminal-state.json',modify,'Incomplete exact terminal retirement')

    def test_31_exact_expected_error(self):
        def modify(raw,a):
            v=json.loads(raw);v['error']='unrelated failure';a['proof']['epochs'][0]['terminal']=v
            return json.dumps(v).encode()
        self.raw_change('native-terminal-state.json',modify,'Wrong expected producer error')

    def test_32_stable_terminal_output(self):
        def modify(raw,a):
            db=sqlite3.connect(':memory:');db.deserialize(raw)
            db.execute("UPDATE terminal SET viewport='{}' WHERE ordinal=2");db.commit();raw=db.serialize();db.close();return raw
        self.raw_change('terminal-samples.sqlite3',modify,'Terminal World or input advanced')


if __name__=='__main__':unittest.main()
