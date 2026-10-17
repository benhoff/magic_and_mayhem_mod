#!/usr/bin/env python3
"""Guard explicit physical interval and command joins against false accounting."""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_egl_source_domains import account


def fixture():
    r = dict(egl_physical_observation=dict(frames=[dict(entry=i, pid=9) for i in (1, 2, 3)]),
             observation=dict(calls=[dict(entry=i, method=7, flags=16) for i in (1, 2, 3, 4)]),
             wine_submission_observation=dict(calls=[dict(call=i, api_entry=i, source_rect=[0, 0, 800, 600], destination_rect=[0, 0, 800, 600]) for i in (1, 2, 3, 4)]),
             wine_blit_packet_observation=dict(bindings=[dict(command=i, submission=i, worker=i, worker_thread=7) for i in (1, 2, 3, 4)]),
             api_source_lease_observation=dict(calls=[dict(api_entry=1, lease=1, before=[0]*16, begin_result=0x80000001), dict(api_entry=2, lease=0, before=[0]*16, begin_result=3)]),
             cursor_source_gate_binding=dict(refused=[dict(api_entry=3, reason=3)]),
             egl_worker_context_observation=dict(calls=[dict(call=i, pid=9, stable_active_context=True, points=[dict(context=dict(command=i, submission=i, worker=i, win_thread=7))]) for i in (1, 2, 3, 4)]))
    return r, b'MNMEGD01' + struct.pack('<6I', 1, 9, 3, 3, 1, 0)


class Guards(unittest.TestCase):
    def test_domains_and_after_close_are_distinct(self):
        r, h = fixture()
        a = account(r, h)
        self.assertEqual(a['closed_physical_calls'], 3)
        self.assertEqual(len(a['outside_closed_interval']), 1)
        self.assertEqual(a['domain_counts'], {'accepted-readonly-source-lease': 1, 'refused-source-domain': 1, 'cursor-gate-refused-before-source-lease': 1})
        self.assertFalse(a['source_byte_ownership_proved'])

    def test_missing_witness_inside_close_refuses(self):
        r, h = fixture()
        r['egl_physical_observation']['frames'].pop(1)
        h = b'MNMEGD01' + struct.pack('<6I', 1, 9, 2, 3, 1, 0)
        with self.assertRaisesRegex(ValueError, 'Missing physical witness'):
            account(r, h)

    def test_duplicate_or_changed_command_refuses(self):
        for duplicate in (True, False):
            r, h = fixture()
            if duplicate:
                r['wine_blit_packet_observation']['bindings'].append(r['wine_blit_packet_observation']['bindings'][0])
            else:
                r['wine_blit_packet_observation']['bindings'][0]['worker_thread'] = 8
            with self.assertRaises(ValueError):
                account(r, h)

    def test_wrong_process_and_missing_context_refuse(self):
        for process in (True, False):
            r, h = fixture()
            if process:
                r['egl_physical_observation']['frames'][0]['pid'] = 10
            else:
                r['egl_worker_context_observation']['calls'].pop(1)
            with self.assertRaises(ValueError):
                account(r, h)


if __name__ == '__main__':
    unittest.main()
