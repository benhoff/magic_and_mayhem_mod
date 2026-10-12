#!/usr/bin/env python3
"""Native media channel ownership/admission; generated inputs, no decoder/game."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from native_media_launch import authorize,media_v1

class AdmissionTests(unittest.TestCase):
    def setUp(self):
        temporary=tempfile.TemporaryDirectory();self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name);self.path=self.root/'native-media.bin'
        self.data=bytearray(media_v1.SIZE);self.data[:media_v1.HEADER_SIZE]=media_v1.initial_header()
        struct.pack_into('<I',self.data,520,1)
        self.metadata={'origin':'menu_action_bridge','native_render':True,'native_media':True,'media_channel':str(self.path)}
        self.write()
    def write(self):self.path.write_bytes(self.data)
    def test_owned_channel_replaces_inherited_path(self):
        env={'MNM_RENDER_MEDIA':'unapproved'};authorize(self.root,self.metadata,env)
        self.assertEqual(env['MNM_RENDER_MEDIA'],'Z:'+str(self.path).replace('/','\\'))
        authorize(self.root,{},env);self.assertNotIn('MNM_RENDER_MEDIA',env)
    def test_stale_or_unstarted_channel_refused(self):
        for offset,value in [(8,2),(12,0),(16,1),(20,1),(24,1),(512,1),(516,1),(520,0),(524,1),(528,2),(532,1)]:
            with self.subTest(offset=offset):
                previous=struct.unpack_from('<I',self.data,offset)[0];struct.pack_into('<I',self.data,offset,value);self.write()
                with self.assertRaises(ValueError):authorize(self.root,self.metadata,{})
                struct.pack_into('<I',self.data,offset,previous)
    def test_redirected_staging_refused(self):
        for key,value in [('native_media',1),('native_render',False),('origin','menu_observation_only'),('media_channel',str(self.root/'other.bin'))]:
            with self.subTest(key=key),self.assertRaises(ValueError):authorize(self.root,{**self.metadata,key:value},{})
    def test_symlink_and_wrong_size_refused(self):
        other=self.root/'other.bin';self.path.rename(other);self.path.symlink_to(other)
        with self.assertRaises(ValueError):authorize(self.root,self.metadata,{})
        self.path.unlink();self.path.write_bytes(b'MNMMED01')
        with self.assertRaises(ValueError):authorize(self.root,self.metadata,{})

if __name__=='__main__':unittest.main()
