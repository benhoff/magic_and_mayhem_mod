#!/usr/bin/env python3
"""Saved functional successes survive independent failures; no failure is waived."""
import importlib.util
from pathlib import Path
from types import SimpleNamespace
import unittest
ROOT=Path(__file__).resolve().parents[1]
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
import sys
sys.path.insert(0,str(ROOT/'tools'))
policy=load('saved_policy','tools/check-native-campaign-artifacts.py')
helper=load('real_campaign_checks','tools/test-native-campaign.py')


def campaign(success=True):
    return dict(success=success,menu_cycles=1,difficulty=0,normal_quit_required=False,portrait_stress_seconds=10,minimum_fps=20,stress_seconds_per_phase=5)


def checks(failing=()):
    def callback(name,result=None):
        def run(*a,**k):
            if name in failing:raise ValueError(name+' rejected')
            return result
        return run
    return SimpleNamespace(validate_events=callback('engine',{'verified':True}),validate_dialogue_readiness=callback('dialogue',{}),validate_casting_combat=callback('gameplay',{'movement':{'verified':True}}),validate_spell_cases=callback('spell_cases',{}),read_spell_rows=callback('spell_rows',[]),read_rows=callback('rows',[]),validate_portrait_stress=callback('portrait',{}),validate_flow=callback('flow'))


class SavedChecks(unittest.TestCase):
    def test_multiple_failures_retain_other_verified_outcomes(self):
        result=policy.validate_saved(campaign(),dict(success=True),{},ROOT,ROOT,checks({'engine','portrait','flow'}))
        self.assertEqual(set(result['errors']),{'engine','portrait','flow'})
        self.assertTrue(result['casting_verified']);self.assertTrue(result['movement_verified'])
        self.assertFalse(result['success']);self.assertIn('spell_cases',result)

    def test_retained_failed_capture_never_becomes_successful(self):
        c=campaign(False);c['error']='FPS floor'
        result=policy.validate_saved(c,dict(success=True),{},ROOT,ROOT,checks())
        self.assertTrue(result['functional_checks_passed']);self.assertFalse(result['success']);self.assertEqual(result['campaign_error'],'FPS floor')

    def test_engine_rate_and_native_worst_window_remain_independent(self):
        c=campaign();c['ordinary_resident_gates']=dict(worlds_passed=True,fps_passed=False,minimum_fps=20)
        flow=dict(success=True,native_command_fallback=False,native_command_frames=10,route_screenshots=False,steps=[dict(step=s) for s in helper.STEPS],gameplay_rates=[])
        for cycle in (0,1):
            for fps in (30,30,1):flow['gameplay_rates'].append(dict(cycle=cycle,seconds=2,native_frames=60,painted_frames=60,native_fps=fps,paint_fps=fps))
        callbacks=checks();callbacks.validate_flow=helper.validate_flow
        result=policy.validate_saved(c,flow,{},ROOT,ROOT,callbacks)
        self.assertEqual(set(result['errors']),{'flow','resident_performance'})
        self.assertTrue(result['functional_checks_passed']);self.assertFalse(result['success'])

    def test_success_requires_all_checks_and_retained_success(self):
        self.assertTrue(policy.validate_saved(campaign(),dict(success=True),{},ROOT,ROOT,checks())['success'])
        c=campaign();c['ordinary_resident_gates']=dict(worlds_passed=True,fps_passed=True,minimum_fps=10)
        self.assertFalse(policy.validate_saved(c,dict(success=True),{},ROOT,ROOT,checks())['success'])

    def test_malformed_floor_cannot_relax_postcheck(self):
        for floor in (True,0,float('nan'),float('inf'),121):
            c=campaign();c['minimum_fps']=floor
            with self.assertRaises(ValueError):policy.validate_saved(c,dict(success=True),{},ROOT,ROOT,checks())


if __name__=='__main__':unittest.main()
