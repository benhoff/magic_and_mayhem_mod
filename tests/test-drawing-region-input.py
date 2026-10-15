from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_region_input import decode


def fixture(nested=False):
    count = 4 if nested else 2
    header = [0] * 16
    header[:2] = struct.unpack('<2I', b'MNMDRE01')
    header[2:13] = [1, 256, 0x40209ca7, 8192, count, 0, 18, 0, 0, count // 2, count]
    rows = []
    for i in range(count):
        row = [0] * 64
        row[:8] = [i + 1, i // 2 + 1, 1 + i % 2, i + 1, 100 + i, 18, 1, 127]
        row[8:18] = [400, 265, 0, 1, 0, 1, 0, 0, 0, 0]
        row[20:32] = [3, 1, 2, 7, 1, 1, 8, 3, 27, 0, 1, 100]
        rows.append(row)
    if nested:
        for row, phase in zip(rows, (1, 4, 2, 3)):
            row[3] = phase
            row[4] = 100 + phase
    return bytearray(struct.pack('<16I', *header) + b''.join(struct.pack('<64I', *r) for r in rows))


class RegionInput(unittest.TestCase):
    def test_pair_and_observed_actor_do_not_prove_selection(self):
        result = decode(fixture())
        self.assertEqual(result['calls'][0]['mouse_record'][:2], [400, 265])
        self.assertEqual(result['calls'][0]['returned']['slot_plus_one'], 3)
        self.assertFalse(result['persistent_selection_proved'])
        self.assertFalse(result['tooltip_proved'])

    def test_reserved_loss_thread_argument_and_actor_shape_refuse(self):
        for offset, value in ((28, 1), (36, 1), (40, 1), (60, 1), (64 + 63 * 4, 1),
                              (64 + 256 + 5 * 4, 19), (64 + 256 + 8 * 4, 401),
                              (64 + 26 * 4, 2), (64 + 7 * 4, 255), (64 + 24 * 4, 2)):
            raw = fixture()
            struct.pack_into('<I', raw, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                decode(raw)

    def test_callback_nesting_is_accounted_for(self):
        self.assertEqual(len(decode(fixture(True))['calls']), 2)
        raw = fixture(True)
        # Unique ordered phases can still have crossed returns.
        struct.pack_into('<I', raw, 64 + 256 + 3 * 4, 3)
        struct.pack_into('<I', raw, 64 + 3 * 256 + 3 * 4, 4)
        with self.assertRaises(ValueError):
            decode(raw)

    def test_duplicate_phase_missing_return_and_truncation_refuse(self):
        for raw in (fixture()[:-1], fixture()[:-256]):
            with self.assertRaises(ValueError):
                decode(raw)
        raw = fixture()
        struct.pack_into('<I', raw, 64 + 256 + 3 * 4, 1)
        with self.assertRaises(ValueError):
            decode(raw)

    def test_v1_labels_normalize_without_changing_raw_bytes(self):
        raw = fixture()
        for i in range(2):
            struct.pack_into('<3I', raw, 64 + i * 256 + 28 * 4, 1, 27, 0)
        before = bytes(raw)
        result = decode(raw)
        self.assertTrue(result['legacy_actor_labels_normalized'])
        sample = result['calls'][0]['entry']
        self.assertEqual((sample['actor_type'], sample['actor_owner'], sample['actor_active']), (27, 0, 1))
        self.assertEqual(bytes(raw), before)

    def test_v2_offsets_and_raw_latches_are_separate(self):
        raw = fixture()
        struct.pack_into('<I', raw, 8, 2)
        for i in range(2):
            struct.pack_into('<I', raw, 64 + i * 256 + 7 * 4, 255)
            struct.pack_into('<3I', raw, 64 + i * 256 + 42 * 4, 1, 0, 4)
        result = decode(raw)
        self.assertFalse(result['legacy_actor_labels_normalized'])
        sample = result['calls'][0]['entry']
        self.assertEqual((sample['actor_type'], sample['actor_owner'], sample['actor_active']), (27, 0, 1))
        self.assertEqual(sample['actor_flag_68'], 1)
        self.assertEqual(sample['raw_latch_count'], 4)
        struct.pack_into('<I', raw, 64 + 45 * 4, 1)
        with self.assertRaises(ValueError):
            decode(raw)


if __name__ == '__main__':
    unittest.main()
