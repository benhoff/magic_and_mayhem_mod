#!/usr/bin/env python3
"""Fresh native music admission and immutable prepared-asset staging refusals."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import threading
import time
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import native_music_launch as launch
from mnm_protocols import music_v1 as wire
spec=importlib.util.spec_from_file_location('fixture',ROOT/'tools/test-native-music-service.py');fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)

class Admission(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.base=Path(self.temp.name);self.root=self.base/'session';self.root.mkdir()
        source=self.base/'catalog';fixture.fixture(source);self.metadata=launch.stage(self.root,source)
        (self.root/'game').mkdir();dll=self.root/'game/MnmAudio.dll';dll.write_bytes(b'fixture dll')
        self.metadata.update(native_voices=True,native_render=True,origin='menu_action_bridge',audio_dll_sha256=launch.sha(dll))
        self.channel=self.root/'native-music.bin';data=bytearray(wire.SIZE);data[:8]=wire.MAGIC.encode() if isinstance(wire.MAGIC,str) else wire.MAGIC
        for offset,value in [(8,wire.VERSION),(12,wire.SIZE),(wire.RESPONSE_SEQUENCE_OFFSET,2),(wire.HEARTBEAT_OFFSET,1),(wire.READY_OFFSET,1),(wire.PHASE_OFFSET,wire.PHASE_STOPPED),(wire.VOLUME_OFFSET,65535),(wire.TRACK_COUNT_OFFSET,3),(wire.LEADOUT_SECTOR_OFFSET,152)]:struct.pack_into('<I',data,offset,value)
        struct.pack_into('<6I',data,wire.TOC_OFFSET,0,2,2,77,77,152);self.channel.write_bytes(data)
    def test_ready(self):
        env={'MNM_MUSIC_CHANNEL':'inherited'};launch.authorize(self.root,self.metadata,env)
        self.assertEqual(env['MNM_MUSIC_CHANNEL'],'Z:'+str(self.channel).replace('/','\\'))
    def test_disabled_scrubs(self):
        env={'MNM_MUSIC_CHANNEL':'inherited'};launch.authorize(self.root,{},env);self.assertNotIn('MNM_MUSIC_CHANNEL',env)
    def test_metadata_refusals(self):
        for name,value in [('native_music',1),('native_voices',False),('native_render',False),('origin','observation'),('music_channel','elsewhere'),('music_catalog','elsewhere'),('audio_dll_sha256','0'*64),('music_inputs',{}),('music_toc',[])]:
            with self.subTest(name=name):
                metadata=copy.deepcopy(self.metadata);metadata[name]=value
                with self.assertRaises(ValueError):launch.authorize(self.root,metadata,{})
    def test_channel_refusals(self):
        original=self.channel.read_bytes()
        for offset,value in [(8,99),(12,123),(wire.READY_OFFSET,0),(wire.HEARTBEAT_OFFSET,0),(wire.REQUEST_ID_OFFSET,1),(wire.PRODUCER_FEATURES_OFFSET,8),(wire.RESPONSE_SEQUENCE_OFFSET,3),(wire.PHASE_OFFSET,wire.PHASE_PLAYING),(wire.VOLUME_OFFSET,0),(wire.TRACK_COUNT_OFFSET,4),(wire.TOC_OFFSET+8,3),(wire.SIZE-4,1)]:
            with self.subTest(offset=offset):
                data=bytearray(original);struct.pack_into('<I',data,offset,value);self.channel.write_bytes(data)
                with self.assertRaises(ValueError):launch.authorize(self.root,self.metadata,{})
        self.channel.write_bytes(original[:-1])
        with self.assertRaises(ValueError):launch.authorize(self.root,self.metadata,{})
    def test_catalog_drift(self):
        track=self.root/'native-music/Track02.wav';track.write_bytes(track.read_bytes()[:-1])
        with self.assertRaises(ValueError):launch.authorize(self.root,self.metadata,{})
    def test_ordinary_launch_does_not_hash_pcm(self):
        metadata={**self.metadata,'full_asset_verification':False}
        original=launch.sha;hashed=[]
        def observe(path):
            hashed.append(path)
            return original(path)
        with patch.object(launch,'sha',side_effect=observe):
            launch.authorize(self.root,metadata,{})
        self.assertTrue(hashed)
        self.assertFalse(any(path.suffix=='.wav' for path in hashed))
        (self.root/'native-music/Track02.wav').unlink()
        with self.assertRaises(ValueError):launch.authorize(self.root,metadata,{})
    def test_fast_staging_retains_catalog_and_policy(self):
        fresh=self.base/'fast';fresh.mkdir()
        original=launch.sha;hashed=[]
        def observe(path):hashed.append(path);return original(path)
        with patch.dict(os.environ,{'MNM_VERIFY_ASSETS':'0'}),patch.object(launch,'sha',side_effect=observe):
            metadata=launch.stage(fresh,self.root/'native-music')
        self.assertIs(metadata['full_asset_verification'],False)
        self.assertFalse(any(path.suffix=='.wav' for path in hashed))
        self.assertEqual(metadata['music_toc'],self.metadata['music_toc'])
    def test_invalid_scan_policy_refused(self):
        with self.assertRaises(ValueError):launch.authorize(self.root,{**self.metadata,'full_asset_verification':0},{})
    def test_symlink(self):
        borrowed=self.base/'borrowed.bin';self.channel.rename(borrowed);self.channel.symlink_to(borrowed)
        with self.assertRaises(ValueError):launch.authorize(self.root,self.metadata,{})
    def test_staging_refuses_existing(self):
        with self.assertRaises(ValueError):launch.stage(self.root,self.root/'native-music')
    def test_transient_snapshot(self):
        with self.channel.open('r+b') as source:source.seek(wire.RESPONSE_SEQUENCE_OFFSET);source.write(struct.pack('<I',3))
        def settle():
            time.sleep(.025)
            with self.channel.open('r+b') as source:source.seek(wire.RESPONSE_SEQUENCE_OFFSET);source.write(struct.pack('<I',4))
        peer=threading.Thread(target=settle);peer.start()
        try:launch.authorize(self.root,self.metadata,{})
        finally:peer.join(timeout=1)

if __name__=='__main__':unittest.main()
