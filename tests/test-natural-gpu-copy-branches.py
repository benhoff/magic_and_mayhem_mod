#!/usr/bin/env python3
"""Independent lattice geometry and strict caller/snapshot/key refusals."""
from pathlib import Path
import copy
import random
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from natural_gpu_copy_branches import union_area, classify_call


def fixture(pieces=None, *, attached=True, keyed=False, self_copy=False, dr=None, sr=None):
    pieces = [[0,0,8,8]] if pieces is None else pieces
    row=[0]*44;row[0]=1;row[1]=0x58c03d;row[2]=0x18c03d
    row[10]=1|2|4|16|32768|(32 if attached else 0)|(64 if keyed else 0)|(128 if self_copy else 0)
    row[15]=2;row[16:22]=[8,8,16,8,8,16];row[22]=0x1234;row[23]=17 if attached else 0
    row[24:28]=[0,0,8,8] if sr is None else [v&0xffffffff for v in sr]
    row[28:32]=[0,0,8,8] if dr is None else [v&0xffffffff for v in dr]
    row[12]=0x8000 if keyed else 0
    row[40:44]=[100,100+len(pieces)+self_copy-1,11,11 if self_copy else 12]
    gpu=[0]*32;gpu[4:9]=[row[42],row[43],13 if self_copy else 0,row[40],row[41]]
    gpu[15]=(1 if self_copy else 0)|(2 if attached else 0)|(4 if keyed else 0);gpu[16]=len(pieces)
    checked={};source=13 if self_copy else 11
    if self_copy:checked[100]=dict(words=[11,13,0,0,8,8,0,0,0,0,0,0,0],source_width=8,source_height=8,target_width=8,target_height=8)
    dr0=[0,0,8,8] if dr is None else dr;sr0=[0,0,8,8] if sr is None else sr
    for i,(x0,y0,x1,y1) in enumerate(pieces):
        sx,sy=x0-dr0[0]+sr0[0],y0-dr0[1]+sr0[1]
        checked[100+self_copy+i]=dict(words=[source,row[43],sx,sy,sx+x1-x0,sy+y1-y0,x0,y0,int(keyed),row[22] if keyed else 0,0,0,0],source_width=8,source_height=8,target_width=8,target_height=8)
    return row,gpu,checked


def check(data, **identity):
    return classify_call(*data,**(dict(module_base=0x400000,module_extent=0x30d000)|identity))


class Controls(unittest.TestCase):
    def test_independent_lattice_union(self):
        rng=random.Random(20261026)
        for _ in range(400):
            rects=[]
            for i in range(rng.randrange(1,25)):
                x0,x1=sorted(rng.sample(range(-8,9),2));y0,y1=sorted(rng.sample(range(-8,9),2));rects.append([x0,y0,x1,y1])
            pixels={(x,y) for x0,y0,x1,y1 in rects for x in range(x0,x1) for y in range(y0,y1)}
            self.assertEqual(union_area(rects),len(pixels))
    def test_attachment_is_not_geometric_clipping(self):
        v=check(fixture());self.assertTrue(v['attached_clipper']);self.assertFalse(v['geometrically_reduced']);self.assertFalse(v['clipper_reduced_within_bounds'])
    def test_multiple_pieces_can_cover_every_pixel(self):
        v=check(fixture([[0,0,4,8],[4,0,8,8]]));self.assertEqual(v['emitted_union_area'],64);self.assertFalse(v['geometrically_reduced'])
    def test_overlapping_pieces_cannot_hide_a_hole(self):
        v=check(fixture([[0,0,4,8],[0,0,4,8]]));self.assertEqual(v['emitted_union_area'],32);self.assertTrue(v['clipper_reduced_within_bounds'])
    def test_disjoint_holes_are_real_reduction(self):
        v=check(fixture([[0,0,2,8],[6,0,8,8]]));self.assertEqual(v['emitted_union_area'],32);self.assertTrue(v['geometrically_reduced'])
    def test_bounds_clipping_is_separate(self):
        v=check(fixture([[0,0,6,6]],dr=[-2,-2,6,6]));self.assertEqual((v['requested_area'],v['bounded_area'],v['emitted_union_area']),(64,36,36));self.assertTrue(v['bounds_reduced']);self.assertTrue(v['geometrically_reduced']);self.assertFalse(v['clipper_reduced_within_bounds'])
    def test_keyed_gpu_source_is_bound(self):
        v=check(fixture(keyed=True));self.assertTrue(v['keyed_gpu'])
    def test_keyed_cpu_source_is_not_gpu_branch(self):
        data=fixture(keyed=True);data[0][15]=1
        with self.assertRaisesRegex(ValueError,'GPU-source'):check(data)
    def test_self_copy_requires_frozen_opaque_snapshot(self):
        v=check(fixture(self_copy=True,keyed=True));self.assertTrue(v['self_copy']);self.assertTrue(v['keyed_gpu'])
    def test_self_snapshot_refusals(self):
        for field,value in [(0,12),(1,11),(2,1),(4,7),(8,1),(9,0x1234),(12,1)]:
            with self.subTest(field=field):
                data=fixture(self_copy=True);data[2][100]['words'][field]=value
                with self.assertRaises(ValueError):check(data)
        data=fixture(self_copy=True);del data[2][100]
        with self.assertRaises(ValueError):check(data)
    def test_scratch_cannot_alias_destination(self):
        data=fixture(self_copy=True);data[1][6]=11
        with self.assertRaises(ValueError):check(data)
    def test_key_or_resource_corruption_refuses(self):
        for field,value in [(0,99),(1,99),(8,1),(9,1),(12,1)]:
            data=fixture();data[2][100]['words'][field]=value
            with self.assertRaises(ValueError):check(data)
        data=fixture(keyed=True);data[0][10]&=~64
        with self.assertRaises(ValueError):check(data)
    def test_original_geometry_and_translation_refuse(self):
        for field,value in [(2,1),(4,9),(6,1),(7,1)]:
            data=fixture();data[2][100]['words'][field]=value
            with self.assertRaises(ValueError):check(data)
    def test_original_module_identity_refuses(self):
        for ident in [dict(module_base=0),dict(module_extent=0),dict(module_base=0x500000)]:
            with self.assertRaises(ValueError):check(fixture(),**ident)
        data=fixture();data[0][2]+=1
        with self.assertRaises(ValueError):check(data)
    def test_missing_output_or_bad_piece_count_refuses(self):
        data=fixture();data[2].clear()
        with self.assertRaises(ValueError):check(data)
        data=fixture();data[1][16]=2
        with self.assertRaises(ValueError):check(data)
    def test_unsuccessful_or_unknown_gpu_state_refuses(self):
        for index,value in [(13,0x80000000),(10,1|2|16),(11,1)]:
            data=fixture();data[0][index]=value
            with self.assertRaises(ValueError):check(data)
        for index,value in [(15,0),(17,0x80000000),(18,0x80000000)]:
            data=fixture();data[1][index]=value
            with self.assertRaises(ValueError):check(data)
    def test_unexplained_unclipped_hole_refuses(self):
        with self.assertRaises(ValueError):check(fixture([[0,0,4,8]],attached=False))
    def test_pure_diagnostic_does_not_change_inputs(self):
        data=fixture(self_copy=True,keyed=True);before=copy.deepcopy(data);check(data);self.assertEqual(data,before)


if __name__=='__main__':unittest.main()
