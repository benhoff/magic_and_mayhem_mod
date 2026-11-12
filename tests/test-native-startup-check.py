"""Capture-free diagnostic pointer must not require a captured paint record."""
import importlib.util
from pathlib import Path
from types import SimpleNamespace
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('startup_check', ROOT / 'tools/run-native-startup-check.py')
startup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(startup)


class Pointer(unittest.TestCase):
    def journey(self):
        calls = []
        j = object.__new__(startup.StartupJourney)
        j.viewport = SimpleNamespace(poll=lambda: dict(world='9', image_width=1060,
            image_height=795, left=0, top=1))
        node = dict(accessible_id='app.nativeGameViewport', showing=True, rect=[20, 32, 1060, 796])
        j.access = SimpleNamespace(nodes=lambda: [(None, node)])
        j.input = SimpleNamespace(display=7,
            xt=SimpleNamespace(XTestFakeMotionEvent=lambda *args: calls.append(args) or 1),
            x=SimpleNamespace(XFlush=lambda display: None))
        j.actions = []
        return j, calls, node

    def test_capture_free_confirmation_point_uses_observed_input_geometry(self):
        j, calls, _ = self.journey()
        j.pointer([212, 316])
        self.assertEqual(calls, [(7, -1, 371, 557, 0)])
        self.assertEqual(j.actions[0]['original_generation'], '9')
        self.assertNotIn('paint', j.actions[0])
        self.assertEqual(j.actions[0]['mapping'], 'native-input-extent/Sharp-no-paint-witness')

    def test_invalid_point_or_hidden_viewport_sends_no_pointer(self):
        j, calls, node = self.journey()
        with self.assertRaises(ValueError):
            j.pointer([640, 100])
        node['showing'] = False
        with self.assertRaises(ValueError):
            j.pointer([212, 316])
        self.assertFalse(calls)


if __name__ == '__main__':
    unittest.main()
