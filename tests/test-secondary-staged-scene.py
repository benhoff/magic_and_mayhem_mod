#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_secondary_staged_scene import staged_values, BASE


class Stage(unittest.TestCase):
    def setUp(self):
        self.phase={'record':'phase','sha256':'phase-sha'};self.ref={'record':'build','sha256':'build-sha'}
        self.variant=dict(scene_sha256='new-dll',actual_compiled_header_sha256='derived',physical_header_sha256='base')
        selection=dict(verified=True,build=self.ref,scene_sha256='new-dll',actual_compiled_header_sha256='derived',source_hash_waivers=[],materialized_checked=True)
        self.acquired=dict(success=True,sources_stable=True,actual_provider_verified=True,broader_phase=self.phase,
            provider_adaptation=dict(scene_runtime_variant=dict(build=self.ref,base_runtime_sources_edited=False,
                actual_compiled_header_is_derived=True,additional_exact_caller_pairs=2,
                other_builders_actions_deadlines_and_capture_policy_changed=False,
                selections=[dict(module='tools/prepare-menu-observer.py',proof=selection)])))
        self.stage=dict(resident_world=True,native_only_policy='native-only-rendering-v1',glyph_backend_mode='takeover',resident_scene_sha256='new-dll',glyph_dll_sha256='new-dll')
        self.compiled=dict(sha256='new-dll',sources={BASE:'derived'},physical_derivation_inputs={BASE:'base'})
    def prove(self):return staged_values(self.acquired,self.stage,self.compiled,self.variant,self.phase,self.ref)
    def refuse(self):
        with self.assertRaises(ValueError):self.prove()
    def test_exact_staged_variant(self):self.assertTrue(self.prove()['verified'])
    def test_failed_acquisition_refuses(self):self.acquired['success']=False;self.refuse()
    def test_source_drift_refuses(self):self.acquired['sources_stable']=False;self.refuse()
    def test_missing_provider_refuses(self):self.acquired['actual_provider_verified']=False;self.refuse()
    def test_other_phase_refuses(self):self.acquired['broader_phase']={};self.refuse()
    def test_edited_base_source_refuses(self):self.acquired['provider_adaptation']['scene_runtime_variant']['base_runtime_sources_edited']=True;self.refuse()
    def test_changed_actions_refuse(self):self.acquired['provider_adaptation']['scene_runtime_variant']['other_builders_actions_deadlines_and_capture_policy_changed']=True;self.refuse()
    def test_no_scene_selection_refuses(self):self.acquired['provider_adaptation']['scene_runtime_variant']['selections']=[];self.refuse()
    def test_selection_from_different_build_refuses(self):self.acquired['provider_adaptation']['scene_runtime_variant']['selections'][0]['proof']['build']={};self.refuse()
    def test_source_waiver_refuses(self):self.acquired['provider_adaptation']['scene_runtime_variant']['selections'][0]['proof']['source_hash_waivers']=['base'];self.refuse()
    def test_unchecked_materialization_refuses(self):self.acquired['provider_adaptation']['scene_runtime_variant']['selections'][0]['proof']['materialized_checked']=False;self.refuse()
    def test_old_scene_binary_refuses(self):self.stage['resident_scene_sha256']='old-dll';self.refuse()
    def test_old_glyph_binary_refuses(self):self.stage['glyph_dll_sha256']='old-dll';self.refuse()
    def test_hidden_compiled_header_refuses(self):self.compiled['sources'][BASE]='base';self.refuse()
    def test_physical_and_compiled_sources_conflated_refuse(self):self.compiled['physical_derivation_inputs'][BASE]='derived';self.refuse()
    def test_another_policy_refuses(self):self.stage['native_only_policy']='fallback';self.refuse()


if __name__=='__main__':unittest.main()
