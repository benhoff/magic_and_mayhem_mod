import importlib.util
from pathlib import Path
import struct
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_action_session import frame_epoch,action_epochs,worker_handoff
from drawing_storage_capture import storage_rows,channel_bytes,drive


def frame(before,after,ordinal=1,entry=8):
    return b'MNMDAF01'+struct.pack('<30I',1,128,ordinal,entry,*before,*after,0,0)


class Cases(unittest.TestCase):
    def setUp(self):
        self.zero=(0,)*12;self.enter=(1,1,29,17,3,0,1,0,7,4,0,0);self.done=(2,1,29,17,3,0,2,1,9,5,1,2);self.states={0:self.zero,1:self.enter,2:self.done}
    def test_worker_handoff_requires_fresh_completed_display_action(self):
        good=dict(request=4,phase=2,action=19,status=1,epoch=8)
        self.assertEqual(worker_handoff([good],4,9,set()),8)
        for events,request,epoch,used in [([],4,9,set()),([good],3,9,set()),([good],4,7,set()),([good],4,9,{4}),([good,good],4,9,set())]:
            with self.assertRaises(ValueError):worker_handoff(events,request,epoch,used)
        for key,value in [('phase',1),('action',18),('status',2)]:
            with self.assertRaises(ValueError):worker_handoff([dict(good,**{key:value})],4,9,set())
    def test_action_states_are_derived_from_paired_actual_journal(self):
        header=bytearray(64);header[:8]=b'MNMDAPI1';struct.pack_into('<7I',header,8,1,128,0x40209ca7,262144,1024,1073741824,1)
        rows=[]
        for phase in (1,2):
            r=[0]*32;r[0]=phase;r[1]=3;r[2]=phase;r[6]=5;r[21:28]=[4,19,10,7,0,phase,phase==2];r[29]=phase;rows.append(struct.pack('<32I',*r))
        terminal=[0]*32;terminal[0]=3;terminal[1]=4;terminal[2]=2;terminal[6]=5;terminal[21]=2
        journal=bytes(header)+b''.join(rows)+struct.pack('<32I',*terminal)
        states,events=action_epochs(journal)
        self.assertEqual(states[1],(1,4,19,10,7,0,1,0,1,0,0,0));self.assertEqual(states[2],(2,4,19,10,7,0,2,1,2,0,4,2));self.assertEqual(len(events),2)
        changed=bytearray(journal);struct.pack_into('<I',changed,64+128+24*4,8)
        with self.assertRaises(ValueError):action_epochs(changed)
    def test_stable_before_action(self):self.assertTrue(frame_epoch(frame(self.zero,self.zero),1,8,self.states)['stable_completed_epoch'])
    def test_stable_completed_action(self):self.assertEqual(frame_epoch(frame(self.done,self.done),1,8,self.states)['before_completed_request'],1)
    def test_in_flight_not_relabelled(self):
        result=frame_epoch(frame(self.enter,self.done),1,8,self.states);self.assertTrue(result['callback_in_flight']);self.assertTrue(result['action_changed_during_swap']);self.assertFalse(result['stable_completed_epoch'])
    def test_complete_action_between_swap_brackets_retained(self):self.assertTrue(frame_epoch(frame(self.zero,self.done),1,8,self.states)['action_changed_during_swap'])
    def test_wrong_frame_or_call_refuses(self):
        for ordinal,entry in [(2,8),(1,9)]:
            with self.assertRaises(ValueError):frame_epoch(frame(self.done,self.done),ordinal,entry,self.states)
    def test_reversed_epoch_refuses(self):
        with self.assertRaises(ValueError):frame_epoch(frame(self.done,self.enter),1,8,self.states)
    def test_unbound_or_mutated_state_refuses(self):
        for index in range(12):
            changed=list(self.done);changed[index]+=1
            with self.assertRaises(ValueError):frame_epoch(frame(changed,self.done),1,8,self.states)
    def test_extent_version_and_reserved_refuse(self):
        good=frame(self.zero,self.zero)
        for raw in [good[:-1],good+b'\0',good[:8]+struct.pack('<I',2)+good[12:],good[:-4]+struct.pack('<I',1)]:
            with self.assertRaises(ValueError):frame_epoch(raw,1,8,self.states)
    def test_variable_egl_extent_admission(self):
        from drawing_egl_physical import decode_request
        for width,height in ((640,480),(800,600)):
            raw=b'MNMEGL02'+struct.pack('<30I',2,128,1,3,4,5,11,1,2,1,1,0,width,height,10,0,*([0]*14))
            self.assertEqual(decode_request(raw)[14:16],(width,height))
        for width,height in ((0,480),(640,0),(2049,600)):
            raw=b'MNMEGL02'+struct.pack('<30I',2,128,1,3,4,5,11,1,2,1,1,0,width,height,10,0,*([0]*14))
            with self.assertRaises(ValueError):decode_request(raw)
    def test_channel_matches_current_wire_header(self):
        import re
        spec=(ROOT/'protocols/include/mnm/menu_v22.h').read_text();size=int(re.search(r'#define MNM_MENU_V22_SIZE (\d+)',spec)[1]);raw=channel_bytes()
        self.assertEqual(len(raw),size);self.assertEqual(raw[:8],b'MNMMCM22');self.assertEqual(struct.unpack_from('<5I',raw,8),(22,size,2,1,1));self.assertEqual(struct.unpack_from('<8I',raw,128),(0,)*8)
    def test_invalid_dwell_refuses_before_any_inputs(self):
        for count in (0,2,17,True,3.0):
            with self.assertRaises(ValueError):drive(*([None]*10),initial_worlds=count)
    def test_storage_rows_use_exact_count(self):
        raw=bytearray(196608);struct.pack_into('<8I',raw,110592,12,17,1,0,0,0,2,2);raw[110752:110756]=b'one\0';raw[110880:110884]=b'two\0';self.assertEqual(storage_rows(raw),['one','two'])
    def test_invalid_storage_count_or_screen_refuses(self):
        for screen,count,selected in [(0,0,0),(12,101,0),(12,1,2)]:
            raw=bytearray(196608);struct.pack_into('<8I',raw,110592,screen,17,1,0,0,0,count,selected)
            with self.assertRaises(ValueError):storage_rows(raw)
if __name__=='__main__':unittest.main()
