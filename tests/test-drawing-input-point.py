from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_input_point import decode,validate_cadence_mode


def fixture():return bytearray(struct.pack('<16I',1,64,0x40209ca7,3,18,10,12,2,0,0,0,8,9,500,400,300))


class InputPoint(unittest.TestCase):
    def test_exact_cursor_identity_is_not_click_proof(self):
        r=decode(fixture());self.assertTrue(r['current_cursor_admissible']);self.assertEqual(r['cursor_record'],8);self.assertFalse(r['click_dispatch_proved'])

    def test_cadence_metadata_exception_needs_all_original_cursor_flags(self):
        from types import SimpleNamespace
        args=SimpleNamespace(engine_cadence=True,cadence_control=False,control=False,observer_mode='metadata',hud_input_point=True,hud_state=True,semantic_state=True)
        validate_cadence_mode(args)
        for key in ('hud_input_point','hud_state','semantic_state'):
            bad=SimpleNamespace(**vars(args));setattr(bad,key,False)
            with self.subTest(key=key),self.assertRaises(ValueError):validate_cadence_mode(bad)
        args.control=True
        with self.assertRaises(ValueError):validate_cadence_mode(args)


    def test_wrong_version_owner_flag_and_future_record_refuse(self):
        for index,value in [(0,2),(1,60),(3,4),(4,0),(11,11),(12,13),(5,262145),(7,1025)]:
            raw=fixture();struct.pack_into('<I',raw,index*4,value)
            with self.subTest(index=index),self.assertRaises(ValueError):decode(raw)

    def test_stopped_lost_and_gapped_points_are_not_current_cursor(self):
        for index,value in [(3,2),(8,1),(9,1)]:
            raw=fixture();struct.pack_into('<I',raw,index*4,value);self.assertFalse(decode(raw)['current_cursor_admissible'])

    def test_failed_or_absent_cursor_cannot_retain_valid_coordinates(self):
        raw=fixture();struct.pack_into('<I',raw,12,1)
        with self.assertRaises(ValueError):decode(raw)
        struct.pack_into('<2I',raw,56,0,0);self.assertIsNone(decode(raw)['cursor'])


if __name__=='__main__':unittest.main()
