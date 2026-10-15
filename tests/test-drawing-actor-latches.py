from pathlib import Path
import copy
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_actor_latches import decode, bind_callbacks


def fixture():
    h = [0] * 16
    h[:2] = struct.unpack('<2I', b'MNMDLA01')
    h[2:11] = [1, 256, 0x40209ca7, 2048, 2, 0, 18, 0, 50]
    rows = []
    for kind in (1, 2):
        r = [0] * 64
        r[:16] = [kind, 100 + kind, 18, 2, kind, 1, kind, 11, 1, 3, 3, 1, 0, 1, 1, 3]
        r[16], r[24] = 7, 2
        r[56:] = [1, 2, 27, 0, 1, 100, 1, 0]
        rows.append(r)
    return bytearray(struct.pack('<16I', *h) + b''.join(struct.pack('<64I', *r) for r in rows))


def region():
    return dict(owner=18, calls=[dict(call=1, mask=1, mouse_record=[400, 265] + [0] * 8,
        entry=dict(phase_ordinal=1, tick=101), returned=dict(phase_ordinal=2, tick=102))])


class ActorLatches(unittest.TestCase):
    def test_latched_actor_is_independent_of_hit(self):
        result = decode(fixture())
        self.assertEqual(result['samples'][1]['hit_slot'], 2)
        self.assertEqual(result['samples'][1]['raw_68_slots'], [1])
        self.assertEqual(result['samples'][1]['first_latched_actor']['slot'], 1)
        self.assertFalse(result['atomic_pool_snapshot_proved'])
        self.assertFalse(result['actor_generations_proved'])

    def test_loss_owner_flags_capacity_bitmap_counts_refuse(self):
        for offset, value in ((28, 1), (36, 1), (44, 1), (64 + 2 * 4, 19),
                              (64 + 7 * 4, 3), (64 + 9 * 4, 257), (64 + 16 * 4, 3),
                              (64 + 24 * 4, 8), (64 + 57 * 4, 3), (64 + 62 * 4, 0),
                              (64 + 40 * 4, 1), (64 + 56 * 4, 0)):
            raw = fixture()
            struct.pack_into('<I', raw, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                decode(raw)
        for raw in (fixture()[:-1], fixture()[:-256]):
            with self.assertRaises(ValueError):
                decode(raw)

    def test_unvalidated_pool_preserves_refusal_without_actor_data(self):
        raw = fixture()
        for at in (64, 320):
            r = list(struct.unpack_from('<64I', raw, at))
            r[7] = 2
            r[8:13] = [0] * 5
            r[15:40] = [0] * 25
            r[56:] = [0] * 8
            struct.pack_into('<64I', raw, at, *r)
        self.assertEqual(decode(raw)['samples'][0]['raw_68_slots'], [])
        struct.pack_into('<I', raw, 64 + 57 * 4, 2)
        with self.assertRaises(ValueError):
            decode(raw)

    def test_exact_phase_join_rejects_missing_duplicate_wrong_owner(self):
        result = decode(fixture())
        self.assertEqual(len(bind_callbacks(result, region())['phases']), 2)
        for mutate in ('missing', 'duplicate', 'phase', 'time'):
            bad = copy.deepcopy(result)
            if mutate == 'missing': bad['samples'].pop()
            if mutate == 'duplicate': bad['samples'].append(bad['samples'][0])
            if mutate == 'phase': bad['samples'][1]['phase_ordinal'] = 4
            if mutate == 'time': bad['samples'][1]['tick'] = 0
            with self.subTest(mutate=mutate), self.assertRaises(ValueError):
                bind_callbacks(bad, region())
        bad = region(); bad['owner'] = 19
        with self.assertRaises(ValueError): bind_callbacks(result, bad)

    def test_equal_callback_ticks_allowed_but_periodic_cadence_is_required(self):
        raw = fixture(); struct.pack_into('<I', raw, 320 + 4, 101)
        self.assertEqual(len(decode(raw)['samples']), 2)
        for at in (64, 320):
            struct.pack_into('<3I', raw, at + 4 * 4, 0, 0, 0)
        with self.assertRaises(ValueError): decode(raw)


if __name__ == '__main__': unittest.main()
