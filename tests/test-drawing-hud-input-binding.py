from pathlib import Path
import copy
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_hud_input_binding import bind


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDAPI1');h[2:9]=[1,128,0x40209ca7,262144,1024,1073741824,2]
    row=[0]*32;row[:3]=[1,2,3];row[6]=18;row[8]=1;row[11]=80;row[27:30]=[400,300,1];row[31]=1
    end=[0]*32;end[:3]=[2,4,1];end[6]=18;end[21]=1
    raw=struct.pack('<16I',*h)+struct.pack('<32I',*row)+struct.pack('<32I',*end)
    point=dict(flags=3,owner_thread=18,record_prefix=1,entry_prefix=1,cursor_record=1,cursor_entry=1,cursor_tick=80,cursor=[400,300],cursor_age_ms=20,current_cursor_admissible=True,timely_cursor=True)
    sample=dict(ordinal=1,tick=100,thread=18,input_point=point,candidate_state=1,actor_slot=0,actor_type=0,actor_owner=0)
    return dict(samples=[sample]),raw


class CursorBinding(unittest.TestCase):
    def test_exact_join_is_not_click_or_selection_proof(self):
        r=bind(*fixture());self.assertEqual(r['linked_samples'],1);self.assertEqual(r['timely_validated_hits'],1);self.assertFalse(r['click_dispatch_proved'])

    def test_wrong_record_entry_tick_owner_coordinates_or_prefix_refuses(self):
        for key,value in [('cursor_record',0),('cursor_entry',2),('cursor_tick',81),('owner_thread',19),('cursor',[401,300]),('record_prefix',3),('entry_prefix',0),('cursor_age_ms',21)]:
            h,raw=fixture();h['samples'][0]['input_point'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):bind(h,raw)

    def test_exact_terminal_prefix_stays_inactive(self):
        h,raw=fixture();p=h['samples'][0]['input_point'];p['record_prefix']=2;p['flags']=2;p['current_cursor_admissible']=False;p['timely_cursor']=False
        r=bind(h,raw);self.assertEqual(r['stopped_linked_samples'],1);self.assertEqual(r['timely_samples'],0)


    def test_missing_getter_remains_excluded(self):
        h,raw=fixture();h['samples'][0]['input_point']=None
        self.assertEqual(bind(h,raw)['refused_points'],1)

    def test_stale_cursor_is_linked_but_not_timely(self):
        h,raw=fixture();h['samples'][0]['tick']=300;p=h['samples'][0]['input_point'];p['cursor_age_ms']=220;p['timely_cursor']=False
        self.assertEqual(bind(h,raw)['timely_samples'],0)


if __name__=='__main__':unittest.main()
