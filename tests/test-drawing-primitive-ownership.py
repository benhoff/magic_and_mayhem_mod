import copy
import importlib.util
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_primitive_ownership import join
from drawing_update_accounting import build
spec=importlib.util.spec_from_file_location('accounting_fixture',Path(__file__).with_name('test-drawing-update-accounting.py'))
fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)


def inputs():
    a=fixture.fixture();a[2]['points'][1]['pending_frames']=1
    a[7][0]['points'][2]['source'][3]=10
    a[1]['calls'][0]['complete_qpc']=210
    accounting=build(*a)
    b=[dict(cadence_record=p['cadence_record'],sequence=p['sequence'],producer_prefix=p['producer_prefix'],queue_prefix=p['queue_prefix'],pending=p['pending_frames'],calls=[91] if p['pending_frames'] else []) for p in a[2]['points']]
    l=dict(success=True,frequency=1000,producer_packets_crosschecked=True,source_callbacks_crosschecked=True,boundaries=b,pending_boundary_calls=[dict(id=91,tag=32,original_execution_envelope_qpc=[115,220])])
    return accounting,l


class Ownership(unittest.TestCase):
    def test_ack_late_and_original_return_remain_distinct_without_gate_promotion(self):
        a,l=inputs();g=copy.deepcopy(a['gates']);r=join(a,l)
        self.assertEqual(r['crossings_after_original_return'],1);self.assertEqual(r['pending_tag_counts'],{'32':1})
        self.assertEqual(r['original_admission_gates'],g);self.assertEqual(r['requested_windows'],120)
        self.assertFalse(r['complete_presented_interval']);self.assertFalse(r['source_bytes_immutable'])
    def test_execution_still_active_is_not_labelled_only_late_ack(self):
        a,l=inputs();a['original_call_intervals'][0]['return_qpc']=205
        self.assertEqual(join(a,l)['crossings_after_original_return'],0)
    def test_unlinked_source_gap_stays_explicit(self):
        a,l=inputs();a['source_point_gaps'].append(dict(sequence=1,entry=999,reason='unknown_source'))
        self.assertEqual(join(a,l)['unclassified_source_gaps'][0]['entry'],999)
    def test_unbound_clock_denominator_pending_or_identity_refuses(self):
        for kind in ('clock','unbound','denominator','boundary','missing_call','duplicate_call'):
            a,l=inputs()
            if kind=='clock':l['frequency']=999
            if kind=='unbound':l['producer_packets_crosschecked']=False
            if kind=='denominator':a['requested_windows']=119
            if kind=='boundary':l['boundaries'][1]['producer_prefix']+=1
            if kind=='missing_call':l['pending_boundary_calls']=[]
            if kind=='duplicate_call':l['boundaries'][1]['calls']=[91,91]
            with self.subTest(kind=kind),self.assertRaises(ValueError):join(a,l)


if __name__=='__main__':unittest.main()
