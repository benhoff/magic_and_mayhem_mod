from types import SimpleNamespace
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_diagnostic_cadence import DISABLED,validate_control,measurements,compare


def config():
    return SimpleNamespace(cadence_control=True,control=True,engine_cadence=True,
                           observer_mode='metadata',**{k:False for k in DISABLED})


def observation(step=10):
    return dict(success=True,frequency=1000,windows=[dict(sequence=i+1,counter_before=i,
                ticks=[i+1],advancing=True,queues=[{}],qpc=i*step) for i in range(5)])


class Cadence(unittest.TestCase):
    def test_explicit_minimal_control(self):
        validate_control(config())
        for name in ('control','engine_cadence'):
            a=config();setattr(a,name,False)
            with self.assertRaises(ValueError):validate_control(a)
        for name in DISABLED:
            a=config();setattr(a,name,True)
            with self.assertRaises(ValueError):validate_control(a)

    def test_bounded_timing_is_not_equivalence(self):
        r=compare(observation(),observation(20),3)
        self.assertEqual(r['common_queued_updates'],3)
        self.assertEqual(r['median_ratio'],2)
        self.assertFalse(r['identical_gameplay_work_proved'])
        self.assertFalse(r['unobtrusive_capture_proved'])

    def test_wrong_counter_sequence_qpc_and_bounds(self):
        for key,value in [('sequence',9),('counter_before',9),('qpc',0)]:
            a=observation();a['windows'][1][key]=value
            with self.assertRaises(ValueError):measurements(a)
        for limit in (0,121):
            with self.assertRaises(ValueError):measurements(observation(),limit)

    def test_no_queue_and_unequal_lengths(self):
        a=observation();a['windows']=a['windows'][:3]
        self.assertEqual(compare(a,observation())['common_queued_updates'],2)
        for w in a['windows']:w['queues']=[]
        with self.assertRaises(ValueError):measurements(a)


if __name__=='__main__':unittest.main()
