import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from rendering_interaction_stability import ready_generation


class Readiness(unittest.TestCase):
    def setUp(self):
        self.paint = dict(world='5', native_gpu_frame=True, gpu_read_complete=True,
                          pixels_captured=True, presentation_owner='gui')
        self.row = dict(kind=1, frame=dict(generation=5), tick=100,
                        cursor=dict(point=[40, 100], tick=90),
                        hit=dict(slot=0, type=0, owner=0, active=1, health=400))

    def ready(self, hover=True):
        return ready_generation([self.row], self.paint, [40, 100], hover, 4)

    def test_completed_original_point_and_display(self):
        self.assertEqual(self.ready(), 5)
        self.row['hit'] = None
        self.assertEqual(self.ready(False), 5)
        self.assertIsNone(self.ready())

    def test_no_unpresented_or_foreign_provider_frame(self):
        for key, value in [('native_gpu_frame', False), ('gpu_read_complete', False),
                           ('pixels_captured', False), ('presentation_owner', 'worker')]:
            with self.subTest(key=key):
                original = self.paint[key];self.paint[key] = value
                self.assertIsNone(self.ready());self.paint[key] = original

    def test_generation_must_match_and_follow_action(self):
        self.row['frame']['generation'] = 6
        self.assertIsNone(self.ready())
        self.row['frame']['generation'] = 5
        self.assertIsNone(ready_generation([self.row], self.paint, [40, 100], True, 5))

    def test_point_and_freshness_are_required(self):
        self.row['cursor']['point'] = [41, 100]
        self.assertIsNone(self.ready(False))
        self.row['cursor']['point'] = [40, 100];self.row['cursor']['tick'] = (100 - 501) % 2**32
        self.assertIsNone(self.ready(False))

    def test_hover_role_must_be_original_living_wizard(self):
        actor = copy.copy(self.row['hit'])
        for key, value in [('slot', 1), ('type', 1), ('owner', 1), ('active', 0), ('health', 0)]:
            with self.subTest(key=key):
                self.row['hit'] = dict(actor, **{key: value});self.assertIsNone(self.ready())


if __name__ == '__main__':
    unittest.main()
