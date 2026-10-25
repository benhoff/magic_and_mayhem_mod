#!/usr/bin/env python3
"""Functional lifecycle refusal controls; no game or original execution credit."""
import copy
import importlib.util
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_functional_acceptance import ACTIONS,STEPS,validate_strict_functional
spec=importlib.util.spec_from_file_location('functional_campaign',Path(__file__).resolve().parents[1]/'tools/test-native-campaign.py')
campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)


def fixture():
    rate=lambda cycle:dict(cycle=cycle,seconds=10.,native_frames=10,painted_frames=10,world_imports=10,native_fps=1.,paint_fps=1.)
    return dict(success=True,sources_stable=True,original_game_executed=True,original_verified_after=True,
        ordinary_campaign_accepted=False,fps_policy='diagnostic',stress_seconds=10,shell_exit=0,
        flow=dict(success=True,normal_quit=True,native_command_fallback=False,native_command_frames=30,native_painted_frames=30,
                  route_screenshots=False,steps=[dict(step=s) for s in STEPS],gameplay_rates=[rate(0),rate(1)]),
        resident=dict(completed='64',imports='64',verified_returns='64',error='',pending=False,normal_quit_retired=True,
                      world_readbacks=0,native_readbacks=0,rgba_readbacks=0,world_pixel_reply_bytes=0,engine_fps=5.),
        engine_journey=dict(actions=copy.deepcopy(ACTIONS),world_ticks=3,world_resumes=4,protocol_version=17),
        mini_readiness=[dict(verified=True,action=a) for a in [4,3,3]],
        physical_stress=[dict(verified=True,seconds=10,actions=40)]*2,native_compiler_inputs_checked=100,
        original_process=dict(original_exit_code=0,cleanup_after_original_exit=True),
        admission=dict(required_backing_mode=3,host_graphics_devices=False,legacy_embedding=False,
                       namespace_network=True,namespace_pid=True,namespace_ipc=True),
        native_only_outcomes=[dict(state='completed',forwarded=0)],routes=dict(native=20,forwarded=0),
        graphics_retirement=dict(process_exit=True,resources_released=True,ddraw_module_loaded=False,surviving_owners=[],counts=dict(devices=0,surfaces=0,dcs=0,clippers=0,palettes=0,pixel_bytes=0)))


class FunctionalAdmission(unittest.TestCase):
    def test_low_fps_does_not_waive_functional_checks_or_claim_benchmark(self):
        value=fixture();result=validate_strict_functional(value)
        self.assertTrue(result['verified']);self.assertEqual(result['observed_engine_fps'],5.)
        self.assertFalse(value['ordinary_campaign_accepted'])
        campaign.validate_flow(value['flow'],stress_seconds=10,min_fps=None,normal_quit=True)
        with self.assertRaisesRegex(ValueError,'too slow'):campaign.validate_flow(value['flow'],stress_seconds=10,min_fps=19,normal_quit=True)
        gates=campaign.validate_resident_cpu_census(value['resident'],None)
        self.assertTrue(gates['worlds_passed']);self.assertIsNone(gates['fps_passed'])
        with self.assertRaisesRegex(ValueError,'FPS below target'):campaign.validate_resident_cpu_census(value['resident'],19)

    def test_in_memory_callback_tuples_and_serialized_lists_agree(self):
        value=fixture();value['engine_journey']['actions']=[tuple(row) for row in value['engine_journey']['actions']]
        self.assertTrue(validate_strict_functional(value)['verified'])

    def test_partial_flows_provenance_and_ownership_fail_even_with_success_flag(self):
        mutations=[lambda v:v.update(sources_stable=False),lambda v:v.update(original_game_executed=False),
          lambda v:v.update(original_verified_after=False),lambda v:v.update(ordinary_campaign_accepted=True),
          lambda v:v.update(fps_policy='required'),lambda v:v.update(stress_seconds=0),
          lambda v:v['flow'].update(native_command_fallback=True),lambda v:v['flow']['steps'].pop(),
          lambda v:v['flow'].update(normal_quit=False),lambda v:v['flow'].update(native_painted_frames=0),
          lambda v:v['flow']['gameplay_rates'].pop(),lambda v:v['flow']['gameplay_rates'][0].update(world_imports=0),
          lambda v:v['flow']['gameplay_rates'][0].update(seconds=float('nan')),
          lambda v:v['flow']['gameplay_rates'][0].update(native_frames=-1),
          lambda v:v['resident'].update(imports='63'),lambda v:v['resident'].update(completed='12',imports='12',verified_returns='12'),
          lambda v:v['resident'].update(pending=True),lambda v:v['resident'].update(error='refused'),
          lambda v:v['resident'].update(world_pixel_reply_bytes=16),lambda v:v['engine_journey']['actions'].reverse(),
          lambda v:v['engine_journey'].update(world_resumes=0),lambda v:v['mini_readiness'].pop(),
          lambda v:v['mini_readiness'][0].update(action=3),lambda v:v.update(physical_stress=[]),
          lambda v:v.update(native_compiler_inputs_checked=0),lambda v:v['original_process'].update(original_exit_code=1),
          lambda v:v['admission'].update(host_graphics_devices=True),lambda v:v['admission'].update(namespace_pid=False),
          lambda v:v['routes'].update(forwarded=1),lambda v:v['native_only_outcomes'][0].update(state='failed'),
          lambda v:v['graphics_retirement']['counts'].update(surfaces=1),lambda v:v['graphics_retirement'].update(ddraw_module_loaded=True)]
        for mutate in mutations:
            value=fixture();mutate(value)
            with self.subTest(mutation=mutate),self.assertRaises(ValueError):validate_strict_functional(value)

    def test_diagnostic_public_validation_still_requires_advancing_output(self):
        value=fixture();value['flow']['gameplay_rates'][0].update(native_frames=0)
        with self.assertRaisesRegex(ValueError,'advancing'):campaign.validate_flow(value['flow'],stress_seconds=10,min_fps=None,normal_quit=True)
        with self.assertRaises(ValueError):campaign.validate_resident_cpu_census(dict(fixture()['resident'],imports=63),None)


if __name__=='__main__':unittest.main()
