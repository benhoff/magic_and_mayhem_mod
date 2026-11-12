import copy
import importlib.util
import os
from pathlib import Path
import struct
import sys
import unittest
import numpy as np

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('fill_join',ROOT/'tools/close-rendering-clipped-fill.py')
j=importlib.util.module_from_spec(spec);spec.loader.exec_module(j);m=j.m

class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.analysis=r.json(os.environ.get('MNM_CLIPPED_FILL_ANALYSIS',m.OUT+'analysis-01.json'))
            assert cls.analysis['success']
            cls.c=cls.analysis['cases'][1];w=cls.c['witness'];refs=w['native_replay']['artifacts']
            cls.before=np.frombuffer(r.read_bytes(refs['native-prior-input']['record']),dtype='<u2').astype('uint32').reshape(600,800)
            cls.upload=r.read_bytes(refs['actual-upload']['record']);cls.actual=r.read_bytes(refs['actual-native-gpu-output']['record'])
            cls.expected,cls.mask=m.fill_output(cls.before,w['query'],w['query']['actual'],cls.upload)
            cls.parent=r.json('research/runtime/rendering-ui-results-20261107/integrated-acceptance-01.json')
            cls.plan=r.json('docs/native-rendering-closure.json')
    def bad(self,change):
        c=copy.deepcopy(self.c);change(c)
        with self.assertRaises(ValueError):m.check_case(c)
    def test_actual_four_domains(self):
        for c in self.analysis['cases']:m.check_case(c)
    def test_actual_complete_gpu_plane(self):m.checked_gpu_output(self.actual,self.expected,self.before,self.mask)
    def test_actual_upload_geometry_and_color(self):m.fill_output(self.before,self.c['witness']['query'],self.c['witness']['query']['actual'],self.upload)
    def test_upload_word_corruption(self):
        raw=bytearray(self.upload);raw[-1]^=1
        with self.assertRaises(ValueError):m.fill_output(self.before,self.c['witness']['query'],self.c['witness']['query']['actual'],bytes(raw))
    def test_upload_geometry_expansion(self):
        raw=bytearray(self.upload);struct.pack_into('<I',raw,4,1)
        with self.assertRaises(ValueError):m.fill_output(self.before,self.c['witness']['query'],self.c['witness']['query']['actual'],bytes(raw))
    def test_wrong_constant_argument(self):
        a=dict(self.c['witness']['query']['actual'],fill_value=1)
        with self.assertRaises(ValueError):m.fill_output(self.before,self.c['witness']['query'],a,self.upload)
    def test_no_clipping(self):
        q=dict(self.c['witness']['query'],regions=[[0,0,800,600]])
        with self.assertRaises(ValueError):m.fill_output(self.before,q,q['actual'],self.upload)
    def test_gpu_exterior_corruption(self):
        raw=bytearray(self.actual);offset=int(np.flatnonzero(~self.mask)[0])*2;raw[offset]^=1
        with self.assertRaises(ValueError):m.checked_gpu_output(bytes(raw),self.expected,self.before,self.mask)
    def test_gpu_interior_corruption(self):
        raw=bytearray(self.actual);offset=int(np.flatnonzero(self.mask)[0])*2;raw[offset]^=1
        with self.assertRaises(ValueError):m.checked_gpu_output(bytes(raw),self.expected,self.before,self.mask)
    def test_gpu_truncated_plane(self):
        with self.assertRaises(ValueError):m.checked_gpu_output(self.actual[:-2],self.expected,self.before,self.mask)
    def test_gpu_omitted_update(self):
        with self.assertRaises(ValueError):m.checked_gpu_output(self.before.astype('<u2').tobytes(),self.expected,self.before,self.mask)
    def test_no_expected_output_given_to_replayer(self):
        records=list(m.commands(m.replay_stream(self.before,self.upload)))
        self.assertEqual([r[0] for r in records],[1,2,6,7,8]);self.assertEqual(records[1][2],self.upload)
        self.assertEqual(records[0][2][28:],self.before.astype('<u2').tobytes())
    def test_unknown_prestate(self):self.bad(lambda c:c['witness'].update(prestate_fully_known=False))
    def test_failed_caller(self):self.bad(lambda c:c['witness']['caller'].__setitem__(13,1))
    def test_after_handle_retirement(self):self.bad(lambda c:c['witness'].update(deleted=c['witness']['sequence']))
    def test_outside_ordered_fence(self):self.bad(lambda c:c['witness'].update(lower_fence=c['witness']['sequence']))
    def test_no_op_fill_credit(self):self.bad(lambda c:c['witness'].update(changed_words=0))
    def test_no_historical_high_paint_invention(self):self.bad(lambda c:c['witness']['native_replay'].update(historical_missing_paint_recovered=True))
    def test_original_pixels_refused(self):self.bad(lambda c:c['witness']['native_replay'].update(original_pixels_used_as_native_inputs=True))
    def test_expected_pixels_refused(self):self.bad(lambda c:c['witness']['native_replay'].update(expected_pixels_supplied_to_native=True))
    def test_replay_mismatch_refused(self):self.bad(lambda c:c['witness']['native_replay']['proof'].update(mismatches=1))
    def test_low_presentation_required(self):self.bad(lambda c:c['presentations'].clear())
    def test_only_two_rows_close(self):
        with Reader(ROOT) as r:q=j.join(r,self.analysis,self.parent,self.plan)
        changes=[(a,b) for a,b in zip(self.parent['requirements'],q['requirements']) if a!=b]
        self.assertEqual(len(changes),2);self.assertTrue(all(b['id']==m.LEAF for a,b in changes))
        self.assertEqual(q['decisions'],self.parent['decisions'])
    def test_missing_domain_refused(self):
        q=copy.deepcopy(self.analysis);q['cases'].pop()
        with Reader(ROOT) as r,self.assertRaises(ValueError):j.join(r,q,self.parent,self.plan)
    def test_duplicate_domain_refused(self):
        q=copy.deepcopy(self.analysis);q['cases'][0]=q['cases'][1]
        with Reader(ROOT) as r,self.assertRaises(ValueError):j.join(r,q,self.parent,self.plan)
    def test_missing_parent_admission_refused(self):
        p=copy.deepcopy(self.parent)
        for row in p['requirements']:row['admitted_execution_evidence']=[]
        with Reader(ROOT) as r,self.assertRaises(ValueError):j.join(r,self.analysis,p,self.plan)
    def test_historical_state_claim_refused(self):
        q=dict(self.analysis,current_native_engine_execution=True)
        with Reader(ROOT) as r,self.assertRaises(ValueError):j.join(r,q,self.parent,self.plan)

if __name__=='__main__':unittest.main()
