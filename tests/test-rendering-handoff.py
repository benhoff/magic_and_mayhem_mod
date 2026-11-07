from copy import deepcopy
from contextlib import closing
from pathlib import Path
import importlib.util
import struct
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_handoff import classify, mapped_identity, menu_header, rebuilt_frame

HISTORICAL = 'research/runtime/rendering-picking-resize-results-20261107/default-06-acquisition.json'


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            a = r.json(HISTORICAL)
            v = next(v for v in a['artifacts'] if v['path'].endswith('run-_lu37sg3/channel.bin'))
            cls.raw = r.read_bytes(v['record'])[:160]

    def test_actual_terminal_header(self):
        w = menu_header(self.raw)
        self.assertEqual((w['request'], w['ack'], w['screen'], w['action']), (18, 17, 7, 12))

    def test_torn_or_unknown_wire(self):
        for raw in (self.raw[:-4], b'badwire!'+self.raw[8:]):
            with self.assertRaises(ValueError):
                menu_header(raw)
        raw = bytearray(self.raw)
        struct.pack_into('<I', raw, 128, 4899)
        self.assertIsNone(menu_header(raw))

    def observations(self):
        w = menu_header(self.raw)
        return [dict(w, alive=1, heartbeat=i+1, ns=1_000_000_000+i*100_000_000) for i in range(31)]

    def test_stalled_engine_with_live_host(self):
        q = classify(self.observations())
        self.assertTrue(q['engine_freeze_observed'])
        self.assertEqual(q['freeze_seconds'], 3)

    def test_acknowledged_or_wrong_action_is_not_pending_finish(self):
        for change in (dict(ack=18), dict(action=1), dict(screen=14), dict(alive=0)):
            rows = [dict(r, **change) for r in self.observations()]
            self.assertFalse(classify(rows)['pending_finish_observed'])

    def test_stopped_host_is_not_live_engine_freeze(self):
        rows = [dict(r, heartbeat=1) for r in self.observations()]
        self.assertFalse(classify(rows)['engine_freeze_observed'])

    def test_fresh_engine_publications_reset_interval(self):
        rows = [dict(r, engine_sequence=i*2+2) for i, r in enumerate(self.observations())]
        self.assertFalse(classify(rows)['engine_freeze_observed'])

    def test_namespace_identity_uses_device_inode(self):
        import os
        maps='00400000-00500000 rw-s 00000000 103:02 9876 /hidden/channel.bin\n'
        self.assertTrue(mapped_identity(maps, (os.makedev(0x103, 2), 9876)))
        self.assertFalse(mapped_identity(maps, (os.makedev(0x103, 2), 9877)))
        self.assertFalse(mapped_identity(maps, (os.makedev(0x103, 3), 9876)))
        self.assertFalse(mapped_identity('bad map line', (1, 9876)))

    def test_rebuild_wait_ignores_old_frame_and_requires_new_progress(self):
        frames=[None, dict(frame_width=800,frame_height=600,world='100'),
                dict(frame_width=800,frame_height=600,world='106'),
                dict(frame_width=640,frame_height=480,world='100'),
                dict(frame_width=640,frame_height=480,world='106')]
        self.assertEqual([rebuilt_frame(f,[640,480],100) for f in frames],
                         [False,False,False,False,True])

    def test_actual_retirement_precedes_ten_second_request_deadline(self):
        import sqlite3
        import tempfile
        with Reader(ROOT) as r:
            a=r.json('research/runtime/rendering-handoff-results-20261107/low-01-acquisition.json')
            v=next(v for v in a['artifacts'] if Path(v['path']).name=='handoff.sqlite3')
            raw=r.read_bytes(v['record'])
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/'observations.sqlite3';path.write_bytes(raw)
            with closing(sqlite3.connect(path)) as db:
                rows=[dict(menu_header(m),ns=ns) for ns,m in db.execute('SELECT ns,menu FROM samples ORDER BY ordinal')]
        result=classify(rows)
        self.assertTrue(result['pending_finish_observed'])
        self.assertLess(result['request_to_retirement_upper_seconds'], 2)
        self.assertFalse(result['engine_freeze_observed'])

    def test_retained_original_failure_stays_failed(self):
        spec = importlib.util.spec_from_file_location('handoff_check', ROOT/'tools/check-rendering-handoff.py')
        m = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(m)
        with Reader(ROOT) as r:
            q = m.analyze(r, HISTORICAL)
        self.assertEqual(q['shell_state']['state'], 'failed')
        self.assertFalse(q['diagnostic_cause_proved'])
        self.assertFalse(q['normal_acceptance'])
        self.assertEqual(q['last_callback']['screen'], 7)


if __name__ == '__main__':
    unittest.main()
