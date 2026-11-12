"""Independent clipping math and strict stopped-observation refusal controls."""
import copy
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from native_copy_clip_regions import decode,plan_regions,ROW_BYTES


class ClipRegions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory(prefix='clip-region-guards-',dir='/tmp')
        out=Path(cls.tmp.name);binary=out/'recorder';payload=out/'record.bin'
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(ROOT/'tests/native-copy-clip-regions-test.c'),'-o',str(binary)],check=True)
        subprocess.run([str(binary),str(payload)],check=True)
        cls.raw=payload.read_bytes();cls.header,cls.rows=decode(cls.raw)

    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()

    def altered(self,word,value,*,row=False):
        raw=bytearray(self.raw);struct.pack_into('<I',raw,(128 if row else 0)+word*4,value);return bytes(raw)

    def test_actual_c_writer_preserves_owned_input_and_order(self):
        self.assertEqual(len(self.rows),1);r=self.rows[0]
        self.assertEqual(r['regions'],[[0,0,2,3],[2,0,4,3]])
        self.assertEqual(plan_regions(r),([[0,0,2,3,0,0],[2,0,4,3,2,0]],0))

    def test_header_refusals(self):
        for word,value in ((2,2),(3,127),(4,1),(5,8193),(6,2),(7,1),(8,0),(9,1),(10,0),(11,8),(12,1)):
            with self.subTest(word=word),self.assertRaises(ValueError):decode(self.altered(word,value))

    def test_row_identity_and_reserved_refusals(self):
        for word,value in ((0,2),(1,0),(1,7),(2,8),(3,0),(5,0),(6,0),(9,0),(11,0),
                           (14,257),(15,63),(24,0),(25,2049),(26,24),(29,24),(30,1),(31,5),(32,257),(35,1)):
            with self.subTest(word=word),self.assertRaises(ValueError):decode(self.altered(word,value,row=True))

    def test_region_header_bounds_and_unused_tail_refusals(self):
        for word,value in ((64,31),(65,2),(66,1),(67,31),(68,1),(72,3),(80,1),(1095,1)):
            with self.subTest(word=word),self.assertRaises(ValueError):decode(self.altered(word,value,row=True))
        raw=bytearray(self.raw);raw[128+ROW_BYTES]=1
        with self.assertRaisesRegex(ValueError,'unused tail'):decode(raw)

    def test_signed_surface_and_region_clipping(self):
        r=copy.deepcopy(self.rows[0]);r.update(destination=[-2,-1,4,3],source=[0,0,6,4],
            source_description=[6,4,16],regions=[[-3,-3,2,3]])
        self.assertEqual(plan_regions(r),([[2,1,4,4,0,0]],0))

    def test_partial_prefix_source_failure_keeps_prior_piece(self):
        r=copy.deepcopy(self.rows[0]);r['source']=[2,0,6,3]
        self.assertEqual(plan_regions(r),([[2,0,4,3,0,0]],0x88760096))

    def test_empty_clip_and_first_failure_are_distinct(self):
        r=copy.deepcopy(self.rows[0]);r['regions']=[]
        self.assertEqual(plan_regions(r),([],0))
        r['regions']=[[0,0,4,3]];r['source']=[-1,0,3,3]
        self.assertEqual(plan_regions(r),([],0x88760096))

    def test_stretch_is_not_an_ordinary_copy(self):
        r=copy.deepcopy(self.rows[0]);r['source']=[0,0,2,3]
        with self.assertRaisesRegex(ValueError,'extents'):plan_regions(r)


if __name__=='__main__':unittest.main()
