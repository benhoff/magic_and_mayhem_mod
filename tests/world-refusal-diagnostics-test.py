"""Stopped sparse journal fixtures; no original artifacts or rendering required."""
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import world_channel
import world_refusal


class DiagnosticsTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root/'capture').mkdir()
        (self.root/'native-world.json').write_text(json.dumps({'producer_reason': 8, 'presentations': 1}))
        world_channel.create(self.root/'world-channel.bin', True, 16)
        self.patch(20, struct.pack('<I', 3))
        self.patch(28, struct.pack('<I', 12))
        self.patch(40, struct.pack('<I', 8))
        self.input_at = 128 + 12*(32 + world_channel.INPUT + world_channel.ORACLE) + 32
        failed = bytearray(80)
        failed[:8] = b'MNMWRLD1'
        struct.pack_into('<4I', failed, 8, 1, 80, 80, 13)
        struct.pack_into('<3I', failed, 36, 0, 8, world_channel.BUILD)
        self.patch(self.input_at, failed)
        for sequence in range(1, 14):
            self.queue(sequence, [9, 17, 0xffffffff] if sequence == 13 else [0])

    def patch(self, offset, data):
        with (self.root/'world-channel.bin').open('r+b') as channel:
            channel.seek(offset)
            channel.write(data)

    def queue(self, sequence, kinds):
        raw = bytearray(64 + len(kinds)*36)
        raw[:8] = b'MNMSTQ01'
        struct.pack_into('<14I', raw, 8, 1, 64, len(raw), sequence, 0, 0, 0,
                         len(kinds), len(kinds), 0x11110000, 0x22220000, 36, len(kinds), 0)
        for ordinal, kind in enumerate(kinds):
            struct.pack_into('<9I', raw, 64 + ordinal*36, 3, 0x12340000, 12,
                             0xfffffffd, 0, 0, kind, 0, 0)
        (self.root/'capture'/f'startup-queue-{sequence:04}.bin').write_bytes(raw)

    def test_partial_prefix_correlates_the_producer_ahead_of_viewer(self):
        result = world_refusal.save(self.root)
        self.assertEqual((result['failed_source_queue'], result['published_frames'], result['presented_frames']), (13, 12, 1))
        self.assertFalse(result['complete_prefix'])
        self.assertEqual([(d['ordinal'], d['kind'], d['y']) for d in result['refused_queue']['unsupported_draws']], [(0, 9, -3), (2, -1, -3)])
        self.assertIn('source queue 13', world_refusal.summarize(result))
        self.assertEqual(json.loads((self.root/'native-world-refusal.json').read_text()), json.loads(json.dumps(result)))

    def test_complete_journal_does_not_assert_complete_rendering(self):
        for sequence in range(14, 17):
            self.queue(sequence, [0])
        result = world_refusal.diagnose(self.root)
        self.assertTrue(result['complete_prefix'])
        self.assertEqual(result['presented_frames'], 1)

    def test_ended_channel_retains_failure_reason(self):
        self.patch(20, struct.pack('<2I', 2, 1))
        self.assertEqual(world_refusal.diagnose(self.root)['failed_source_queue'], 13)

    def test_missing_journal_has_no_invented_kind(self):
        (self.root/'capture/startup-queue-0013.bin').unlink()
        with self.assertRaisesRegex(ValueError, 'cannot be recovered'):
            world_refusal.diagnose(self.root)

    def test_gap_is_refused(self):
        (self.root/'capture/startup-queue-0004.bin').unlink()
        self.queue(14, [0])
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_admitted_wave_cannot_explain_kind_refusal(self):
        self.queue(13, [16, 17, 20])
        with self.assertRaisesRegex(ValueError, 'unsupported raw draw'):
            world_refusal.diagnose(self.root)

    def test_failed_packet_must_match_published_boundary(self):
        self.patch(self.input_at + 20, struct.pack('<I', 12))
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_partial_raster_cannot_be_a_kind_refusal(self):
        self.patch(self.input_at + 36, struct.pack('<I', 1))
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_wrong_build_refused(self):
        self.patch(64, struct.pack('<I', 1))
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_active_producer_is_not_a_stopped_fixture(self):
        self.patch(20, struct.pack('<I', 1))
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_malformed_queue_refused(self):
        self.queue(13, [9])
        path = self.root/'capture/startup-queue-0013.bin'
        path.write_bytes(path.read_bytes()[:-1])
        with self.assertRaises(ValueError):
            world_refusal.diagnose(self.root)

    def test_other_failure_not_mislabeled(self):
        (self.root/'native-world.json').write_text(json.dumps({'producer_reason': 7}))
        self.assertIsNone(world_refusal.diagnose(self.root))


class StagingTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.source = self.root/'source'
        self.source.mkdir()
        (self.source/'CFG').mkdir()
        (self.source/'Chaos.exe').write_bytes(b'unchanged executable')
        (self.source/'CFG/prefs.cfg').write_bytes(b'unchanged preferences')
        spec = importlib.util.spec_from_file_location('guarded_staging', ROOT/'tools/prepare-scene-observer.py')
        self.staging = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.staging)

    def test_directory_link_becomes_an_independent_copy(self):
        alias = self.root/'alias'
        alias.symlink_to(self.source, target_is_directory=True)
        game = self.root/'game'
        self.staging.copy_installation(alias, game)
        self.assertFalse(game.is_symlink())
        (game/'Chaos.exe').write_bytes(b'staged executable')
        (game/'CFG/prefs.cfg').write_bytes(b'staged preferences')
        self.assertEqual((self.source/'Chaos.exe').read_bytes(), b'unchanged executable')
        self.assertEqual((self.source/'CFG/prefs.cfg').read_bytes(), b'unchanged preferences')

    def test_existing_destination_link_is_refused_before_copy(self):
        game = self.root/'game'
        game.symlink_to(self.source, target_is_directory=True)
        with self.assertRaises(ValueError):
            self.staging.copy_installation(self.source, game)
        self.assertFalse((self.source/'source').exists())

    def test_external_executable_link_is_refused_before_patch(self):
        outside = self.root/'outside.exe'
        outside.write_bytes(b'untouched external executable')
        (self.source/'Chaos.exe').unlink()
        (self.source/'Chaos.exe').symlink_to(outside)
        with self.assertRaises(ValueError):
            self.staging.copy_installation(self.source, self.root/'game')
        self.assertEqual(outside.read_bytes(), b'untouched external executable')

    def test_external_preferences_link_is_refused_before_configuration(self):
        outside = self.root/'outside.cfg'
        outside.write_bytes(b'untouched external preferences')
        (self.source/'CFG/prefs.cfg').unlink()
        (self.source/'CFG/prefs.cfg').symlink_to(outside)
        with self.assertRaises(ValueError):
            self.staging.copy_installation(self.source, self.root/'game')
        self.assertEqual(outside.read_bytes(), b'untouched external preferences')


if __name__ == '__main__':
    unittest.main()
