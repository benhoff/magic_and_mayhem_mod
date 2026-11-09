#!/usr/bin/env python3
"""Independent stretch arithmetic and actual GPU-plane binding refusal controls."""
from pathlib import Path
import sys,unittest,copy
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from natural_stretch_copy import expected_stretch,digest,verify_gpu_checkpoint


class Oracle(unittest.TestCase):
    def test_key_holes_and_complete_exterior(self):
        source=np.array([[31,7],[9,11]],dtype='uint32');before=np.full((4,5),99,dtype='uint32')
        actual,counts=expected_stretch(source,before,[0,0,2,2],[1,1,4,4],31)
        self.assertEqual(actual.tolist(),[[99]*5,[99,99,99,7,99],[99,99,99,7,99],[99,9,9,11,99]])
        self.assertEqual(counts,dict(changed_pixels=5,preserved_key_pixels=4,exterior_pixels=11))
        self.assertTrue(np.all(before==99))

    def test_alias_reads_frozen_source(self):
        original=np.array([[1,2,3,4,5,6]],dtype='uint32')
        actual,_=expected_stretch(original,original,[0,0,4,1],[1,0,6,1],None)
        self.assertEqual(actual.tolist(),[[1,1,1,2,3,4]])
        self.assertEqual(original.tolist(),[[1,2,3,4,5,6]])

    def test_floor_policy_does_not_align_last_endpoints(self):
        actual,_=expected_stretch(np.array([[5,7,9]],dtype='uint32'),np.zeros((1,2),dtype='uint32'),[0,0,3,1],[0,0,2,1],None)
        self.assertEqual(actual.tolist(),[[5,7]])

    def test_invalid_rectangles_refuse_before_indexing(self):
        for sr,dr in (([0,0,0,1],[0,0,1,1]),([-1,0,2,1],[0,0,1,1]),([0,0,1,1],[0,0,4,1])):
            with self.subTest(sr=sr,dr=dr),self.assertRaisesRegex(ValueError,'extent'):
                expected_stretch(np.ones((1,3)),np.ones((1,3)),sr,dr,None)


class Binding(unittest.TestCase):
    def inputs(self,role):
        pixels=np.array([[1,2,3],[4,5,6]],dtype='uint32');sha=digest(pixels)
        proof=dict(gpu_plane=role,target=8,target_size=[3,2],expected_sha256=sha)
        diagnostic=dict(words=[8,9] if role=='source' else [9,8],source_width=3,source_height=2,
            target_width=3,target_height=2,source_sha256=sha,before_sha256=sha)
        return proof,diagnostic,pixels

    def test_both_full_plane_roles(self):
        for role in ('source','before'):verify_gpu_checkpoint(*self.inputs(role))

    def test_corrupt_hash_pixels_identity_extent_and_role_refuse(self):
        for role in ('source','before'):
            for mutation in ('hash','pixels','identity','extent','role'):
                with self.subTest(role=role,mutation=mutation),self.assertRaises(ValueError):
                    proof,d,p=self.inputs(role)
                    if mutation=='hash':d[role+'_sha256']='0'*64
                    if mutation=='pixels':p[0,0]^=1
                    if mutation=='identity':d['words'][0 if role=='source' else 1]=77
                    if mutation=='extent':d[('source' if role=='source' else 'target')+'_width']=2
                    if mutation=='role':proof['gpu_plane']='actual'
                    verify_gpu_checkpoint(proof,d,p)


if __name__=='__main__':unittest.main()
