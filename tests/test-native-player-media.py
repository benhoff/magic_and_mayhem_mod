#!/usr/bin/env python3
"""Reject incomplete, cancelled and incorrectly admitted default intro evidence."""
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_media_acceptance import validate_media,media_v1

class MediaAcceptance(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        self.rows=[dict(operation=4,status=3,native_pipeline_available=True)]
        self.rows.extend(dict(operation=1,request=i+1,asset=f'Intro{i}.avi',status=3,accepted=True,
            video_frames=4,presented_video_frames=4,audio_frames=20,audible_output_requested=True,
            audio_device_available=True,error='') for i in range(2))
        (self.root/'native-audio-launch-policy.json').write_text(json.dumps(dict(native_media_startup_probe=True,wine_directsound_disabled=True)))
        data=bytearray(media_v1.SIZE);data[:media_v1.HEADER_SIZE]=media_v1.initial_header()
        (self.root/'native-media.bin').write_bytes(data)

    def check(self,**options):
        (self.root/'native-media-report.json').write_text(json.dumps(self.rows))
        return validate_media(self.root,native_factory=True,require_intro_completion=True,**options)

    def test_complete_intros(self):
        self.assertTrue(self.check()['intro_completion']['verified'])

    def test_one_intro_does_not_prove_both(self):
        self.rows.pop()
        with self.assertRaisesRegex(ValueError,'intros'):self.check()

    def test_cancelled_movie_is_not_natural_completion(self):
        self.rows[1]['status']=4
        with self.assertRaisesRegex(ValueError,'intros'):self.check()

    def test_decoded_but_not_presented(self):
        self.rows[1]['presented_video_frames']=0
        with self.assertRaisesRegex(ValueError,'intros'):self.check()

    def test_producer_cancel_after_complete_report(self):
        p=self.root/'native-media.bin';data=bytearray(p.read_bytes());struct.pack_into('<I',data,48,1);p.write_bytes(data)
        with self.assertRaisesRegex(ValueError,'cancelled'):self.check()

    def test_corrupt_or_truncated_retirement_channel(self):
        p=self.root/'native-media.bin';data=p.read_bytes()
        for value in (b'badmagic'+data[8:],data[:64]):
            with self.subTest(bytes=len(value)):
                p.write_bytes(value)
                with self.assertRaisesRegex(ValueError,'channel'):self.check()

    def test_silence_needs_explicit_admission_and_consumed_pcm(self):
        for row in self.rows[1:]:row.update(audio_device_available=False,initial_silent_output=True)
        with self.assertRaises(ValueError):self.check()
        self.assertTrue(self.check(allow_initial_silence=True)['intro_completion']['verified'])
        self.rows[1]['audio_frames']=0
        with self.assertRaises(ValueError):self.check(allow_initial_silence=True)

if __name__=='__main__':unittest.main()
