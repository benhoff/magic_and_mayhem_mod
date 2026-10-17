#!/usr/bin/env python3
"""Cadence clocks, phase boundaries, stalls and World/presentation separation."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_cadence import analyze_cadence, CLOCK


class CadenceTests(unittest.TestCase):
    def fixture(self):
        inputs=[];rows=[]
        for cycle, start in enumerate([100.,130.]):
            intervals=[dict(begin=start,end=start+2),dict(begin=start+12,end=start+14)]
            inputs.append(dict(success=True,host_clock=CLOCK,monotonic_start_seconds=start,seconds=14,
                               camera_dialogue=dict(intervals=intervals,overhead_seconds=4)))
            for index in range(14):
                rows.append(dict(cycle=cycle,host_clock=CLOCK,host_begin_us=str(int((start+index)*1e6)),
                                 host_end_us=str(int((start+index+1)*1e6)),seconds=1.,
                                 world_imports_begin=str(index*6),world_imports_end=str((index+1)*6),
                                 world_imports=6,native_frames=40,painted_frames=30))
        return dict(gameplay_rates=rows),inputs

    def test_counts_worlds_separately_and_keeps_menu_time(self):
        flow,inputs=self.fixture();r=analyze_cadence(flow,inputs,10,5)
        self.assertEqual(r['between_stress_phases_seconds'],16)
        for phase in r['phases']:
            self.assertEqual(phase['active_sample_seconds'],10)
            self.assertEqual(phase['rates'],dict(world_imports_fps=6,native_frames_fps=40,painted_frames_fps=30))
            self.assertEqual(phase['readiness_windows'],4)
        self.assertTrue(r['whole_journey_metric_unchanged'])

    def test_cached_paints_cannot_pass_world_floor(self):
        flow,inputs=self.fixture()
        for row in flow['gameplay_rates']:
            row.update(world_imports_begin='0',world_imports_end='0',world_imports=0)
        r=analyze_cadence(flow,inputs,10,5)
        self.assertTrue(all(p['meets_paint_floor'] and not p['meets_world_floor'] for p in r['phases']))

    def test_wrong_clocks_counts_and_intervals_refuse(self):
        mutations=[lambda f,i:f['gameplay_rates'][0].update(host_clock='other'),
                   lambda f,i:i[0].update(host_clock='other'),
                   lambda f,i:f['gameplay_rates'][3].update(world_imports=7),
                   lambda f,i:f['gameplay_rates'][3].update(world_imports_begin='0'),
                   lambda f,i:f['gameplay_rates'][3].update(world_imports=True),
                   lambda f,i:f['gameplay_rates'][3].update(seconds=float('nan')),
                   lambda f,i:i[0]['camera_dialogue'].update(overhead_seconds=3),
                   lambda f,i:i[0]['camera_dialogue']['intervals'][1].update(begin=101),
                   lambda f,i:i[0]['camera_dialogue']['intervals'][0].update(begin=99),
                   lambda f,i:f['gameplay_rates'].pop(4),
                   lambda f,i:f['gameplay_rates'].append(copy.deepcopy(f['gameplay_rates'][2]))]
        for mutate in mutations:
            flow,inputs=self.fixture();mutate(flow,inputs)
            with self.subTest(mutate=mutate),self.assertRaises(ValueError):analyze_cadence(flow,inputs,10,5)

    def test_insufficient_active_windows_and_false_targets_refuse(self):
        flow,inputs=self.fixture()
        with self.assertRaises(ValueError):analyze_cadence(flow,inputs,11,5)
        for target in [True,float('inf'),float('nan'),0]:
            with self.subTest(target=target),self.assertRaises(ValueError):analyze_cadence(flow,inputs,10,target)

    def test_long_stalls_are_counted_and_lower_the_rate(self):
        flow,inputs=self.fixture()
        # Extend one active window and everything after it, keeping complete
        # counters. It remains in the denominator instead of being discarded.
        for row in flow['gameplay_rates']:
            if row['cycle']!=0:continue
            lo=int(row['host_begin_us'])/1e6;hi=int(row['host_end_us'])/1e6
            if lo>=106:row['host_begin_us']=str(int((lo+4)*1e6))
            if hi>=106:row['host_end_us']=str(int((hi+4)*1e6))
            if lo==105:row['seconds']=5
        inputs[0]['seconds']=18
        inputs[0]['camera_dialogue']['intervals'][1]=dict(begin=116.,end=118.)
        r=analyze_cadence(flow,inputs,10,5)['phases'][0]
        self.assertEqual(r['long_active_windows'],1)
        self.assertAlmostEqual(r['rates']['world_imports_fps'],60/14)
        self.assertFalse(r['meets_world_floor'])


if __name__=='__main__':unittest.main()
