#!/usr/bin/env python3
"""Controls for inverse ANI placement; no original game or pixel oracle."""
import hashlib
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_animation_target import asset_offsets, frame_identity, projected_targets, POLICY


def assets():
    frame = bytearray(40)
    struct.pack_into('<5I', frame, 0, 40, 1, 1, 0, 0)
    spr = bytearray(24+768+4) + frame
    struct.pack_into('<6I', spr, 0, 0x525053, len(spr), 4, 1, 1, 0)
    ani = bytearray(44+8+88)
    struct.pack_into('<6I', ani, 0, 0x494e41, len(ani), 2, 5, 0, 2)
    struct.pack_into('<2I', ani, 44, 0, 2)
    struct.pack_into('<4i', ani, 52, 0, 0, -76, -82)
    struct.pack_into('<i', ani, 96, 6)
    return spr, ani, frame


class Controls(unittest.TestCase):
    def test_exact_asset_mapping(self):
        s,a,f=assets();self.assertEqual(asset_offsets(s,a), {frame_identity(f):[(-76,-82)]})
    def test_recorder_clears_palette_pointer(self):
        _,_,f=assets();before=frame_identity(f);struct.pack_into('<I',f,28,0xabcdef01)
        self.assertEqual(frame_identity(f),before)
    def test_other_source_bytes_remain_identity(self):
        _,_,f=assets();before=frame_identity(f);f[20]=1
        self.assertNotEqual(frame_identity(f),before)
    def test_truncated_frame_refused(self):
        with self.assertRaises(ValueError):frame_identity(bytes(39))
    def test_bad_spr_extent_refused(self):
        s,a,_=assets();s.pop()
        with self.assertRaises(ValueError):asset_offsets(s,a)
    def test_bad_spr_table_refused(self):
        s,a,_=assets();struct.pack_into('<I',s,792,100000)
        with self.assertRaises(ValueError):asset_offsets(s,a)
    def test_bad_ani_extent_refused(self):
        s,a,_=assets();a.pop()
        with self.assertRaises(ValueError):asset_offsets(s,a)
    def test_bad_ani_sequence_refused(self):
        s,a,_=assets();struct.pack_into('<I',a,48,3)
        with self.assertRaises(ValueError):asset_offsets(s,a)
    def test_bad_frame_reference_refused(self):
        s,a,_=assets();struct.pack_into('<i',a,56,1)
        with self.assertRaises(ValueError):asset_offsets(s,a)
    def target(self,index):
        body=dict(frame_sha256='frame',point=[379,285],anchor=[308,198],opaque_pixel=[20,60])
        with patch('broader_picking_animation_target.sprite_target',return_value=body):
            return projected_targets([], [308,198,106], [15,26,4], index)
    def test_counterexample_uses_projected_base(self):
        body,cell=self.target({'frame':[[-76,-82]]})
        self.assertEqual(body['point'],[379,285]);self.assertEqual(cell['point'],[384,280])
        self.assertEqual(cell['expected_cell_xy'],[15,26]);self.assertEqual(cell['target_policy'],POLICY)
        self.assertNotIn('opaque_pixel',cell)
    def test_missing_mapping_refused(self):
        with self.assertRaises(ValueError):self.target({})
    def test_ambiguous_mapping_refused(self):
        with self.assertRaises(ValueError):self.target({'frame':[[-76,-82],[-75,-82]]})
    def test_noninteger_mapping_refused(self):
        with self.assertRaises(ValueError):self.target({'frame':[[False,-82]]})
    def test_no_ground_or_hit_feedback_argument(self):
        import inspect
        self.assertEqual(list(inspect.signature(projected_targets).parameters),['records','history','position','index'])

if __name__=='__main__':unittest.main()
