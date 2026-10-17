import copy
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_primitive_lifetimes import decode, MAX_ROWS


def fixture():
    header=[0]*16;header[2:8]=[1,256,0x40209ca7,MAX_ROWS,1000,0]
    rows=[];active={};prefix=0
    def row(phase,identity=0,thread=7,tag=100,parent=0,record=0,accepted=0):
        nonlocal prefix
        if phase==1:active[identity]=1
        if phase==4:del active[identity]
        r=[0]*64;r[0:6]=[len(rows)+1,phase,identity,thread,tag if identity else 0,0x401234 if identity else 0]
        r[6]=r[0]*10;r[8]=prefix;r[10]=parent;r[13]=len(active)
        if record:r[24:48]=record;r[59]=1
        if phase in (5,6):r[49:51]=[accepted*10,accepted]
        rows.append(r)
    row(1,1);row(2,1)
    request=[0]*24;request[0]=96;request[2]=5;request[3]=1
    row(1,2,tag=10,parent=1);row(2,2,tag=10,parent=1,record=request)
    row(5,accepted=1);row(6,1,accepted=1);row(6,2,tag=10,parent=1,record=request,accepted=1)
    row(3,2,tag=10,parent=1,record=request);prefix=1;request[1]=1
    row(4,2,tag=10,parent=1,record=request)
    row(3,1);row(4,1)
    row(1,3,thread=8,tag=32);row(2,3,thread=8,tag=32)
    row(5,accepted=2);row(6,3,thread=8,tag=32,accepted=2)
    row(3,3,thread=8,tag=32);row(4,3,thread=8,tag=32);row(7)
    callbacks=[dict(cadence_record=10,sequence=1,thread=7,producer_prefix=0,queue_prefix=0,pending_frames=2),dict(cadence_record=20,sequence=2,thread=7,producer_prefix=1,queue_prefix=0,pending_frames=1)]
    return header,rows,[(request,b'')],callbacks


def raw(header,rows):
    h=bytearray(struct.pack('<16I',*header));h[:8]=b'MNMPLIF1'
    return bytes(h)+b''.join(struct.pack('<64I',*r) for r in rows)


class Lifetimes(unittest.TestCase):
    def test_nested_and_foreign_pending_calls_have_snapshot_and_return_envelopes(self):
        h,r,p,c=fixture();value=decode(raw(h,r),p,c)
        self.assertEqual(value['calls'],3);self.assertEqual([b['calls'] for b in value['boundaries']],[[1,2],[3]])
        self.assertEqual(value['pending_boundary_calls'][1]['snapshot_acquisition_envelope_qpc'],[30,40])
        self.assertFalse(value['source_bytes_immutable']);self.assertFalse(value['complete_presented_interval'])
    def test_envelope_and_header_refusals(self):
        h,r,p,c=fixture()
        for data in (raw(h,r)[:-1],raw(h,r)+bytes(256),b'bad'):
            with self.assertRaises(ValueError):decode(data,p,c)
        for index in (2,3,4,5,6,8):
            header=h.copy();header[index]=0 if header[index] else 1
            with self.assertRaises(ValueError):decode(raw(header,r),p,c)
    def test_clock_prefix_identity_parent_and_reserved_mutations_refuse(self):
        h,r,p,c=fixture()
        for row,word,value in ((3,2,99),(3,3,8),(3,10,0),(4,6,1),(9,8,0),(4,14,1),(3,60,1),(3,11,2),(3,12,129),(2,48,1)):
            rows=copy.deepcopy(r);rows[row][word]=value
            with self.subTest(row=row,word=word),self.assertRaises(ValueError):decode(raw(h,rows),p,c)
    def test_missing_duplicate_or_unreported_pending_call_refuses(self):
        h,r,p,c=fixture()
        for index in (5,6,7,9,16):
            rows=copy.deepcopy(r);rows.pop(index)
            for i,row in enumerate(rows,1):row[0]=i
            with self.assertRaises(ValueError):decode(raw(h,rows),p,c)
        rows=copy.deepcopy(r);rows[6]=rows[5].copy();rows[6][0]=7
        with self.assertRaises(ValueError):decode(raw(h,rows),p,c)
    def test_source_callback_identity_count_and_producer_packet_refuse(self):
        h,r,p,c=fixture()
        for field in ('cadence_record','sequence','thread','producer_prefix','queue_prefix','pending_frames'):
            callbacks=copy.deepcopy(c);callbacks[0][field]+=1
            with self.subTest(field=field),self.assertRaises(ValueError):decode(raw(h,r),p,callbacks)
        with self.assertRaises(ValueError):decode(raw(h,r),p,c[:-1])
        packets=copy.deepcopy(p);packets[0][0][3]=99
        with self.assertRaises(ValueError):decode(raw(h,r),packets,c)
    def test_return_with_live_child_and_changed_result_refuse(self):
        h,r,p,c=fixture();rows=copy.deepcopy(r);rows[7][2]=1;rows[7][4]=100;rows[7][10]=0
        with self.assertRaises(ValueError):decode(raw(h,rows),p,c)
        rows=copy.deepcopy(r);rows[8][48]=99
        with self.assertRaises(ValueError):decode(raw(h,rows),p,c)
    def test_unbound_fixture_never_claims_external_ownership(self):
        h,r,_,_=fixture();value=decode(raw(h,r));self.assertFalse(value['producer_packets_crosschecked']);self.assertFalse(value['source_callbacks_crosschecked'])


if __name__=='__main__':unittest.main()
