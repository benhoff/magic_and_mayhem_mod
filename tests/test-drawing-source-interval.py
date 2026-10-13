#!/usr/bin/env python3
"""Reject premature interval closure and preserve unique close-request ownership."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_source_interval import poll_source_close


class Process:
    def poll(self):
        return None


def marker(count=32, failures=0):
    return b'MNMPDONE' + struct.pack('<6I', 1, count, 200, failures, 100, 20000)


class Interval(unittest.TestCase):
    def test_absent_and_partial_do_not_close(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.assertIsNone(poll_source_close(root, 32, Process(), lambda: None))
            (root / 'canvas-producers.done').write_bytes(marker()[:16])
            self.assertIsNone(poll_source_close(root, 32, Process(), lambda: None))
            self.assertFalse((root / 'egl-physical-close.flag').exists())

    def test_only_exact_success_closes(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / 'canvas-producers.done').write_bytes(marker())
            result = poll_source_close(root, 32, Process(), lambda: None)
            self.assertEqual(result['source_completion']['expected_queues'], 32)
            self.assertFalse(result['full_source_or_physical_ownership_proved'])
            self.assertEqual((root / 'egl-physical-close.flag').read_bytes(), b'producer-completion-observed\n')

    def test_failed_count_corruption_and_budget_do_not_close(self):
        for data in (marker(16), marker(failures=1), b'INVALID!' + marker()[8:], marker() + b'extra'):
            with tempfile.TemporaryDirectory() as d:
                root = Path(d)
                (root / 'canvas-producers.done').write_bytes(data)
                with self.assertRaises(ValueError):
                    poll_source_close(root, 32, Process(), lambda: None)
                self.assertFalse((root / 'egl-physical-close.flag').exists())

    def test_existing_close_request_is_not_overwritten(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / 'canvas-producers.done').write_bytes(marker())
            close = root / 'egl-physical-close.flag'
            close.write_bytes(b'prior-owner\n')
            with self.assertRaises(FileExistsError):
                poll_source_close(root, 32, Process(), lambda: None)
            self.assertEqual(close.read_bytes(), b'prior-owner\n')


if __name__ == '__main__':
    unittest.main()
