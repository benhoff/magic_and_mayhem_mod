#!/usr/bin/env python3
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import wave
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from cd_audio import extract,parse_cue,timecode,SECTOR_BYTES

CUE='FILE "disc.img" BINARY\n TRACK 01 MODE1/2352\n INDEX 01 00:00:00\n TRACK 02 AUDIO\n INDEX 00 00:00:01\n INDEX 01 00:00:02\n TRACK 03 AUDIO\n INDEX 00 00:00:04\n INDEX 01 00:00:05\n'
class CdAudioTests(unittest.TestCase):
    def test_exact_track_spans_endian_and_read_only_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);cue=root/'disc.cue';image=root/'disc.img'
            cue.write_text(CUE);pcm=b''.join(struct.pack('<hh',i,-i) for i in range(8*588));image.write_bytes(pcm)
            before=image.read_bytes();report=extract(cue,root/'out')
            self.assertEqual(image.read_bytes(),before);self.assertTrue(report['inputs_unchanged'])
            self.assertEqual([(x['start_sector'],x['end_sector']) for x in report['tracks']],[(2,5),(5,8)])
            for row in report['tracks']:
                expected=pcm[row['start_sector']*SECTOR_BYTES:row['end_sector']*SECTOR_BYTES]
                with wave.open(str(root/'out'/row['file']),'rb') as audio:
                    self.assertEqual((audio.getnchannels(),audio.getsampwidth(),audio.getframerate()),(2,2,44100))
                    self.assertEqual(audio.readframes(audio.getnframes()),expected)
                self.assertEqual(row['pcm_sha256'],hashlib.sha256(expected).hexdigest())
            with self.assertRaises(FileExistsError):extract(cue,root/'out')
    def test_timecodes(self):
        self.assertEqual(timecode('01:02:03'),4653)
        for value in ('00:60:00','00:00:75','-1:00:00','0:0:0'):
            with self.assertRaises(ValueError):timecode(value)
    def test_unpadded_track_and_index_numbers(self):
        self.assertEqual(parse_cue(CUE,8*2352),parse_cue(CUE.replace('TRACK 0','TRACK ').replace('INDEX 0','INDEX '),8*2352))
    def test_rejections(self):
        for text,size in [(CUE,0),(CUE,1),(CUE.replace('00:00:05','00:00:08'),8*2352),
                          (CUE.replace('00:00:05','00:00:02'),8*2352),
                          (CUE.replace('TRACK 03','TRACK 04'),8*2352),
                          (CUE.replace('disc.img','../disc.img'),8*2352),
                          (CUE.replace('BINARY','WAVE'),8*2352),
                          (CUE+' FLAGS DCP\n',8*2352),
                          (CUE.replace('INDEX 01 00:00:05','INDEX 00 00:00:05'),8*2352)]:
            with self.subTest(text=text,size=size),self.assertRaises(ValueError):parse_cue(text,size)
    def test_symlink_image_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'real.img').write_bytes(bytes(8*2352));(root/'disc.img').symlink_to('real.img');(root/'disc.cue').write_text(CUE)
            with self.assertRaises(ValueError):extract(root/'disc.cue',root/'out')
            self.assertFalse((root/'out').exists())
if __name__=='__main__':unittest.main()
