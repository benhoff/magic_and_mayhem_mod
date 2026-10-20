#!/usr/bin/env python3
"""Exercise concurrent private X servers and owned cleanup on error."""
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import sys
import tempfile
from threading import Barrier
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from terrain_validation_display import private_display


class PrivateDisplayTest(unittest.TestCase):
    def test_concurrent_servers_get_distinct_displays_and_cleanup(self):
        with tempfile.TemporaryDirectory() as temporary:
            barrier = Barrier(3)

            def own(index):
                with private_display(Path(temporary) / f'{index}.log') as env:
                    display = env['DISPLAY']
                    self.assertNotIn('XAUTHORITY', env)
                    barrier.wait(timeout=40)
                    return display

            with ThreadPoolExecutor(max_workers=3) as pool:
                displays = list(pool.map(own, range(3)))
            self.assertEqual(len(set(displays)), 3)
            for display in displays:
                self.assertFalse(Path('/tmp/.X11-unix/X' + display[1:]).exists())

    def test_body_failure_releases_the_owned_server(self):
        with tempfile.TemporaryDirectory() as temporary:
            display = None
            with self.assertRaisesRegex(RuntimeError, 'controlled failure'):
                with private_display(Path(temporary) / 'failure.log') as env:
                    display = env['DISPLAY']
                    raise RuntimeError('controlled failure')
            self.assertFalse(Path('/tmp/.X11-unix/X' + display[1:]).exists())


if __name__ == '__main__':
    unittest.main()
