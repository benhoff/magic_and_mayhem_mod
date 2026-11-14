#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_player_secondary_matrix as matrix
import broader_player_current_interaction_matrix as previous


class Reader:
    def __init__(self,phase,record):self.phase=phase;self.record=record;self.changed=set()
    def digest(self,name):return 'changed'if name in self.changed else'h'
    def json(self,name):return copy.deepcopy(self.phase if name=='phase'else self.record)


class Current(unittest.TestCase):
    def setUp(self):
        self.spec=dict(plan='phase',case={'id':'pilot','report':'a'},role='world',family='interaction');self.row={}
        self.value=dict(acquisition={'record':'a','sha256':'h'},validation={'record':'v','sha256':'h'},phase={'current_native_source_candidate':True})
        self.record=dict(success=True,kind='current_secondary_runtime_pre_admission_verification',phase={'record':'phase','sha256':'h'},acquisition=self.value['acquisition'],validation=self.value['validation'],actual_ledger_rows_renamed=False,sources={'source.py':'h'},scene_sha256='new-dll',actual_compiled_header_sha256='derived',staged_compiler_manifest={'record':'compiled','sha256':'h'})
        self.record['actual_subprocess_selection']={'record':'child','sha256':'h'}
        self.phase={'scene_runtime_variant':{'record':'build','sha256':'h'}};self.reader=Reader(self.phase,self.record)
        self.actual=dict(verified=True,scene_sha256='new-dll',actual_compiled_header_sha256='derived',staged_compiler_manifest={'record':'compiled','sha256':'h'})
    def run_current(self):
        with patch.object(matrix,'prior_current',return_value=self.value)as prior,patch.object(matrix,'verify_staged',return_value=self.actual)as verify,patch.object(matrix,'verify_selection',return_value=({'record':'child','sha256':'h'},{})):
            result=matrix.current(ROOT,self.reader,self.spec,self.row)
            prior.assert_called_once_with(ROOT,self.reader,self.spec,self.row)
            return result,verify
    def refuse(self):
        with self.assertRaises(ValueError):self.run_current()
    def test_retained_predicate_function_is_the_exact_previous_function(self):self.assertIs(matrix.prior_current,previous.current)
    def test_base_phase_delegates_unchanged(self):
        self.reader.phase={};result,verify=self.run_current();self.assertIs(result,self.value);verify.assert_not_called()
    def test_variant_phase_requires_and_records_staged_join(self):
        result,verify=self.run_current();self.assertTrue(result['phase']['derived_runtime_variant_verified']);self.assertTrue(result['phase']['current_native_source_candidate']);verify.assert_called_once()
    def test_failed_verification_report_refuses(self):self.reader.record['success']=False;self.refuse()
    def test_wrong_acquisition_refuses(self):self.reader.record['acquisition']={};self.refuse()
    def test_wrong_validation_refuses(self):self.reader.record['validation']={};self.refuse()
    def test_wrong_phase_refuses(self):self.reader.record['phase']={};self.refuse()
    def test_source_drift_refuses(self):self.reader.changed.add('source.py');self.refuse()
    def test_renamed_actual_row_refuses(self):self.reader.record['actual_ledger_rows_renamed']=True;self.refuse()
    def test_changed_actual_subprocess_selection_refuses(self):self.reader.record['actual_subprocess_selection']={};self.refuse()
    def test_stale_scene_binary_refuses(self):self.actual['scene_sha256']='different';self.refuse()
    def test_stale_compiled_header_refuses(self):self.actual['actual_compiled_header_sha256']='different';self.refuse()
    def test_stale_staged_manifest_refuses(self):self.actual['staged_compiler_manifest']={};self.refuse()


if __name__=='__main__':unittest.main()
