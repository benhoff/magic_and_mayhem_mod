#!/usr/bin/env python3
"""Raster return provenance, input separation and external comparison guards."""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_scene_corpus as corpus


def history():
    header=bytearray(64);header[:8]=b'MNMPRO01'
    struct.pack_into('<5I',header,8,1,64,0x40209ca7,32768,1)
    output=bytes(header)
    for sequence,kind in enumerate((1,11,9,12,4),1):
        r=[0]*24;r[0]=96;r[1]=sequence;r[2]=kind;r[3]=7;r[5]=5;r[6]=3
        payload=b''
        if kind in (11,12):r[14]=1
        if kind==9:
            payload=struct.pack('<10I',40,2,2,0,0,0,0,0,0,0)
            r[0]+=len(payload);r[18]=len(payload);r[19]=40;r[21]=1
        if kind==4:r[21]=1234
        output+=struct.pack('<24I',*r)+payload
    return output


class DrawingResultsTest(unittest.TestCase):
    def test_only_raster_observations_are_removed(self):
        raw=history();clean=corpus.raster_inputs(raw)
        expected=bytearray(raw);struct.pack_into('<I',expected,64+96*2+84,0)
        self.assertEqual(clean,bytes(expected))
        self.assertEqual(len(raw),len(clean))
        self.assertEqual(corpus.original_raster_results(raw,{'bypass_checks':[]}),[(3,1,7,0,1)])

    def test_bypassed_returns_are_not_original_observations(self):
        for historical in ({},{'bypass_checks':None},{'bypass_checks':[{'sequence':3}]}):
            with self.subTest(historical=historical),self.assertRaises(ValueError):
                corpus.original_raster_results(history(),historical)
        self.assertEqual(len(corpus.original_raster_results(history(),{'bypass_checks':[{'sequence':6}]})),1)

    def test_native_return_mismatch_and_unknown_remain_visible(self):
        expected=[(3,1,7,0,1)]
        for value in ('0','unknown'):
            with self.subTest(value=value):
                result=corpus.compare_raster_results(f'3\t1\t7\t0\t{value}\n'.encode(),expected)
                self.assertFalse(result['matching']);self.assertEqual(result['mismatches'],1)
                self.assertEqual(result['unknown_results'],int(value=='unknown'))
        self.assertTrue(corpus.compare_raster_results(b'3\t1\t7\t0\t1\n',expected)['matching'])

    def test_native_result_identity_count_and_bounds_refuse(self):
        expected=[(3,1,7,0,1)]
        for data in (b'',b'3\t2\t7\t0\t1\n',b'3\t1\t8\t0\t1\n',b'3\t1\t7\t0\t65536\n',
                     b'3\t1\t7\t0\t-1\n',b'3\t1\t7\t0\t1\textra\n',b'3\t1\t7\t0\t1\n'*2):
            with self.subTest(data=data),self.assertRaises(ValueError):
                corpus.compare_raster_results(data,expected)


if __name__=='__main__':unittest.main()
