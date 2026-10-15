from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_cursor_mutex import decode


def fixture():
    h = [0] * 16
    h[:2] = struct.unpack('<2I', b'MNMDMX01')
    # Create, wait, release, failed close, successful close, reused handle,
    # successful close, untracked-handle wait.
    specs = [(1, 0, 1, 0), (2, 1, 0, 10), (3, 1, 1, 0), (4, 1, 0, 0),
             (4, 1, 1, 0), (1, 0, 1, 1), (4, 2, 1, 0), (2, 0, 0, 10)]
    h[2:12] = [1, 128, 0x40209ca7, 65536, 16, 0, 0, 8, 0, 2]
    rows = []
    for i, (op, identity, result, parameter) in enumerate(specs, 1):
        for phase in (1, 2):
            row = [0] * 32
            row[:11] = [len(rows) + 1, i, op, phase, 18, 100 + len(rows), 0xa494e, identity, parameter, 0 if phase == 1 else result, 0xabc]
            if op == 1 and phase == 2:
                row[7] = 1 if i == 1 else 2
            row[15] = 1
            row[16:32] = [1, 64, 0x40209ca7, 1, 18, 20, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            rows.append(row)
    return bytearray(struct.pack('<16I', *h) + b''.join(struct.pack('<32I', *r) for r in rows))


class CursorMutex(unittest.TestCase):
    def test_returns_can_interleave(self):
        raw = fixture()
        rows = [list(r) for r in struct.iter_unpack('<32I', raw[64:])]
        # Move the release entry between the wait entry/return, without retiring
        # the mutex or pretending that return order is entry order.
        rows[2:6] = [rows[2], rows[4], rows[3], rows[5]]
        for i, row in enumerate(rows, 1):
            row[0] = i
            row[5] = 100 + i
        raw[64:] = b''.join(struct.pack('<32I', *r) for r in rows)
        self.assertEqual(len(decode(raw)['calls']), 8)

    def test_failed_close_and_handle_reuse_keep_distinct_ids(self):
        result = decode(fixture())
        self.assertEqual(result['created_identities'], 2)
        self.assertEqual(result['remaining_live_ids'], [])
        self.assertEqual(result['calls'][5]['returned']['mutex_id'], 2)
        self.assertFalse(result['real_mutex_ownership_proved'])
        self.assertFalse(result['safe_source_synchronization_proved'])

    def test_unknown_handle_is_visible(self):
        self.assertEqual(decode(fixture())['calls'][-1]['entry']['mutex_id'], 0)

    def test_drops_pending_duplicate_and_argument_changes_refuse(self):
        for offset, value in ((28, 1), (32, 1), (40, 1), (64, 2),
                              (64 + 3 * 128 + 8 * 4, 11), (64 + 16 * 4 + 4 * 4, 19)):
            raw = fixture()
            struct.pack_into('<I', raw, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                decode(raw)

    def test_stale_identity_and_unsupported_envelope_refuse(self):
        raw = fixture()
        struct.pack_into('<I', raw, 64 + 14 * 128 + 7 * 4, 1)
        with self.assertRaises(ValueError):
            decode(raw)
        for raw in (fixture()[:-1], fixture()[:-128]):
            with self.assertRaises(ValueError):
                decode(raw)


if __name__ == '__main__':
    unittest.main()
