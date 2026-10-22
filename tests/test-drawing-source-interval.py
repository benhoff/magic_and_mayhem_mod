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


    def test_symlink_fifo_and_boolean_queue_do_not_close(self):
        import os
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);target=root/'real';target.write_bytes(marker())
            path=root/'canvas-producers.done';path.symlink_to(target)
            with self.assertRaises(OSError):poll_source_close(root,32,Process(),lambda:None)
            path.unlink();os.mkfifo(path)
            with self.assertRaises(ValueError):poll_source_close(root,32,Process(),lambda:None)
            path.unlink();path.write_bytes(marker(1))
            with self.assertRaises(ValueError):poll_source_close(root,True,Process(),lambda:None)
            self.assertFalse((root/'egl-physical-close.flag').exists())

    def test_hash_is_of_the_single_validated_snapshot(self):
        from drawing_api_session import sha
        from unittest.mock import patch
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            with patch('drawing_source_interval.regular_bytes',return_value=marker()) as read:
                result=poll_source_close(root,32,Process(),lambda:None)
            read.assert_called_once_with(root/'canvas-producers.done',32)
            self.assertEqual(result['source_marker_sha256'],sha(marker()))

if __name__ == '__main__':
    unittest.main()
