from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_cursor_mutex_binding import analyze


def fixture():
    api_h = [0] * 16
    api_h[:2] = struct.unpack('<2I', b'MNMDAPI1')
    api_h[2:9] = [1, 128, 0x40209ca7, 262144, 1024, 1073741824, 1]
    cursor = [0] * 32
    cursor[:3] = [1, 2, 3]
    cursor[6] = 18
    cursor[8] = 1
    cursor[11] = 90
    cursor[27:30] = [400, 265, 1]
    cursor[31] = 1
    terminal = [0] * 32
    terminal[:3] = [2, 4, 1]
    terminal[6] = 18
    terminal[21] = 1
    api = bytearray(struct.pack('<16I', *api_h) + struct.pack('<32I', *cursor) + struct.pack('<32I', *terminal))
    mutex_h = [0] * 16
    mutex_h[:2] = struct.unpack('<2I', b'MNMDMX01')
    mutex_h[2:12] = [1, 128, 0x40209ca7, 65536, 2, 0, 0, 1, 0, 0]
    rows = []
    for phase in (1, 2):
        row = [0] * 32
        row[:11] = [phase, 1, 2, phase, 18, 100 + phase, 0xa494e, 0, 10, 0, 0]
        row[15] = 1
        row[16:] = [1, 64, 0x40209ca7, 3 if phase == 1 else 2, 18, phase, 1, 0, 0, 0, 0, 1, 1, 90, 400, 265]
        rows.append(row)
    mutex = bytearray(struct.pack('<16I', *mutex_h) + b''.join(struct.pack('<32I', *r) for r in rows))
    return mutex, api


class MutexBinding(unittest.TestCase):
    def test_exact_cursor_and_inactive_terminal_prefix(self):
        result = analyze(*fixture())
        self.assertEqual(result['calls'], 1)
        self.assertEqual(result['inactive_points'], 1)
        self.assertFalse(result['safe_source_synchronization_proved'])

    def test_wrong_record_entry_tick_coordinates_and_loss_refuse(self):
        for word, value in ((5, 3), (6, 0), (8, 1), (11, 2), (12, 2), (13, 91), (14, 401)):
            mutex, api = fixture()
            struct.pack_into('<I', mutex, 64 + (16 + word) * 4, value)
            with self.subTest(word=word), self.assertRaises(ValueError):
                analyze(mutex, api)

    def test_query_refusal_stays_visible(self):
        mutex, api = fixture()
        struct.pack_into('<I', mutex, 64 + 15 * 4, 2)
        struct.pack_into('<16I', mutex, 64 + 16 * 4, *([0] * 16))
        self.assertEqual(analyze(mutex, api)['query_refusals'], 1)


if __name__ == '__main__':
    unittest.main()
