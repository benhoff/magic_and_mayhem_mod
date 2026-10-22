"""Refuse altered storage action/save cohorts without starting Wine or a renderer."""
from copy import deepcopy
import json
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import read_text as evidence_text
import drawing_recorded_session as session
from drawing_corpus import artifact

class Cases(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog=json.loads(evidence_text(ROOT/session.STORAGE_CATALOG))
        cls.original=json.loads(evidence_text(ROOT/'research/runtime/storage-action-original-20261016.json'))
        cls.journal=artifact(ROOT,cls.catalog['traces']['journal'])
        cls.actions=artifact(ROOT,cls.catalog['action_frames'])
    def test_legacy_and_explicit_retention_ids_coexist(self):
        self.assertEqual(session.retention_id(self.catalog),session.STORAGE_RETENTION)
        self.assertEqual(session.retention_id({'schema':1}),session.RETENTION)
        self.assertEqual(session.retention_id({'schema':3,'retention_evidence':'new.reference'}),'new.reference')
        for value in (None,'',3):
            with self.assertRaises(ValueError):session.retention_id({'schema':3,'retention_evidence':value})
    def test_corrected_profile_cannot_drop_policy_or_witnesses(self):
        original={'egl_correct_stale_upload_policy':True,'egl_destination_gpu_witness':True,'egl_xcb_image_witness':True}
        self.assertEqual(session.presentation_profile(original)['name'],'stale_upload_crop')
        self.assertEqual(session.presentation_profile({})['name'],'unmodified_upload')
        for key,value in [('egl_destination_gpu_witness',False),('egl_xcb_image_witness',False),('egl_correct_stale_upload_policy',1),('egl_drawable_rebind_policy',True),('egl_drawable_release_policy',True),('egl_viewport_refresh_policy',True)]:
            with self.subTest(key=key),self.assertRaises(ValueError):session.presentation_profile(dict(original,**{key:value}))
    def test_explicit_corrected_profile_and_witness_admission(self):
        catalog=json.loads(evidence_text(ROOT/'tests/fixtures/drawing-storage-corrected-session/session.json'))
        original=json.loads(evidence_text(ROOT/'research/runtime/egl-destination-original-corrected-20261016.json'))
        # Check complete cohorts and original metadata, independently of the immutable catalog guard.
        files=session.admit_witnesses(ROOT,catalog,original)
        self.assertEqual(len(files),222)
        self.assertEqual(catalog['reference_profile'],session.presentation_profile(original))
        for change in ('missing','order','identity','metadata','extent','policy'):
            changed=deepcopy(catalog);source=deepcopy(original)
            if change=='missing':changed['witnesses'].pop()
            elif change=='order':changed['witnesses'].reverse()
            elif change=='identity':source['egl_swap_completion_observation']['frames'][0]['call']+=1
            elif change=='metadata':changed['witnesses'][0]['files'].pop(next(iter(changed['witnesses'][0]['files'])))
            elif change=='extent':source['egl_swap_completion_observation']['frames'][0]['width']-=1
            else:changed['reference_profile']['policies']['egl_correct_stale_upload_policy']=False
            with self.subTest(change=change),self.assertRaises(ValueError):session.admit_witnesses(ROOT,changed,source)
    def test_corrected_reference_cannot_be_relabelled_legacy(self):
        catalog=json.loads(evidence_text(ROOT/'tests/fixtures/drawing-storage-corrected-session/session.json'))
        register=json.loads(evidence_text(ROOT/'research/runtime/coverage/register.json'))
        original=json.loads(evidence_text(ROOT/'research/runtime/egl-destination-original-corrected-20261016.json'))
        bound=json.loads(evidence_text(ROOT/'research/runtime/egl-destination-native-corrected-20261016.json'))
        for mode in ('legacy','profile','retention-kind'):
            changed=deepcopy(catalog);reg=deepcopy(register)
            if mode=='legacy':changed.update(schema=2)
            elif mode=='profile':changed['reference_profile']['name']='unmodified_upload'
            else:
                next(e for e in reg['evidence'] if e['id']==session.retention_id(changed))['kind']='native_integration'
            # Bypass only the catalog hash to exercise semantic provenance checks.
            values=[changed,original,bound]
            with patch('rendering_coverage.evidence_record',side_effect=values),self.subTest(mode=mode),self.assertRaises(ValueError):session.admit(ROOT,changed,reg)
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
        suite=json.loads(evidence_text(ROOT/'tests/fixtures/drawing-journeys/storage-suite.json'))
        # The focused suite retains exactly the default acceptance denominator.
        self.assertEqual(len(suite['requirements']),14);self.assertEqual(len(suite['operation_requirements']),21)
if __name__=='__main__':unittest.main()
