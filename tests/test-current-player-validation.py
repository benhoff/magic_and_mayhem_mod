#!/usr/bin/env python3
"""Refuse incomplete or over-budget current-player progress without new games."""
import copy
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('current_player_check',ROOT/'tools/check-current-player-validation.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class Progress(unittest.TestCase):
    def setUp(self):
        self.a=dict(scenario='normal-storage',configuration=dict(deadline_ms=530000),flow=dict(success=True),resident=dict(
            completed='6',source_chunk_max_records='16384',source_chunk_max_interval_records='16384',engine_fps=1,
            stage_profile=dict(samples_dropped=0,samples=[dict(generation=str(i),producer_wait_ack_ms=100,native_consume_ms=40)for i in range(1,7)],
            capture=dict(peak_queue_bytes=1000,queue_limit_bytes=256*1024**2,queue_bytes=0,file_bytes=1000))))
        self.observed=dict(swaps=list(range(1,7)),paints={i:(dict(world=str(i)),b'')for i in range(1,7)})
        self.c=dict(comparison=dict(world_presentations=6))

    def check(self):return m.response_progress(self.a,self.observed,self.c)
    def test_fps_does_not_override_functional_result(self):
        self.assertTrue(self.check()['verified']);self.assertIsNone(self.check()['fps_acceptance_threshold'])
    def test_missing_timing_generation_refused(self):
        del self.a['resident']['stage_profile']['samples'][1]
        with self.assertRaisesRegex(ValueError,'Incomplete'):self.check()
    def test_repeated_source_generation_refused(self):
        self.a['resident']['stage_profile']['samples'][2]['generation']='2'
        with self.assertRaisesRegex(ValueError,'Incomplete'):self.check()
    def test_repeated_paints_do_not_supply_source_progress(self):
        for row,_ in self.observed['paints'].values():row['world']='1'
        with self.assertRaisesRegex(ValueError,'Repaint'):self.check()
    def test_pending_queue_refused(self):
        self.a['resident']['stage_profile']['capture']['queue_bytes']=1
        with self.assertRaisesRegex(ValueError,'bound'):self.check()
    def test_queue_cap_refused(self):
        self.a['resident']['stage_profile']['capture']['peak_queue_bytes']=256*1024**2+1
        with self.assertRaisesRegex(ValueError,'bound'):self.check()
    def test_missing_deadline_measurement_refused(self):
        del self.a['resident']['stage_profile']['samples'][2]['producer_wait_ack_ms']
        with self.assertRaisesRegex(ValueError,'measurement'):self.check()
    def test_negative_deadline_measurement_refused(self):
        self.a['resident']['stage_profile']['samples'][2]['native_consume_ms']=-1
        with self.assertRaisesRegex(ValueError,'measurement'):self.check()
    def test_nonfinite_deadline_measurement_refused(self):
        self.a['resident']['stage_profile']['samples'][2]['producer_wait_ack_ms']=float('nan')
        with self.assertRaisesRegex(ValueError,'measurement'):self.check()
    def test_source_deadline_refused(self):
        self.a['resident']['stage_profile']['samples'][-1]['producer_wait_ack_ms']=120000
        with self.assertRaisesRegex(ValueError,'deadline'):self.check()
    def test_widened_storage_deadline_refused(self):
        self.a['configuration']['deadline_ms']=550000
        with self.assertRaisesRegex(ValueError,'deadline'):self.check()

class Cohort(unittest.TestCase):
    def setUp(self):
        spec=importlib.util.spec_from_file_location('current_cohort',ROOT/'tools/close-current-player-validation.py')
        self.c=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.c)
        self.rows=[dict(profile=p,scenario=s,identity=str(i),verified=True,world_extents=[[640,480],[800,600]])
            for i,(p,s)in enumerate([(p,s)for p in ('player-default','player-software')for s in ('normal-quick','normal-storage')]+[(p,'normal-quick')for _ in range(2)for p in ('player-default','player-software')])]
    def test_complete_finite_cohort(self):self.assertTrue(self.c.cohort_domains(self.rows))
    def test_missing_configuration(self):
        with self.assertRaisesRegex(ValueError,'Missing'):self.c.cohort_domains(self.rows[:-1])
    def test_same_actual_process_is_not_two_runs(self):
        self.rows[1]['identity']=self.rows[0]['identity']
        with self.assertRaisesRegex(ValueError,'duplicate'):self.c.cohort_domains(self.rows)
    def test_missing_actual_low_world_refused(self):
        for r in self.rows:r['world_extents']=[[800,600]]
        with self.assertRaisesRegex(ValueError,'extents'):self.c.cohort_domains(self.rows)
    def test_failed_pair_refused(self):
        self.rows[-1]['verified']=False
        with self.assertRaisesRegex(ValueError,'unverified'):self.c.cohort_domains(self.rows)

class LowPreferences(unittest.TestCase):
    def setUp(self):
        import tempfile
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        spec=importlib.util.spec_from_file_location('current_low',ROOT/'tools/run-current-player-low-validation.py')
        self.low=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.low)
        self.path=Path(self.temp.name)/'config/mnm-qt-shell/engine-preferences.json'
    def test_public_resolution_fixture_accepted(self):
        self.low.seed_preferences(self.path,[0,-250,1,0,1,0,0])
        self.assertEqual(self.low.load(self.path)['values'],[0,-250,1,0,1,0,0])
    def test_existing_configuration_preserved(self):
        self.path.parent.mkdir(parents=True);self.path.write_bytes(b'held')
        with self.assertRaisesRegex(ValueError,'Fresh'):self.low.seed_preferences(self.path,[0,-250,1,0,1,0,0])
        self.assertEqual(self.path.read_bytes(),b'held')
    def test_unrelated_preference_change_refused(self):
        with self.assertRaisesRegex(ValueError,'non-resolution'):self.low.seed_preferences(self.path,[1,-250,1,0,1,0,0])
        self.assertFalse(self.path.exists())

if __name__=='__main__':unittest.main()
