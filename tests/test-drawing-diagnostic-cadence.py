from types import SimpleNamespace
from pathlib import Path
import sys
import unittest
import copy
import json
import runpy
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_diagnostic_cadence import DISABLED,validate_control,measurements,compare
from drawing_api_session import sha
executed_schedule=runpy.run_path(str(Path(__file__).resolve().parents[1]/'tools/compare-drawing-diagnostic-cadence.py'))['executed_schedule']


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


class ExecutedSchedule(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory();self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name);self.name='tests/fixtures/drawing-journeys/actual-schedule.json'
        self.actions=[dict(kind='wait',after_ms=8000)]
        path=self.root/self.name;path.parent.mkdir(parents=True);path.write_text(json.dumps(dict(actions=self.actions)))
        self.report=dict(sources={self.name:sha(path.read_bytes())},input_schedule=dict(success=True,error='',actions=[dict(self.actions[0],ordinal=1,start_ms=50,end_ms=8050,presentation_file='physical/input-client-0001.png',synchronized_original_frame=False)]))

    def test_actual_schedule_replaces_fixed_short_schedule(self):
        short='tests/fixtures/drawing-journeys/wine-submission-schedule.json';path=self.root/short
        path.write_text(json.dumps(dict(actions=[dict(kind='wait',after_ms=800)])))
        self.report['sources'][short]=sha(path.read_bytes())
        self.assertEqual(executed_schedule(self.report,self.root)['path'],self.name)

    def test_declared_short_schedule_cannot_mask_changed_executed_actions(self):
        self.report['input_schedule']['actions'][0]['after_ms']=800
        with self.assertRaises(ValueError):executed_schedule(self.report,self.root)

    def test_action_order_and_execution_status_refuse(self):
        for field,value in [('ordinal',2),('start_ms',-1),('end_ms',49),('ordinal',True),('unknown_action_field',1)]:
            report=copy.deepcopy(self.report);report['input_schedule']['actions'][0][field]=value
            with self.assertRaises(ValueError):executed_schedule(report,self.root)
        self.report['input_schedule']['success']=False
        with self.assertRaises(ValueError):executed_schedule(self.report,self.root)

    def test_source_change_missing_and_ambiguous_schedule_refuse(self):
        report=copy.deepcopy(self.report);report['sources']={}
        with self.assertRaises(ValueError):executed_schedule(report,self.root)
        second='tests/fixtures/drawing-journeys/second-schedule.json';path=self.root/second
        path.write_bytes((self.root/self.name).read_bytes());self.report['sources'][second]=sha(path.read_bytes())
        with self.assertRaises(ValueError):executed_schedule(self.report,self.root)
        path.write_text('{}')
        with self.assertRaises(ValueError):executed_schedule(self.report,self.root)


class ResultsKeyPolicy(unittest.TestCase):
    def test_duration_is_explicit_and_invalid_policies_refuse(self):
        from drawing_diagnostic_cadence import results_action
        self.assertEqual(results_action(60),dict(kind='tap',key='Escape',after_ms=600))
        self.assertEqual(results_action(600),dict(kind='hold',key='Escape',duration_ms=600,after_ms=600))
        for bad in (0,59,300,601,True,'600'):
            with self.assertRaises(ValueError):results_action(bad)


if __name__=='__main__':unittest.main()
