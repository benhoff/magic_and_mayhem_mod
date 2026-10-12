from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_semantic_state import decode,menu_interval


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDSS01');h[2:10]=[1,192,0x40209ca7,4096,3,0,18,50]
    rows=[]
    for i,tick in enumerate((100,400,700),1):
        r=[0]*48;r[:18]=[i,tick,18,17,2,3,1,1,63,5,17,1,40,40,2,100,100,9];r[18:42]=[1]*24;rows.append(r)
    return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in rows))


class SemanticState(unittest.TestCase):
    def test_stable_menu_interval_is_not_full_pause_equivalence(self):
        r=decode(fixture());intervals=menu_interval(r,17)
        self.assertEqual(len(intervals),1);self.assertTrue(all(intervals[0]['stable'].values()))
        self.assertFalse(r['pause_equivalence']);self.assertFalse(intervals[0]['allanimation_pause_proved'])

    def test_changed_counter_is_visible(self):
        raw=fixture();struct.pack_into('<I',raw,64+192+12*4,41)
        self.assertFalse(menu_interval(decode(raw),17)[0]['stable']['world_counter'])

    def test_wrong_count_loss_thread_cadence_or_reserve_refuses(self):
        for offset,value in [(24,2),(28,1),(64+2*4,19),(64+192+4,120),(64+42*4,1)]:
            raw=fixture();struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(raw)

    def test_invalid_state_cannot_authorize_menu_interval(self):
        raw=fixture()
        for i in range(3):
            at=64+i*192;struct.pack_into('<I',raw,at+8*4,0);struct.pack_into('<33I',raw,at+9*4,*([0]*33))
        self.assertEqual(menu_interval(decode(raw),17),[])


if __name__=='__main__':unittest.main()
