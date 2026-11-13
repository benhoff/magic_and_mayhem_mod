#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_player_derived_matrix as matrix
import broader_player_secondary_matrix as previous
class Reader:
 def __init__(self,phase,record):self.phase=phase;self.record=record;self.changed=set()
 def digest(self,n):return 'changed'if n in self.changed else'h'
 def json(self,n):return copy.deepcopy(self.phase if n=='phase'else self.record)
class Current(unittest.TestCase):
 def setUp(self):
  self.spec=dict(plan='phase',case={'id':'case'},role='world',family='interaction');self.value=dict(acquisition={'record':'a','sha256':'h'},validation={'record':'v','sha256':'h'},phase={'current_native_source_candidate':True});self.phase=dict(scene_runtime_variant={},derived_native_admission={},recent_interaction_sources={},interaction_pause_policy={});self.proof=dict(verified=True,actual_owned_runner_admission={'record':'owned','sha256':'h'},actual_compiled_header_sha256='compiled',physical_header_sha256='physical');self.record=dict(schema=1,kind='current_derived_native_admission_pre_admission_verification',success=True,phase={'record':'phase','sha256':'h'},acquisition=self.value['acquisition'],validation=self.value['validation'],actual_ledger_rows_renamed=False,sources={'source':'h'},**self.proof);self.record['recent_interaction_sources']={'verified':True};self.record['public_pause_collection']={'verified':True};self.reader=Reader(self.phase,self.record)
 def run_current(self):
  with patch.object(matrix.previous,'current',return_value=self.value)as old,patch.object(matrix,'verify_admission',return_value=self.proof)as verify,patch.object(matrix,'verify_recent',return_value={'verified':True}),patch.object(matrix,'verify_pause',return_value={'verified':True}):
   result=matrix.current(ROOT,self.reader,self.spec,{});old.assert_called_once_with(ROOT,self.reader,self.spec,{});return result,verify
 def refuse(self):
  with self.assertRaises(ValueError):self.run_current()
 def test_exact_catalog_predicate_code_object_preserved(self):self.assertIs(matrix.specs.__code__,previous.specs.__code__)
 def test_base_context_unchanged(self):self.reader.phase={};result,guard=self.run_current();self.assertIs(result,self.value);guard.assert_not_called()
 def test_derived_context_requires_current_startup_join(self):result,guard=self.run_current();self.assertTrue(result['phase']['derived_native_admission_verified']);guard.assert_called_once()
 def test_missing_declaration_refuses(self):self.reader.phase.pop('derived_native_admission');self.refuse()
 def test_failed_record_refuses(self):self.reader.record['success']=False;self.refuse()
 def test_changed_phase_refuses(self):self.reader.record['phase']={};self.refuse()
 def test_changed_acquisition_refuses(self):self.reader.record['acquisition']={};self.refuse()
 def test_changed_validation_refuses(self):self.reader.record['validation']={};self.refuse()
 def test_changed_source_refuses(self):self.reader.changed.add('source');self.refuse()
 def test_actual_row_rename_refuses(self):self.reader.record['actual_ledger_rows_renamed']=True;self.refuse()
 def test_actual_runner_receipt_drift_refuses(self):self.proof['actual_owned_runner_admission']={};self.refuse()
 def test_compiled_header_drift_refuses(self):self.proof['actual_compiled_header_sha256']='other';self.refuse()
 def test_physical_header_drift_refuses(self):self.proof['physical_header_sha256']='other';self.refuse()
if __name__=='__main__':unittest.main()
