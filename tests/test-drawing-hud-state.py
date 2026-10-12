from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_hud_state import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDHI01');h[2:10]=[1,128,0x40209ca7,4096,2,0,18,50]
    rows=[]
    for i in range(2):
        r=[0]*32;r[:22]=[i+1,100+i*50,18,2,15,2,4,1,8,3,14,1,1,100,2,40,30,0,4,5,0,1];rows.append(r)
    return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<32I',*r) for r in rows))


class HUDState(unittest.TestCase):
    def test_valid_candidate_is_not_selection_or_lifetime_proof(self):
        r=decode(fixture());self.assertEqual(r['samples'][0]['actor_slot'],2)
        self.assertEqual(r['samples'][0]['camera_whole'],[40,30,0])
        self.assertFalse(r['selection_verified']);self.assertFalse(r['actor_generation_proved'])

    def test_loss_thread_cadence_reserved_and_pool_shape_refuse(self):
        for offset,value in [(28,1),(64+8,19),(64+128+4,120),(64+22*4,1),(64+8*4,257),(64+9*4,9),(64+7*4,2)]:
            raw=fixture();struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(raw)

    def test_unvalidated_pointer_cannot_be_actor(self):
        raw=fixture();struct.pack_into('<I',raw,64+4*4,11)
        with self.assertRaises(ValueError):decode(raw)
        struct.pack_into('<5I',raw,64+9*4,*([0]*5));struct.pack_into('<I',raw,64+21*4,2)
        self.assertIsNone(decode(raw)['samples'][0]['actor_slot'])

    def test_numeric_pool_change_is_visible_without_generation_claim(self):
        raw=fixture();struct.pack_into('<I',raw,64+128+7*4,2)
        self.assertEqual(decode(raw)['samples'][1]['observed_pool_id'],2)


if __name__=='__main__':unittest.main()
