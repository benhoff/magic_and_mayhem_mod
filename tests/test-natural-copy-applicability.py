"""Post-retirement collector changes cannot hide observation/native drift."""
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from natural_copy_applicability import collector_review

OLD=b'''policy = 7
def observe(): return policy
def main():
    journey.run()
    require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit did not retire normally')
    if child.poll() is None:
        journey.input.close_window()
    require(child.wait(timeout=10) == 0, 'Native host failed')
    retain()
'''
NEW=OLD.replace(b"    if child.poll() is None:\n        journey.input.close_window()\n    require(child.wait(timeout=10) == 0, 'Native host failed')",b'    close_after_retirement(child,journey.input)')+b'\ndef close_after_retirement(child,input): return None\n'


class Review(unittest.TestCase):
    def test_only_guarded_post_retirement_change_is_preserved(self):
        self.assertTrue(collector_review(OLD,NEW)['verified'])

    def test_changed_observation_configuration_or_initialization_refuses(self):
        for changed in (NEW.replace(b'journey.run()',b'journey.inject()'),NEW.replace(b'policy = 7',b'policy = 8'),
                        NEW.replace(b'return policy',b'return 0'),NEW+b'other_policy=1\n'):
            with self.assertRaises(ValueError):collector_review(OLD,changed)

    def test_missing_guard_or_extra_close_refuses(self):
        for changed in (NEW.replace(b"'Original Quit did not retire normally'",b"'unchecked'"),
                        NEW.replace(b'    retain()',b'    close_after_retirement(child,journey.input)\n    retain()')):
            with self.assertRaises(ValueError):collector_review(OLD,changed)

    def test_helper_phase_is_exact(self):
        with self.assertRaises(ValueError):collector_review(OLD,NEW.replace(b'def close_after_retirement(',b'def other_helper('))


if __name__=='__main__':unittest.main()
