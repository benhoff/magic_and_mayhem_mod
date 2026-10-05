#!/usr/bin/env python3
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('stream_rate', REPO/'tools/profile-render-stream.py')
rate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rate)


class StreamRateTest(unittest.TestCase):
    def test_busy_invalid_and_missed_polls(self):
        header = bytearray(rate.wire.initial_header())
        struct.pack_into('<I', header, rate.wire.FRAME_COUNT_OFFSET, 10)
        window = rate.RateWindow(10)
        window.observe(*rate.sample(header))
        struct.pack_into('<I', header, rate.wire.SEQUENCE_OFFSET, 1)
        self.assertEqual(rate.sample(header), ('busy', None, None))
        window.observe(*rate.sample(header))
        struct.pack_into('<I', header, rate.wire.SEQUENCE_OFFSET, 2)
        struct.pack_into('<I', header, rate.wire.FRAME_COUNT_OFFSET, 17)
        window.observe(*rate.sample(header))
        header[0] = 0
        window.observe(*rate.sample(header))
        row = window.report(10.5)
        self.assertEqual((row['published_updates'], row['published_updates_per_second']), (7, 14))
        self.assertEqual((row['busy_samples'], row['invalid_samples'], row['samples']), (1, 1, 4))
        window.observe('stable', 18, 1)
        self.assertEqual(window.report(11)['published_updates'], 1)

    def test_counter_wrap(self):
        window = rate.RateWindow(0)
        window.observe('stable', 0xfffffffe, 1)
        window.observe('stable', 2, 1)
        self.assertEqual(window.report(1)['published_updates'], 4)

    def test_read_only_cli(self):
        with tempfile.TemporaryDirectory() as directory:
            stream = Path(directory)/'frame.bin'
            output = Path(directory)/'rate.jsonl'
            with stream.open('wb') as file:
                file.write(rate.wire.initial_header())
                file.truncate(rate.wire.SIZE)
            before = stream.stat().st_mtime_ns
            subprocess.run([sys.executable, str(REPO/'tools/profile-render-stream.py'), str(stream),
                            '--output', str(output), '--seconds', '.05'], check=True, timeout=5)
            rows = [json.loads(line) for line in output.read_text().splitlines()]
            self.assertEqual(len(rows), 1)
            self.assertGreater(rows[0]['samples'], 0)
            self.assertEqual(rows[0]['published_updates'], 0)
            self.assertEqual(stream.stat().st_mtime_ns, before)


if __name__ == '__main__':
    unittest.main()
