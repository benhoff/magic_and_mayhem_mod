"""Poll actual owned admission without an accessibility RPC during Launch."""
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import rendering_transition_live as base
from rendering_transition_low_live import TransitionJourney
from fixture_support import menu_publication, owned_menu_session


class Startup(unittest.TestCase):
    def test_pending_launch_does_not_query_busy_qt_thread(self):
        with tempfile.TemporaryDirectory() as directory:
            journey = object.__new__(TransitionJourney)
            journey.out = Path(directory)
            journey.session = None
            journey.epoch_roots = []
            journey.current_epoch = 0
            class BusyQt:
                def nodes(self):
                    raise AssertionError('AT-SPI RPC during synchronous Launch')
            journey.access = BusyQt()
            self.assertIsNone(journey.read())

    def test_only_owned_stable_original_publication_is_ready(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            session = owned_menu_session(root)
            journey = object.__new__(TransitionJourney)
            journey.out = root
            journey.session = None
            journey.epoch_roots = []
            with patch.object(base, 'ROOT', root):
                self.assertIsNone(journey.read())
                raw = menu_publication(sequence=3)
                (session / 'channel.bin').write_bytes(raw)
                self.assertIsNone(journey.read())  # Odd seqlock is unpublished.
                struct.pack_into('<I', raw, 128, 4)
                (session / 'channel.bin').write_bytes(raw)
                value = journey.read()
                self.assertTrue(value[0]['ready'])
                self.assertEqual((value[0]['screen'], value[0]['thread']), (3, 99))

    def test_unowned_or_duplicate_admissions_refuse(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            admissions = root / 'owner/sessions'
            admissions.mkdir(parents=True)
            path = admissions / 'one.json'
            path.write_text(json.dumps({'session': str(root / 'elsewhere')}))
            journey = object.__new__(TransitionJourney)
            journey.out = root
            journey.session = None
            journey.epoch_roots = []
            with patch.object(base, 'ROOT', root), self.assertRaisesRegex(ValueError, 'Unowned'):
                journey.read()
            session = root / 'working/experiments/menu-observer/run-test'
            path.write_text(json.dumps({'session': str(session)}))
            (admissions / 'two.json').write_bytes(path.read_bytes())
            with patch.object(base, 'ROOT', root), self.assertRaisesRegex(ValueError, 'Unexpected original launch count'):
                journey.read()

    def test_terminal_original_start_failure_is_not_a_readiness_timeout(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            session = owned_menu_session(root)
            journey = object.__new__(TransitionJourney)
            journey.out = root
            journey.session = None
            journey.epoch_roots = []
            native = dict(session=str(session), state='failed', error='producer GAP reason 2')
            path = session / 'native-only-session.json'
            path.write_text(json.dumps(native))
            with patch.object(base, 'ROOT', root), self.assertRaisesRegex(ValueError, 'producer GAP reason 2'):
                journey.read()
            native['session'] = str(root / 'other')
            path.write_text(json.dumps(native))
            with patch.object(base, 'ROOT', root), self.assertRaisesRegex(ValueError, 'another session'):
                journey.read()


if __name__ == '__main__':
    unittest.main()
