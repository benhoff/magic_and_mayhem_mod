from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_windows import bind


class Windows(unittest.TestCase):
    def setUp(self):
        self.windows=[dict(sequence=1,qpc=100,counter_before=5,begin_record=2,queues=[dict(record=4)],advancing=True),dict(sequence=2,qpc=200,counter_before=6,begin_record=5,queues=[],advancing=True)]
        self.samples=[dict(submission=1,thread=7,status=0,qpc_before=120,qpc_after=180),dict(submission=2,thread=8,status=0,qpc_before=200,qpc_after=280)]
    def run_model(self):return bind(self.samples,self.windows,300,7)
    def test_exact_containment_and_foreign_thread_are_separate(self):
        r=self.run_model();self.assertEqual([x['sequence'] for x in r['assignments']],[1,2]);self.assertEqual([x['captured_on_counter_owner'] for x in r['assignments']],[True,False]);self.assertEqual(r['assignments'][0]['observed_queue_records'],[4]);self.assertFalse(r['producer_identity_bound']);self.assertFalse(r['all_presentations_in_update_interval_retained'])
    def test_crossing_and_equal_end_boundary_are_retained(self):
        for end in (200,210):
            self.samples[0]['qpc_after']=end;r=self.run_model();self.assertEqual(r['crossing_samples'],1);self.assertEqual(r['assigned_samples'],1);self.assertFalse(r['all_samples_contained'])
    def test_outside_both_sides_and_last_boundary(self):
        self.samples[0].update(qpc_before=50,qpc_after=99);self.samples[1].update(qpc_before=300,qpc_after=310);r=self.run_model();self.assertEqual(r['outside_samples'],2);self.assertEqual(r['assigned_samples'],0)
    def test_bad_clock_identity_or_boundary_refuses(self):
        for value in (None,0,True,181,2**64):
            old=self.samples[0]['qpc_before'];self.samples[0]['qpc_before']=value
            with self.assertRaises(ValueError):self.run_model()
            self.samples[0]['qpc_before']=old
        self.samples.append(dict(self.samples[0]))
        with self.assertRaises(ValueError):self.run_model()
        self.samples.pop();self.windows[1]['qpc']=100
        with self.assertRaises(ValueError):self.run_model()
    def test_refused_source_is_not_a_valid_pixel_claim(self):
        self.samples[0]['status']=5;r=self.run_model();self.assertEqual(r['assignments'][0]['status'],5);self.assertFalse(r['worker_identity_bound'])


if __name__=='__main__':unittest.main()
