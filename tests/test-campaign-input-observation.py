import pathlib
import struct
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'tools'))
from campaign_input_observation import decode


class ObservationTest(unittest.TestCase):
    def trace(self, rows):
        return b'MNMCAI01' + struct.pack('<II', 1, 128) + b''.join(struct.pack('<32I', *r) for r in rows)

    def row(self):
        return [1, 100, 312, 0, 31, 3, 1, 1, 0, 16, 2, 309, 3, 2448, 1,
                440, 310, 2, 0x80000001, 800, 600, 100, 20] + [0] * 9

    def test_observation_is_explicitly_not_click_or_cast_proof(self):
        d = decode(self.trace([self.row()]))
        self.assertTrue(d['prefix_only'])
        self.assertFalse(d['click_delivery_proved'])
        self.assertFalse(d['cast_admission_proved'])
        self.assertEqual(d['rows'][0]['selected_slot'], 1)
        self.assertEqual(d['rows'][0]['right'], 0x80000001)

    def test_invalid_snapshot_thread_reserved_and_overflow_refuse(self):
        for index, value in [(13, 2449), (2, 0), (23, 1), (4, 0x8000001f), (7, 257)]:
            row = self.row()
            row[index] = value
            with self.assertRaises(ValueError):
                decode(self.trace([row]))

    def test_unreadable_fields_cannot_carry_data(self):
        for flag in (1, 2, 4, 8):
            row = self.row()
            row[4] &= ~flag
            with self.assertRaises(ValueError):
                decode(self.trace([row]))

    def test_missing_sequence_foreign_owner_and_backwards_clock_refuse(self):
        for index, value in [(0, 3), (2, 313), (1, 99)]:
            first = self.row()
            second = self.row()
            second[0] = 2
            second[index] = value
            with self.assertRaises(ValueError):
                decode(self.trace([first, second]))

    def test_clock_wrap_is_allowed(self):
        first = self.row()
        first[1] = 0xfffffff0
        second = self.row()
        second[0] = 2
        self.assertEqual(len(decode(self.trace([first, second]))['rows']), 2)

    def test_partial_and_unknown_envelopes_refuse(self):
        data = self.trace([self.row()])
        for bad in (data[:-1], b'', b'wrong' + data[5:], data[:8] + struct.pack('<II', 3, 128) + data[16:]):
            with self.assertRaises(ValueError):
                decode(bad)

    def test_v2_retains_actual_callback_arguments_and_signed_ground(self):
        row = self.row()
        row[3] = 2
        row[4] = 127
        row[23:29] = [0xffffffff, 71, 4, 96, 96, 8]
        row[29:] = [0x40, 440 | (309 << 16), 10]
        data = b'MNMCAI01' + struct.pack('<II', 2, 128) + struct.pack('<32I', *row)
        sample = decode(data)['rows'][0]
        self.assertEqual(sample['ground'], [-1, 71, 4])
        self.assertEqual(sample['mouse'], [440, 309])
        self.assertEqual(sample['mouse_flags'], 10)
        row[4] &= ~32
        with self.assertRaises(ValueError):
            decode(b'MNMCAI01' + struct.pack('<II', 2, 128) + struct.pack('<32I', *row))

    def v3(self,rows):
        return b'MNMCAI01'+struct.pack('<II',3,144)+b''.join(struct.pack('<36I',*r) for r in rows)

    def test_v3_distinguishes_world_returns_from_the_simulation_clock(self):
        a=self.row()+[10,0,0,0];a[3]=1;a[4]|=128
        b=a.copy();b[0]=2;b[1]+=1;b[32]=11
        rows=decode(self.v3([a,b]))['rows']
        self.assertEqual([r['world_counter'] for r in rows],[100,100])
        self.assertEqual([r['world_return_ordinal'] for r in rows],[10,11])

    def test_v3_requires_token_flag_reserved_zero_and_forward_order(self):
        a=self.row()+[10,0,0,0];a[3]=1;a[4]|=128
        for index,value in ((4,31),(33,1),(34,1),(35,1)):
            bad=a.copy();bad[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):decode(self.v3([bad]))
        b=a.copy();b[0]=2;b[32]=9
        with self.assertRaises(ValueError):decode(self.v3([a,b]))

    def test_v3_return_ordinal_wrap_is_forward(self):
        a=self.row()+[0xffffffff,0,0,0];a[3]=1;a[4]|=128
        b=a.copy();b[0]=2;b[32]=0
        self.assertEqual(decode(self.v3([a,b]))['rows'][-1]['world_return_ordinal'],0)


if __name__ == '__main__':
    unittest.main()
