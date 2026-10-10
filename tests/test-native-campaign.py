#!/usr/bin/env python3
"""Fail-closed evidence checks for the public native campaign smoke runner."""
import copy
import importlib.util
from pathlib import Path
import struct
import tempfile
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('campaign_smoke', ROOT / 'tools/test-native-campaign.py')
smoke = importlib.util.module_from_spec(spec)
spec.loader.exec_module(smoke)


class EvidenceTests(unittest.TestCase):
    def flow(self):
        return {'success': True, 'native_command_fallback': False, 'native_command_frames': 6,
                'steps': [{'step': s, 'screenshot_saved': True} for s in smoke.STEPS]}

    def test_complete_flow(self):
        smoke.validate_flow(self.flow())

    def test_original_window_or_missing_native_frames_cannot_pass(self):
        for updates in [{'success': False}, {'native_command_fallback': True},
                        {'native_command_fallback': None}, {'native_command_frames': 0},
                        {'native_command_frames': True}, {'native_command_frames': 5}]:
            with self.subTest(updates=updates):
                flow = self.flow(); flow.update(updates)
                with self.assertRaises(ValueError):
                    smoke.validate_flow(flow)

    def test_partial_flow_or_missing_screenshot_cannot_pass(self):
        for mutate in [lambda f: f['steps'].pop(), lambda f: f['steps'].reverse(),
                       lambda f: f['steps'][0].update(screenshot_saved=False)]:
            flow = self.flow(); mutate(flow)
            with self.assertRaises(ValueError):
                smoke.validate_flow(flow)

    def events(self, root):
        rows = []
        def event(kind, screen, arg=0, initialized=1, active=0x6cbb78):
            row = [0]*16
            row[0], row[1], row[2], row[3] = len(rows)+1, kind, 400, screen
            row[6], row[9], row[11] = initialized, arg, active
            rows.append(row)
        event(3, 3); event(3, 18)
        for _ in range(3):
            event(12, 2)
        event(3, 17, 4); event(20, 2)
        channel = bytearray(106496)
        channel[:16] = b'MNMMCM12' + struct.pack('<II', 12, len(channel))
        struct.pack_into('<I', channel, 36*4, 3)
        (root / 'channel.bin').write_bytes(channel)
        return rows

    def write_events(self, root, rows):
        (root / 'events.bin').write_bytes(b'MNMMENU1' + struct.pack('<II', 1, 64) +
                                         b''.join(struct.pack('<16I', *r) for r in rows))

    def test_independent_gameplay_and_resume_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); rows = self.events(root); self.write_events(root, rows)
            self.assertEqual(smoke.validate_events(root)['world_ticks'], 3)
            for kind in ['no_resume', 'wrong_thread', 'missing_action', 'before_cancel', 'bad_sequence']:
                changed = copy.deepcopy(rows)
                if kind == 'no_resume': changed.pop()
                elif kind == 'wrong_thread': changed[-1][2] = 401
                elif kind == 'missing_action': changed[0][1] = 2
                elif kind == 'before_cancel': changed[-1][0] = changed[-2][0]-1
                else: changed[0][0] = 2
                self.write_events(root, changed)
                with self.subTest(kind=kind), self.assertRaises(ValueError):
                    smoke.validate_events(root)

    def test_startup_frames_do_not_authorize_gameplay(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);rows=self.events(root)
            self.write_events(root,[r for r in rows if r[1]!=12])
            self.assertFalse(smoke.world_ready(root))
            self.write_events(root,rows)
            self.assertTrue(smoke.world_ready(root))
            rows[3][2]=401;self.write_events(root,rows)
            self.assertFalse(smoke.world_ready(root))

    def test_slow_or_missing_paints_fail_gameplay(self):
        flow=self.flow()
        flow['gameplay_rates']=[dict(cycle=c,seconds=1,native_frames=30,painted_frames=30,native_fps=30,paint_fps=30) for c in [0,1] for _ in range(10)]
        smoke.validate_flow(flow,stress_seconds=10)
        for field,value in [('painted_frames',1),('native_frames',1),('seconds',3)]:
            slow=copy.deepcopy(flow)
            for row in slow['gameplay_rates']:row[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):smoke.validate_flow(slow,stress_seconds=10)
        flow['gameplay_rates']=[]
        with self.assertRaises(ValueError):smoke.validate_flow(flow,stress_seconds=10)

    def test_protocol_downgrade_cannot_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); rows = self.events(root); self.write_events(root, rows)
            channel = bytearray((root / 'channel.bin').read_bytes())
            channel[:8] = b'MNMMCMD8'; (root / 'channel.bin').write_bytes(channel)
            with self.assertRaises(ValueError): smoke.validate_events(root)


if __name__ == '__main__':
    unittest.main()
