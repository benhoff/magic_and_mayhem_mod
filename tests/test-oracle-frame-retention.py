#!/usr/bin/env python3
"""Guard prospective temporary classification; no original/game execution."""
from pathlib import Path
import hashlib
import struct
import sys
import tempfile
import unittest
import zlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from oracle_frame_retention import frame_manifest


class OracleFrames(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)/'reference';self.root.mkdir()
        self.raw=bytes(range(32));self.path=self.root/'world-000001.qz'
        self.body=struct.pack('>I',32)+zlib.compress(self.raw,3);self.path.write_bytes(self.body)
        self.index=dict(success=True,native_player_pixels_read=False,original_destination_capture_read=False,
                        worlds=[dict(generation='1',path=self.path.name,width=4,height=4)])

    def tearDown(self):self.temp.cleanup()

    def test_exact_encoded_and_raw_identity_without_duplicate_payload(self):
        rows=frame_manifest(self.root,self.index);self.assertEqual(len(rows),1)
        self.assertEqual(rows[0]['sha256'],hashlib.sha256(self.body).hexdigest())
        self.assertEqual(rows[0]['raw_sha256'],hashlib.sha256(self.raw).hexdigest())
        self.assertEqual(rows[0]['classification'],'reproducible_temporary')
        self.assertEqual(self.path.read_bytes(),self.body)

    def test_provenance_and_complete_history_required(self):
        for key,value in [('success',False),('native_player_pixels_read',True),('original_destination_capture_read',True),('worlds',[])]:
            with self.subTest(key=key),self.assertRaises(ValueError):frame_manifest(self.root,dict(self.index,**{key:value}))

    def test_generation_gap_duplicate_and_changed_name_refuse(self):
        row=self.index['worlds'][0]
        for changed in (dict(row,generation='2'),dict(row,path='other.qz')):
            with self.assertRaises(ValueError):frame_manifest(self.root,dict(self.index,worlds=[changed]))
        with self.assertRaises(ValueError):frame_manifest(self.root,dict(self.index,worlds=[row,row]))

    def test_path_escape_refuses(self):
        self.index['worlds'][0]['path']='../world-000001.qz'
        with self.assertRaises(ValueError):frame_manifest(self.root,self.index)

    def test_symlink_refuses(self):
        other=self.root/'held';self.path.rename(other);self.path.symlink_to(other)
        with self.assertRaises(ValueError):frame_manifest(self.root,self.index)

    def test_raw_extent_and_boolean_dimensions_refuse(self):
        for change in (dict(width=True),dict(height=0),dict(width=4096,height=4096),dict(width=5)):
            row=dict(self.index['worlds'][0],**change)
            with self.assertRaises(ValueError):frame_manifest(self.root,dict(self.index,worlds=[row]))

    def test_truncated_extra_and_wrong_envelope_refuse(self):
        for body in (self.body[:-1],self.body+b'extra',struct.pack('>I',31)+self.body[4:]):
            self.path.write_bytes(body)
            with self.assertRaises((ValueError,zlib.error)):frame_manifest(self.root,self.index)

    def test_decompression_is_bounded(self):
        self.path.write_bytes(struct.pack('>I',32)+zlib.compress(bytes(65536)))
        with self.assertRaises(ValueError):frame_manifest(self.root,self.index)

    def test_unindexed_plane_requires_review(self):
        (self.root/'world-000002.qz').write_bytes(self.body)
        with self.assertRaises(ValueError):frame_manifest(self.root,self.index)


if __name__=='__main__':unittest.main()
