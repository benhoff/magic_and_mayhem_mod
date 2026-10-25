#!/usr/bin/env python3
"""Wire-bound controls for original campaign ownership observation."""
from pathlib import Path
import struct
import sys
import unittest
import importlib.util
import tempfile
import os
import subprocess
import time

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from alternate_canvas_journeys import lane,saved_row,keeping_lease,enter_mini,LeasePulse


class JourneyWire(unittest.TestCase):
    def test_pulse_survives_blocking_host_and_stops_before_unmapping(self):
        beats=[];pulse=LeasePulse(lambda:beats.append(1),interval=.01);pulse.start()
        time.sleep(.09);pulse.close();count=len(beats);time.sleep(.03)
        self.assertGreaterEqual(count,3);self.assertEqual(len(beats),count)

    def fixture(self,names):
        raw=bytearray(12960);struct.pack_into('<8I',raw,0,12,17,6,5,0,0,len(names),0)
        for i,name in enumerate(names):raw[160+i*128:160+i*128+len(name)]=name
        return raw

    def test_original_one_based_rows(self):
        raw=self.fixture([b'old',b'owner-watch',b'new'])
        self.assertEqual(saved_row(raw,'owner-watch'),2)

    def test_last_bounded_row(self):
        raw=self.fixture([('row%03d'%i).encode() for i in range(100)])
        self.assertEqual(saved_row(raw,'row099'),100)

    def test_duplicate_refuses_before_dispatch(self):
        with self.assertRaises(ValueError):saved_row(self.fixture([b'owner',b'owner']),'owner')

    def test_missing_refuses_before_dispatch(self):
        with self.assertRaises(ValueError):saved_row(self.fixture([b'other']),'owner')

    def test_over_budget_count(self):
        raw=self.fixture([]);struct.pack_into('<I',raw,24,101)
        with self.assertRaises(ValueError):saved_row(raw,'owner')

    def test_truncated_lane(self):
        with self.assertRaises(ValueError):saved_row(self.fixture([])[:-1],'owner')

    def test_engine_seqlock(self):
        raw=bytearray(131072);struct.pack_into('<I',raw,128,2);raw[110592:110592+12960]=self.fixture([b'owner'])
        self.assertEqual(saved_row(lane(raw,110592,12960),'owner'),1)
        struct.pack_into('<I',raw,128,3);self.assertIsNone(lane(raw,110592,12960))

    def test_changing_engine_seqlock(self):
        class Changing(bytearray):
            def __getitem__(self,key):
                if isinstance(key,slice):struct.pack_into('<I',self,128,4)
                return super().__getitem__(key)
        raw=Changing(131072);struct.pack_into('<I',raw,128,2)
        self.assertIsNone(lane(raw,110592,12960))

    def test_debugger_versions_remain_readable(self):
        root=Path(__file__).resolve().parents[1]
        for name in ('test-alternate-ownership-session','test-alternate-canvas-live','test-screen-presentation-live'):
            spec=importlib.util.spec_from_file_location(name,root/'tools'/(name+'.py'));module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            with tempfile.TemporaryDirectory() as temporary:
                path=Path(temporary)/'watch.bin'
                for version in (1,2,3):
                    header=[0]*64;header[:5]=[0x414d4e4d,0x31305743,version,256,1];header[39:45]=[42,192,160,640,480,10]
                    path.write_bytes(struct.pack('<64I',*header)+(bytes(256) if version>=2 else b''))
                    value=module.decode(path);self.assertTrue(value['complete']);self.assertEqual(value['slot_writes'],[0,0])
                    if version==3 and name=='test-alternate-ownership-session':self.assertEqual(value['client']['width'],640)

    def test_unknown_debugger_version_refused(self):
        root=Path(__file__).resolve().parents[1];spec=importlib.util.spec_from_file_location('owner_session',root/'tools/test-alternate-ownership-session.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as temporary:
            path=Path(temporary)/'watch.bin';header=[0]*64;header[:5]=[0x414d4e4d,0x31305743,4,256,1];path.write_bytes(struct.pack('<64I',*header))
            with self.assertRaises(ValueError):module.decode(path)

    def test_heartbeat_during_blocking_child(self):
        beats=[];result=keeping_lease([sys.executable,'-c','import time;time.sleep(.15);print("done")'],dict(os.environ),lambda:beats.append(1),2,text=True)
        self.assertEqual(result.stdout.strip(),'done');self.assertGreaterEqual(len(beats),3)

    def test_timed_out_child_is_reaped(self):
        with self.assertRaises(subprocess.TimeoutExpired):keeping_lease([sys.executable,'-c','import time;time.sleep(10)'],dict(os.environ),lambda:None,.08)

    def test_mini_becomes_ready_during_capture(self):
        channel=bytearray(131072);struct.pack_into('<I',channel,128,2);struct.pack_into('<7I',channel,132,1,0,0,0,1,1,0);events=[]
        class Player:
            def activate(self):return {}
            def hermes(self):
                struct.pack_into('<7I',channel,132,2,17,1,0,1,1,0)
                return dict(hermes_title=False)
        enter_mini(channel,Player(),events.append,lambda:None,[],attempts=1,wait_seconds=0)
        self.assertEqual(events,['escape'])

    def test_existing_mini_is_not_toggled(self):
        channel=bytearray(131072);struct.pack_into('<I',channel,128,2);struct.pack_into('<7I',channel,132,1,17,1,0,1,1,0)
        enter_mini(channel,None,lambda _:self.fail('Unexpected input'),lambda:None,[])

    def test_retired_menu_is_not_given_input(self):
        channel=bytearray(131072);struct.pack_into('<I',channel,128,2);struct.pack_into('<7I',channel,132,1,0,0,0,5,1,0)
        with self.assertRaises(ValueError):enter_mini(channel,None,lambda _:self.fail('Unexpected input'),lambda:None,[])


if __name__=='__main__':unittest.main()
