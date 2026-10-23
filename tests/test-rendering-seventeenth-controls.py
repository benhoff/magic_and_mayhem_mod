#!/usr/bin/env python3
"""Negative controls for fixed-length and failed-execution evidence admission."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_seventeenth_controls import require_continuous, require_public_implementation, public_portrait_pixels


def record(count=4096, verify=False):
    value = {name: True for name in ('sources_stable', 'frozen_sources_stable', 'generated_sources_stable',
                                    'supplied_declaration_stable', 'inputs_stable', 'original_manifest_verified_before_after')}
    value.update(success=True, resident=False, requested_queues=count,
                 admission_policy=dict(functional_only=False), performance_budget=dict(passed=True),
                 completed=dict(native_continuous=dict(success=True, original_returns=count,
                    native_completions=count, remaining_surfaces=0, qt_paints=count, coalesced_presentations=0, verify=verify,
                    cpu_reference_queues=count if verify else 0)))
    return value


class Acceptance(unittest.TestCase):
    def test_full_positive(self):
        require_continuous(record(), 'normal')
        require_continuous(record(verify=True), 'verification')
    def test_short_prefix_refuses(self):
        with self.assertRaises(ValueError): require_continuous(record(128), 'normal')
    def test_functional_only_cannot_complete_performance(self):
        value = record(); value['admission_policy']['functional_only'] = True
        with self.assertRaises(ValueError): require_continuous(value, 'normal')
    def test_resident_cannot_replace_pixel_verification(self):
        value = record(verify=True); value['resident'] = True
        with self.assertRaises(ValueError): require_continuous(value, 'verification')
    def test_bad_pixels_cannot_be_expected_timing_failure(self):
        value = record(128); value.update(success=False, error='Full-path performance budget exceeded',
            performance_budget=dict(passed=False, impossible_negative_control=True, median_limit_ms=.001, checks=dict(warm_median=False)))
        require_continuous(value, 'impossible')
        value['completed']['native_continuous']['success'] = False
        with self.assertRaises(ValueError): require_continuous(value, 'impossible')
    def test_real_budget_cannot_be_impossible_control(self):
        value = record(128); value.update(success=False, error='Full-path performance budget exceeded',
            performance_budget=dict(passed=False, impossible_negative_control=True, median_limit_ms=50, checks=dict(warm_median=False)))
        with self.assertRaises(ValueError): require_continuous(value, 'impossible')
    def test_lost_completion_refuses(self):
        value = record(); value['completed']['native_continuous']['native_completions'] = 4095
        with self.assertRaises(ValueError): require_continuous(value, 'normal')
    def test_early_death_cannot_complete_failure_control(self):
        value = record(); value.update(mode='consumer-death', expected_terminal_failure=True,
                                     completed_success_claimed=False, fault_generation=34)
        require_continuous(value, 'consumer-death')
        value['fault_generation'] = 32
        with self.assertRaises(ValueError): require_continuous(value, 'consumer-death')
    def test_stale_provenance_refuses(self):
        value = record(); value['sources_stable'] = False
        with self.assertRaises(ValueError): require_continuous(value, 'normal')



def public_record():
    return dict(success=False,ordinary_campaign_accepted=False,error='Resident World engine FPS below target: 17.5',
        **{k:True for k in ('casting_verified','combat_verified','movement_verified','normal_quit_verified','sources_unchanged','inputs_stable','original_manifest_verified_after','private_wine_stopped')},
        changed_sources=[],spell_cases=dict(verified=True),portrait=dict(maximum_channel_error=1,native_fps=19.5,paint_fps=19.5,worst_window_fps=19),
        resident_world=dict(completed='1024',imports='1024',verified_returns='1024',normal_quit_retired=True,pending=False,error='',world_readbacks=0,native_readbacks=0,rgba_readbacks=0,world_pixel_reply_bytes=0,engine_fps=17.5),
        ordinary_resident_gates=dict(minimum_fps=19.,worlds_passed=True,fps_passed=False))

class ImplementationStage(unittest.TestCase):
    def test_performance_failure_stays_failed(self):
        v=public_record();result=require_public_implementation(v)
        self.assertEqual(result['stages'],['implementation']);self.assertTrue(result['implementation_verified'])
        self.assertFalse(result['performance_gate_passed']);self.assertFalse(result['ordinary_campaign_accepted']);self.assertFalse(v['success'])
    def test_actual_ordinary_pass_remains_pass(self):
        v=public_record();v.update(success=True,ordinary_campaign_accepted=True);v.pop('error');v['resident_world']['engine_fps']=19.5;v['ordinary_resident_gates']['fps_passed']=True
        self.assertTrue(require_public_implementation(v)['performance_gate_passed'])
    def test_other_failure_cannot_earn_credit(self):
        for error in ('Portrait rendering too slow','Gameplay too slow','Source changed during execution','Player wizard died'):
            v=public_record();v['error']=error
            with self.assertRaises(ValueError):require_public_implementation(v)
    def test_mismatched_or_incomplete_returns_refuse(self):
        for key,value in [('verified_returns','1023'),('pending',True),('normal_quit_retired',False),('world_readbacks',1)]:
            v=public_record();v['resident_world'][key]=value
            with self.assertRaises(ValueError):require_public_implementation(v)
    def test_source_cleanup_and_functional_failures_refuse(self):
        for key in ('casting_verified','combat_verified','movement_verified','normal_quit_verified','sources_unchanged','inputs_stable','original_manifest_verified_after','private_wine_stopped'):
            v=public_record();v[key]=False
            with self.assertRaises(ValueError):require_public_implementation(v)
    def test_no_false_acceptance_or_lowered_threshold(self):
        v=public_record();v['ordinary_campaign_accepted']=True
        with self.assertRaises(ValueError):require_public_implementation(v)
        v=public_record();v['ordinary_resident_gates']['minimum_fps']=17
        with self.assertRaises(ValueError):require_public_implementation(v)
    def window_flow(self):
        import importlib.util
        path=Path(__file__).resolve().parents[1]/'tools/test-native-campaign.py'
        spec=importlib.util.spec_from_file_location('test_implementation_flow',path)
        m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        steps=m.STEPS+['campaign-quit-menu','quit-no-resumed','campaign-quit-menu','campaign-defeat','quit-main']
        rates=[dict(cycle=c,seconds=2,native_frames=0 if c==0 and n==0 else 40,painted_frames=0 if c==0 and n==0 else 40,native_fps=0 if c==0 and n==0 else 20,paint_fps=0 if c==0 and n==0 else 20) for c in [0,1] for n in range(5)]
        return dict(success=True,native_command_fallback=False,native_command_frames=1024,normal_quit=True,route_screenshots=False,steps=[dict(step=s) for s in steps],gameplay_rates=rates)
    def test_window_timing_refusal_preserves_both_performance_failures(self):
        v=public_record();v['error']='Gameplay too slow in cycle 0: native 16.0, paints 16.0 FPS; minimum 19.0'
        result=require_public_implementation(v,flow=self.window_flow())
        self.assertTrue(result['implementation_verified']);self.assertFalse(result['performance_gate_passed']);self.assertFalse(v['success'])
        v['resident_world']['engine_fps']=19.5;v['ordinary_resident_gates']['fps_passed']=True
        result=require_public_implementation(v,flow=self.window_flow())
        self.assertTrue(result['aggregate_fps_passed']);self.assertFalse(result['performance_gate_passed'])
    def test_timing_label_cannot_hide_flow_or_mismatched_failure(self):
        v=public_record();v['error']='Gameplay too slow in cycle 0: native 16.0, paints 16.0 FPS; minimum 19.0'
        with self.assertRaises(ValueError):require_public_implementation(v)
        for key,bad in [('success',False),('normal_quit',False),('native_command_fallback',True),('steps',[])]:
            flow=self.window_flow();flow[key]=bad
            with self.assertRaises(ValueError):require_public_implementation(v,flow=flow)
        flow=self.window_flow();v['error']=v['error'].replace('16.0','15.0')
        with self.assertRaises(ValueError):require_public_implementation(v,flow=flow)
    def test_failed_portrait_timing_requires_independent_pixel_and_flow_proof(self):
        v=public_record();v['error']='Portrait rendering too slow';v.pop('portrait')
        with self.assertRaises(ValueError):require_public_implementation(v,flow=self.window_flow())
        pixels=dict(verified=True,maximum_channel_error=1,ordinary_minimum_fps=19,ordinary_timing_error=v['error'])
        result=require_public_implementation(v,flow=self.window_flow(),portrait_pixels=pixels)
        self.assertTrue(result['implementation_verified']);self.assertFalse(result['performance_gate_passed'])
        pixels['maximum_channel_error']=2
        with self.assertRaises(ValueError):require_public_implementation(v,flow=self.window_flow(),portrait_pixels=pixels)
    def test_independent_portrait_compares_exact_retained_images_after_timing_failure(self):
        import tempfile,hashlib
        from PIL import Image
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);name='combat-01-portrait-stress';hashes={}
            for route in ('native','original'):
                p=root/(name+'-'+route+'.png');Image.new('RGB',(800,600),(10,20,30)).save(p);hashes[route]=hashlib.sha256(p.read_bytes()).hexdigest()
            inputs=dict(portrait_stress=dict(seconds=10,cycles=3),actions=[dict(action='portrait-stress-start',seconds=0),*[dict(action='portrait-recenter',seconds=n) for n in [1,3,5]],dict(action='portrait-stress-complete',seconds=10)],captures=[dict(name=name,metadata=dict(fallback=False,native_saved=True,original_saved=True),image_sha256=hashes)])
            flow=dict(gameplay_rates=[dict(cycle=0,seconds=1,native_frames=18,painted_frames=18,native_fps=18,paint_fps=18) for n in range(12)])
            result=public_portrait_pixels(inputs,flow,root)
            self.assertTrue(result['verified']);self.assertEqual(result['maximum_channel_error'],0);self.assertEqual(result['ordinary_timing_error'],'Portrait rendering too slow')
            p=root/(name+'-native.png');image=Image.new('RGB',(800,600),(10,20,30));image.putpixel((720,510),(12,20,30));image.save(p)
            with self.assertRaisesRegex(ValueError,'changed'):public_portrait_pixels(inputs,flow,root)
            hashes['native']=hashlib.sha256(p.read_bytes()).hexdigest()
            with self.assertRaisesRegex(ValueError,'differs'):public_portrait_pixels(inputs,flow,root)
    def test_portrait_and_spell_proof_remain_required(self):
        v=public_record();v['portrait']['maximum_channel_error']=2
        with self.assertRaises(ValueError):require_public_implementation(v)
        v=public_record();v['spell_cases']['verified']=False
        with self.assertRaises(ValueError):require_public_implementation(v)

if __name__ == '__main__': unittest.main()
