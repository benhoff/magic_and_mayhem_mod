"""Malformed native journals and bounded origin lookup controls."""
import importlib.util
from pathlib import Path
import struct
import sys
import tracemalloc
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from drawing_source_held_refusal import decode as held
from drawing_source_pending_refusal import decode as pending
from drawing_wine_lifetime import decode as lifetime
from drawing_wine_buffer import decode as buffer


def fixture(name,function):
    spec=importlib.util.spec_from_file_location(name,ROOT/('tests/test-drawing-'+name+'.py'))
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return getattr(module,function)()


def change(raw,offset,value):
    data=bytearray(raw);struct.pack_into('<I',data,offset,value);return data


def crowded_lifetime(objects=256,repeats=3000):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048]
    rows=[];call=0
    def pair(method,object_id,generation,result=0):
        nonlocal call
        call+=1;r=[0]*24;r[:5]=[len(rows)+1,1,call,method,7]
        if method==2:r[10:12]=[object_id,generation]
        rows.append(r);r=list(r);r[:2]=[len(rows)+1,2];r[8]=result
        r[10:12]=[object_id,generation];rows.append(r)
    for i in range(1,objects+1):pair(3 if i==1 else 1,i,i)
    for _ in range(repeats):pair(2,objects,objects,1)
    r=[0]*24;r[:5]=[len(rows)+1,4,1,call,7];r[11]=objects;rows.append(r)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)


class FourthTen(unittest.TestCase):
    def test_held_all_reasons_and_raw_rejected_phase(self):
        raw=fixture('source-held-refusal','fixture')
        for reason in range(1,12):
            self.assertEqual(held(change(raw,68,reason))['reason_code'],reason)
        invalid=change(change(raw,68,8),72,3)
        self.assertEqual(held(invalid)['phase'],3)
        with self.assertRaises(ValueError):held(change(raw,72,3))

    def test_held_impossible_budget_counts_refuse(self):
        raw=fixture('source-held-refusal','fixture')
        for offset,value in ((32,2049),(36,1025),(40,1024**3+1)):
            with self.subTest(offset=offset),self.assertRaisesRegex(ValueError,'budget'):
                held(change(raw,offset,value))

    def test_pending_owner_and_exported_slot_order(self):
        raw=fixture('source-pending-refusal','fixture')
        with self.assertRaises(ValueError):pending(change(raw,64+12,0))
        raw=change(change(raw,64+4,2),128+4,1)
        with self.assertRaises(ValueError):pending(raw)

    def test_terminal_threads_cannot_be_unknown(self):
        raw=fixture('wine-lifetime','wire')
        with self.assertRaises(ValueError):lifetime(change(raw,len(raw)-96+16,0))
        raw,life=fixture('wine-buffer','fixture')
        with self.assertRaises(ValueError):buffer(change(raw,len(raw)-96+16,0),life)

    def test_lifetime_active_capacity_is_enforced(self):
        self.assertEqual(lifetime(crowded_lifetime(2048,0))['remaining_observed_objects'],2048)
        with self.assertRaisesRegex(ValueError,'capacity'):lifetime(crowded_lifetime(2049,0))

    def test_origin_lookup_memory_scales_with_events(self):
        raw,_=fixture('wine-buffer','fixture');life=crowded_lifetime()
        tracemalloc.start()
        try:
            result=buffer(raw,life);_,peak=tracemalloc.get_traced_memory()
        finally:tracemalloc.stop()
        self.assertEqual(result['calls'][0]['texture_generation'],0)
        # Old all-object snapshots use >60 MiB for this finite corpus.
        self.assertLess(peak,16*1024*1024)

    def test_origin_prefix_retirement_and_reuse(self):
        raw,life=fixture('wine-buffer','fixture')
        # Independently append a texture creation, retirement and same-ID reuse.
        rows=[list(r) for r in struct.iter_unpack('<24I',life[64:-96])]
        for call,method,generation in ((2,1,2),(3,2,2),(4,1,3)):
            r=[0]*24;r[:5]=[len(rows)+1,1,call,method,7]
            if method==2:r[10:12]=[2,generation]
            rows.append(r);r=list(r);r[:2]=[len(rows)+1,2];r[10:12]=[2,generation]
            if method==2:r[12]=2
            rows.append(r)
        t=[0]*24;t[:5]=[len(rows)+1,4,1,4,7];t[11]=3;rows.append(t)
        life=life[:64]+b''.join(struct.pack('<24I',*r) for r in rows)
        for prefix,generation in ((3,0),(4,2),(5,2),(6,0),(7,0),(8,3)):
            sample=change(change(raw,64+96+16*4,prefix),64+96+14*4,generation)
            self.assertEqual(buffer(sample,life)['calls'][0]['texture_generation'],generation)
            with self.assertRaises(ValueError):buffer(change(sample,64+96+14*4,99),life)


if __name__=='__main__':unittest.main()
