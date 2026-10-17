#!/usr/bin/env python3
import struct
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_update_routes import analyze


def trace(events):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCAD01');h[2:11]=[2,96,1000,0,len(events)+3,0,0,7,262144]
    rows=[]
    for event in [2,3]+events+[2]:
        r=[0]*24;r[:7]=[len(rows)+1,event,7,len(rows)+1,0,1,1]
        if not rows:r[5]=0
        if event==2:r[6]=1 if not rows else 2
        r[13]={9:0x46bb9d,10:0x46bba5,18:0x46bbd8,20:0x46bc19,21:0x4fd536,8:0x4fd559}.get(event,0)
        if event in (9,10):r[15]=64
        if event==21:r[19]=1
        rows.append(r)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)


class Routes(unittest.TestCase):
    def run_trace(self,raw):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'cadence.bin';p.write_bytes(raw);return analyze(p)
    def test_world_and_unavailable_target(self):
        r=self.run_trace(trace([9,10,18,21,8]));self.assertEqual(r['windows'][0]['queue_absence_or_route'],'selected_world_queue')
        raw=bytearray(trace([9,10,18,21,20]));struct.pack_into('<I',raw,64+5*96+15*4,64);struct.pack_into('<I',raw,64+5*96+19*4,0)
        r=self.run_trace(raw);self.assertEqual(r['windows'][0]['queue_absence_or_route'],'original_default_target_unavailable');self.assertTrue(r['windows'][0]['results_push_observed'])
    def test_original_gate_flags_explain_absence(self):
        for events,row,reason in (([9],2,'original_suppression_branch'),([9,10,20],3,'original_transition_branch')):
            raw=bytearray(trace(events));struct.pack_into('<I',raw,64+row*96+15*4,0)
            r=self.run_trace(raw);self.assertEqual(r['windows'][0]['queue_absence_or_route'],reason);self.assertFalse(r['no_rendering_in_queue_free_windows_proved'])
    def test_early_exit_retains_unresolved_gap(self):
        r=self.run_trace(trace([]));self.assertFalse(r['all_selected_branches_accounted']);self.assertFalse(r['complete_presented_journey'])
    def test_legacy_cannot_supply_comparison_flags(self):
        raw=bytearray(trace([]));struct.pack_into('<I',raw,8,1)
        with self.assertRaises(ValueError):self.run_trace(raw)
    def test_scene_without_gates_and_impossible_target_queue_refuse(self):
        with self.assertRaises(ValueError):self.run_trace(trace([18,8]))
        raw=bytearray(trace([9,10,18,21,8]));struct.pack_into('<I',raw,64+5*96+15*4,64);struct.pack_into('<I',raw,64+5*96+19*4,0)
        with self.assertRaises(ValueError):self.run_trace(raw)
    def test_duplicate_route_and_changed_site_refuse(self):
        with self.assertRaises(ValueError):self.run_trace(trace([9,9]))
        raw=bytearray(trace([9]));struct.pack_into('<I',raw,64+2*96+13*4,0)
        with self.assertRaises(ValueError):self.run_trace(raw)


if __name__=='__main__':unittest.main()
