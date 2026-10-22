"""Driver order must come from identities, independently of oracle pixels."""
from pathlib import Path
import sys,struct,unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_copy_order import normalize

def fixture():
    records=[]
    for sequence,dst,src in ((1,2,1),(2,1,3)):
        r=[0]*24;r[:7]=[96,sequence,6,dst,src,2,2];r[12:15]=[2,2,0xffffffff];records.append((tuple(r),b''))
    raw=b'X'*64+b''.join(struct.pack('<24I',*r) for r,_ in records)
    points=[];calls=[];workers=[];samples=[];packets=[]
    # Earlier command's wrapper returns second, with another thread's copy in between.
    for api,thread,dst,src,worker in ((1,10,1,3,1),(2,11,2,1,2)):
        calls.append(dict(entry=api,thread=thread,method=7,related=1,hresult=0))
        for phase in (1,2,3):
            row=[0]*48;row[1:3]=[phase,api];row[22]=dst;row[38]=src;points.append(row)
        workers.append(dict(call=worker,source_box=[0,0,2,2,0,1],destination_box=[0,0,2,2,0,1],flags=0x41000000,effects=[0]*6,filter=1,source_subresource=0,destination_subresource=0))
        samples.append(dict(submission=worker,api_entry=api));packets.append(dict(worker=worker,submission=worker))
    # Limit each candidate to its own endpoint pair; both return before emission.
    original=dict(observation=dict(calls=calls),wine_blit_worker_observation=dict(calls=workers),wine_source_map_observation=dict(samples=samples),wine_blit_packet_observation=dict(bindings=packets))
    point_raw=b'X'*64+b''.join(struct.pack('<48I',*r) for r in points)+b'X'*192
    return raw,records,point_raw,original

class Cases(unittest.TestCase):
    def test_copy_effect_follows_driver_order(self):
        args=fixture();result,proof=normalize(*args,[])
        self.assertEqual([struct.unpack_from('<24I',result,64+i*96)[3:5] for i in range(2)],[(1,3),(2,1)])
        self.assertEqual([struct.unpack_from('<I',result,68+i*96)[0] for i in range(2)],[1,2])
        self.assertEqual(len(proof['moved_copies']),2);self.assertFalse(proof['original_pixels_used_as_native_inputs'])
    def test_read_uses_command_prefix(self):
        query=dict(sequence=1,canvas=1,api_entry=2);_,proof=normalize(*fixture(),[query])
        self.assertEqual(query['sequence'],1);self.assertEqual(proof['rebound_reads'],[])
        query=dict(sequence=1,canvas=3,api_entry=1);_,proof=normalize(*fixture(),[query])
        self.assertEqual(query['sequence'],0);self.assertEqual(query['original_sequence'],1)
    def test_missing_read_identity_refuses(self):
        with self.assertRaises(ValueError):normalize(*fixture(),[dict(sequence=1,canvas=1)])
    def test_unsupported_copy_is_a_barrier(self):
        raw,records,points,original=fixture();original['wine_blit_worker_observation']['calls'][0]['flags']=123
        result,proof=normalize(raw,records,points,original,[]);self.assertEqual(result,raw);self.assertEqual(proof['moved_copies'],[])
    def test_multiple_driver_workers_preserve_the_recorded_first_command_policy(self):
        raw,records,points,original=fixture()
        original['wine_blit_worker_observation']['calls'].append(dict(original['wine_blit_worker_observation']['calls'][0],call=3))
        original['wine_source_map_observation']['samples'].append(dict(submission=3,api_entry=1))
        original['wine_blit_packet_observation']['bindings'].append(dict(worker=3,submission=3))
        result,proof=normalize(raw,records,points,original,[])
        self.assertEqual(result,normalize(*fixture(),[])[0])
        self.assertEqual(len(proof['moved_copies']),2)
    def test_explicit_read_worker_cannot_disagree_with_its_api(self):
        with self.assertRaises(ValueError):
            normalize(*fixture(),[dict(sequence=1,canvas=3,api_entry=1,worker=2)])
        query=dict(sequence=1,canvas=3,api_entry=1,worker=1)
        normalize(*fixture(),[query]);self.assertEqual(query['sequence'],0)
    def test_failed_normalization_keeps_original_snapshot_prefixes(self):
        query=dict(sequence=1,canvas=3,api_entry=1)
        with self.assertRaises(ValueError):
            normalize(*fixture(),[query,dict(sequence=1,canvas=1)])
        self.assertEqual(query,dict(sequence=1,canvas=3,api_entry=1))
    def test_no_original_pixels_can_select_order(self):
        args=list(fixture());args[3]['expected_pixels']=b'arbitrary diagnostic bytes'
        self.assertEqual(normalize(*args,[])[0],normalize(*fixture(),[])[0])
    def test_ambiguous_api_bounds_refuse(self):
        raw,records,points,original=fixture();r=list(records[1][0]);r[3:5]=[2,1];records[1]=(tuple(r),b'');raw=raw[:160]+struct.pack('<24I',*r)
        with self.assertRaises(ValueError):normalize(raw,records,points,original,[])
