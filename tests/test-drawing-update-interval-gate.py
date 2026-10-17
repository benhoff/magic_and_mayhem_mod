import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('controls',ROOT/'tests/test-drawing-update-accounting.py');controls=importlib.util.module_from_spec(spec);spec.loader.exec_module(controls)
spec=importlib.util.spec_from_file_location('gate',ROOT/'tools/check-drawing-update-interval.py');gate=importlib.util.module_from_spec(spec);spec.loader.exec_module(gate)


def fixture():
    value=controls.m.build(*controls.fixture())
    value.update(analysis_completed=True,sources_stable=True,raw_observations_match_original_report=True,acquisition_observers_match_captured_sources=True)
    return value


class Gate(unittest.TestCase):
    def test_valid_accounting_refuses_unknown_global_ownership(self):
        r=gate.validate_accounting(fixture());self.assertTrue(r['success']);self.assertFalse(r['admitted']);self.assertEqual(r['source_queues'],119);self.assertEqual(r['queue_free_updates'],[96])
    def test_queue_free_window_and_denominator_cannot_be_dropped(self):
        for kind in ('window','count','queue'):
            v=fixture()
            if kind=='window':v['windows'].pop(95)
            if kind=='count':v['requested_windows']=119
            if kind=='queue':v['source_queues']=120
            with self.assertRaises(ValueError):gate.validate_accounting(v)
    def test_gate_claim_cannot_hide_pending_source_or_point_gap(self):
        for kind in ('pending','points','egl','pixels'):
            v=fixture()
            if kind=='pending':v['windows'][1]['source_begin']['pending_frames']=1
            if kind=='points':v['source_point_gaps']=[{'entry':1}]
            if kind=='egl':v['missing_primary_egl_links']=[{'entry':1}]
            if kind=='pixels':v['reference_pixel_gaps']=[{'entry':1}]
            with self.assertRaises(ValueError):gate.validate_accounting(v)
    def test_source_counter_time_and_frame_identity_mutations_refuse(self):
        for kind in ('prefix','counter','time','frames','calls'):
            v=fixture()
            if kind=='prefix':v['windows'][10]['producer_records']+=1
            if kind=='counter':v['windows'][10]['counter_before']+=2
            if kind=='time':v['windows'][10]['begin_qpc']+=1
            if kind=='frames':v['physical_primary_witnesses']-=1
            if kind=='calls':v['original_call_intervals'].pop()
            with self.assertRaises(ValueError):gate.validate_accounting(v)
    def test_unsupported_global_proof_and_complete_promotion_refuse(self):
        v=fixture();v['gates']['global_presentation_routes_closed']=True
        with self.assertRaises(ValueError):gate.validate_accounting(v)
        v=fixture();v['complete_presented_interval']=True
        with self.assertRaises(ValueError):gate.validate_accounting(v)
    def test_unknown_missing_and_nonboolean_gates_refuse(self):
        for kind in ('unknown','missing','scalar'):
            v=fixture()
            if kind=='unknown':v['gates']['optional_override']=True
            if kind=='missing':v['gates'].pop('global_presentation_routes_closed')
            if kind=='scalar':v['gates']['consecutive_advancing_windows']=1
            with self.assertRaises(ValueError):gate.validate_accounting(v)


if __name__=='__main__':unittest.main()
