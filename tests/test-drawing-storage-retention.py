"""Refuse altered storage action/save cohorts without starting Wine or a renderer."""
from copy import deepcopy
import json
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import drawing_recorded_session as session
from drawing_corpus import artifact

class Cases(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog=json.loads((ROOT/session.STORAGE_CATALOG).read_text())
        cls.original=json.loads((ROOT/'research/runtime/storage-action-original-20261016.json').read_text())
        cls.journal=artifact(ROOT,cls.catalog['traces']['journal'])
        cls.actions=artifact(ROOT,cls.catalog['action_frames'])
    def test_complete_original_action_and_save_cohort(self):
        session.admit_actions(ROOT,self.catalog,self.original,self.journal)
    def check_action_mutation(self,mutate):
        changed=mutate(self.actions)
        real=artifact
        def read(root,entry):
            return changed if entry==self.catalog['action_frames'] else real(root,entry)
        with patch.object(session,'artifact',side_effect=read),self.assertRaises((ValueError,KeyError)):
            session.admit_actions(ROOT,self.catalog,self.original,self.journal)
    def test_missing_or_extra_action_sidecar(self):
        for changed in (self.actions[:-128],self.actions+self.actions[-128:]):self.check_action_mutation(lambda _:changed)
    def test_reordered_action_sidecars(self):self.check_action_mutation(lambda a:a[128:256]+a[:128]+a[256:])
    def test_wrong_frame_call_or_epoch(self):
        for word in (4,5,6,18):
            def change(a):
                b=bytearray(a);struct.pack_into('<I',b,word*4,struct.unpack_from('<I',b,word*4)[0]+1);return bytes(b)
            self.check_action_mutation(change)
    def test_save_hash_or_extent_changed(self):
        for key,value in (('saved_sha256','0'*64),('save_bytes',0)):
            original=deepcopy(self.original);original['storage_flow'][key]=value
            with self.assertRaises(ValueError):session.admit_actions(ROOT,self.catalog,original,self.journal)
    def test_missing_reordered_or_stale_world_checkpoint(self):
        for change in ('missing','order','stale','rebuild'):
            original=deepcopy(self.original);flow=original['storage_flow']
            if change=='missing':flow['checkpoints'].pop()
            elif change=='order':flow['checkpoints'].reverse()
            elif change=='stale':flow['checkpoints'][0]['world_returns']=flow['checkpoints'][0]['source_base']+2
            else:flow['display_rebuilds']=[0,1]
            with self.assertRaises(ValueError):session.admit_actions(ROOT,self.catalog,original,self.journal)
    def test_changed_recorded_action_binding_refuses(self):
        original=deepcopy(self.original);original['action_bound_presentations']['worker_handoffs']=[]
        with self.assertRaises(ValueError):session.admit_actions(ROOT,self.catalog,original,self.journal)
    def test_exact_expected_completed_callbacks_required(self):
        observed=deepcopy(self.original['action_bound_presentations'])
        for event in observed['actions']:
            if event['phase']==2 and event['status']==1:event['action']+=1;break
        original=deepcopy(self.original);original['action_bound_presentations']=observed
        with patch('drawing_action_session.bind',return_value=observed),self.assertRaises(ValueError):
            session.admit_actions(ROOT,self.catalog,original,self.journal)
    def test_partial_boundary_does_not_close_generic_acceptance(self):
        suite=json.loads((ROOT/'tests/fixtures/drawing-journeys/storage-suite.json').read_text())
        # The focused suite retains exactly the default acceptance denominator.
        self.assertEqual(len(suite['requirements']),14);self.assertEqual(len(suite['operation_requirements']),21)
if __name__=='__main__':unittest.main()
