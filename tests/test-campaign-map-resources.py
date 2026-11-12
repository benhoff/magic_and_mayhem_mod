#!/usr/bin/env python3
"""Adverse guards for compact original World resource handoffs."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_map_resources import GLOBALS,OFFSETS,assess,decode


class Resources(unittest.TestCase):
    def fixture(self):
        h=[0x4d4d4e4d,0x31534552,1,64,320,48,0x40209ca7,33,12,4]+[0]*6
        rows=[]
        for epoch,phase in [(1,1),(1,2),(1,3),(2,1),(2,2),(2,3),(2,9)]:
            r=[0]*80;r[:16]=[len(rows)+1,phase,epoch,312,100+len(rows),20,0x5c5dd8,1,0x6cbb78,4,20+epoch,0,55,0x6cbb78,0,0]
            if phase!=9:
                for i in range(33):r[16+i]=0x1000+i if phase!=3 else 0
                for i in range(12):r[49+i]=0x2000+i if phase!=3 else 0
                r[63:71]=[0x4000,0x4010,0x4020,0x4030,0x5000,0x5010,0x5020,0x5030]
                if phase==3:
                    r[7]=0;r[8]=0x659408;r[9]=3;r[61]=1
                    r[16+GLOBALS.index(0x5d9e38)]=0xffffffff
            rows.append(r)
        return h,rows

    @staticmethod
    def packed(h,rows):return struct.pack('<16I',*h)+b''.join(struct.pack('<80I',*r) for r in rows)

    def check(self,change=None):
        h,rows=self.fixture()
        if change:change(h,rows)
        return assess(self.packed(h,rows),dict(worlds=[1,2],states=[dict(thread=312)]))

    def test_populated_retirement_and_reused_addresses_have_fresh_epoch(self):
        self.assertTrue(self.check()['verified'])

    def test_original_exit_keeps_freed_queue_map_address_as_nonowning_value(self):
        def retain(h,rows):
            for r in (rows[2],rows[5]):r[49+OFFSETS.index(0xfc4a)]=0xdead0000
        self.assertTrue(self.check(retain)['verified'])

    def test_exhausted_queue_must_reset_both_iterator_groups(self):
        for offset in OFFSETS[:8]:
            with self.subTest(offset=hex(offset)),self.assertRaises(ValueError):
                self.check(lambda h,r:r[2].__setitem__(49+OFFSETS.index(offset),1))

    def test_local_shutdown_and_resource_reset_fields_are_required(self):
        for field,value in ((61,0),(16+GLOBALS.index(0x5d9e38),0),
                            (16+GLOBALS.index(0x6e1f94),1),(16+GLOBALS.index(0x6e1f98),1)):
            with self.subTest(field=field),self.assertRaises(ValueError):
                self.check(lambda h,r:r[2].__setitem__(field,value))

    def test_unclosed_observer(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r.pop())

    def test_wrong_build(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:h.__setitem__(6,0))

    def test_fault_or_unreadable_observation(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[2].__setitem__(11,1))

    def test_mixed_original_thread(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[3].__setitem__(3,999))

    def test_surviving_bitmap_storage(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[2].__setitem__(16+GLOBALS.index(0x6de558),123))

    def test_surviving_pointer_queue(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[2].__setitem__(49+OFFSETS.index(0xfc52),1))

    def test_still_active_world(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[2].__setitem__(7,1))

    def test_provider_change_requires_accounting(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[2].__setitem__(63,0x9900))

    def test_provider_change_between_maps_requires_accounting(self):
        def replace(h,rows):
            for r in rows[3:6]:r[63]=0x9900
        with self.assertRaises(ValueError):self.check(replace)

    def test_later_generation_not_another_initial_world(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[3].__setitem__(2,1))

    def test_reserved_fields(self):
        with self.assertRaises(ValueError):self.check(lambda h,r:r[0].__setitem__(79,1))


if __name__=='__main__':unittest.main()