#!/usr/bin/env python3
"""Refuse incomplete or mismatched copy histories while retaining reversed returns."""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_copy_order import decode


def journal(mutate=None):
    h = bytearray(64)
    h[:8] = b'MNMDCQ01'
    struct.pack_into('<4I', h, 8, 1, 128, 0x40209ca7, 65536)
    rows = []
    for phase, request, owner, prefix, returned, active in [(1, 1, 11, 10, 0, 1), (1, 2, 22, 11, 0, 2), (2, 2, 22, 12, 12, 1), (2, 1, 11, 13, 13, 0)]:
        r = [0]*32
        r[:8] = [len(rows)+1, phase, request, owner, 7, 100, prefix, 1]
        r[8:20] = [1, 2, 800, 600, 0, 0, 0, 0, 0, 800, 600, 0xffffffff]
        r[20], r[21], r[24] = 0x1234, returned, active
        rows.append(r)
    r = [0]*32
    r[0], r[1], r[2], r[6], r[7] = 5, 3, 2, 13, 1
    rows.append(r)
    if mutate:
        mutate(rows)
    return bytes(h) + b''.join(struct.pack('<32I', *r) for r in rows)


class Order(unittest.TestCase):
    def test_two_requests_cannot_claim_one_producer_return(self):
        with self.assertRaises(ValueError):decode(journal(lambda rows: rows[3].__setitem__(21,12)))

    def test_tick_wrap_and_reversed_callback_time(self):
        def wrap(rows):rows[0][5]=0xfffffff0;rows[3][5]=16
        self.assertEqual(decode(journal(wrap))['requests'],2)
        with self.assertRaises(ValueError):decode(journal(lambda rows: rows[3].__setitem__(5,99)))
    def test_reversed_returns_retained(self):
        r = decode(journal())
        self.assertEqual([e['request'] for e in r['entries']], [1, 2])
        self.assertEqual([e['request'] for e in r['returns']], [2, 1])
        self.assertFalse(r['driver_order_proved'])

    def test_changed_descriptor_owner_and_bad_return_prefix_refuse(self):
        for index, value in [(8, 99), (3, 99), (21, 10)]:
            with self.assertRaises(ValueError):
                decode(journal(lambda rows: rows[3].__setitem__(index, value)))

    def test_duplicate_entry_and_incomplete_terminal_refuse(self):
        for mutation in [lambda r: r[1].__setitem__(2, 1), lambda r: r[-1].__setitem__(24, 1), lambda r: r.pop(3)]:
            with self.assertRaises(ValueError):
                decode(journal(mutation))

    def test_reserved_and_count_corruption_refuse(self):
        for index in (22, 25, 24):
            with self.assertRaises(ValueError):
                decode(journal(lambda rows: rows[0].__setitem__(index, 9)))


if __name__ == '__main__':
    unittest.main()
