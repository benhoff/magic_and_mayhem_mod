import json,struct,sys,tempfile,unittest
from hashlib import sha256
from pathlib import Path
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_egl_destination import decode,decode_rebind,decode_xcb,attach,plane,compare_frame


class Cases(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.directory=Path(self.temp.name)
        r=[0]*32;r[2:17]=[1,128,4,20,30,2,2,0,1,0x8ce0,0x404,0x405,8,1,4];r[22]=1;r[24]=2;r[26:28]=[1,1]
        self.raw=b'MNMEDB01'+struct.pack('<30I',*r[2:]);self.metadata=self.directory/'egl-destination-before-swap-000004.bin';self.metadata.write_bytes(self.raw)
        self.words=np.array([[0xf800,0x7e0],[31,0xffff]],dtype='<u2');self.pixels=self.directory/'egl-destination-before-swap-000004.565';self.pixels.write_bytes(self.words[::-1].tobytes())
        self.row=dict(ordinal=1,call=4,pid=20,thread=30,width=2,height=2,begin_ns=3)
        attach(self.directory,self.row)

    def test_orientation_and_restoration(self):
        np.testing.assert_array_equal(plane(self.directory,self.row),self.words)
        self.assertTrue(self.row['gpu_destination']['restored']);self.assertEqual(self.row['gpu_destination']['default_read_buffer'],0x404)

    def test_malformed_or_unrestored_state_refuses(self):
        for index,value in [(2,2),(3,127),(4,0),(7,2049),(9,1),(13,0x408),(14,9),(15,0),(16,3),(17,1),(21,1),(24,0),(26,0),(27,0),(28,1)]:
            raw=bytearray(self.raw);struct.pack_into('<I',raw,index*4,value)
            with self.assertRaises(ValueError):decode(raw)

    def test_frame_identity_and_time_refuse(self):
        for key,value in [('thread',31),('width',3),('begin_ns',1)]:
            row=dict(self.row);row[key]=value
            with self.assertRaises(ValueError):attach(self.directory,row)

    def test_pixels_or_metadata_changed_refuse(self):
        self.pixels.write_bytes(b'bad')
        with self.assertRaises(ValueError):plane(self.directory,self.row)
        self.pixels.write_bytes(self.words[::-1].tobytes());self.metadata.write_bytes(self.raw[:-1])
        with self.assertRaises(ValueError):plane(self.directory,self.row)

    def test_path_escape_refuses(self):
        self.row['gpu_destination']['pixels_path']='../outside.565'
        with self.assertRaises(ValueError):plane(self.directory,self.row)

    def test_split_destination_from_physical_failure(self):
        words=self.words;rgba=np.zeros((2,2,4),dtype=np.uint8);rgba[:,:,0]=(words>>11)*255//31;rgba[:,:,1]=((words>>5)&63)*255//63;rgba[:,:,2]=(words&31)*255//31
        original=dict(egl_swap_completion_observation=dict(frames=[self.row]))
        result=compare_frame(original,self.directory,1,words.tobytes(),rgba.tobytes(),2,2);self.assertEqual(result['native_destination_mismatching_words'],0);self.assertEqual(result['destination_physical_mismatching_pixels'],0)
        rgba[0,0,:3]=0;result=compare_frame(original,self.directory,1,words.tobytes(),rgba.tobytes(),2,2);self.assertEqual(result['native_destination_mismatching_words'],0);self.assertEqual(result['destination_physical_mismatching_pixels'],1)

    def test_rebind_applied_and_noop_records(self):
        r=[0]*24;r[2:8]=[1,96,4,20,30,0];raw=b'MNMERB01'+struct.pack('<22I',*r[2:]);self.assertFalse(decode_rebind(raw)['applied'])
        r[7:15]=[1,2,800,600,640,480,1,1];raw=b'MNMERB01'+struct.pack('<22I',*r[2:]);self.assertTrue(decode_rebind(raw)['state_preserved'])
        for index,value in [(8,3),(9,0),(11,2049),(13,0),(14,0),(15,1)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
            with self.assertRaises(ValueError):decode_rebind(bad)
    def test_release_requires_explicit_success(self):
        r=[0]*24;r[2:17]=[2,96,4,20,30,1,1,0,0,640,480,1,1,2,1];raw=b'MNMERB01'+struct.pack('<22I',*r[2:]);self.assertTrue(decode_rebind(raw)['context_released'])
        for index,value in [(15,3),(16,0),(17,1)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
            with self.assertRaises(ValueError):decode_rebind(bad)
    def test_xcb_packet_geometry_and_cookie(self):
        r=[0]*32;r[2:28]=[1,128,4,20,30,1,1,640,480,9,800,600,0,120,640,480,0,0,24,2,0,0,0,7,0,99];r[28]=1;r[30]=2;raw=b'MNMEXI01'+struct.pack('<30I',*r[2:]);v=decode_xcb(raw);self.assertEqual(v['total_height'],600);self.assertEqual(v['source_y'],120);self.assertEqual(v['cookie_sequence'],99)
        for index,value in [(2,2),(7,1025),(8,0),(9,0),(13,65536),(18,65536),(20,256),(30,0)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
            with self.assertRaises(ValueError):decode_xcb(bad)
    def test_crop_packet_requires_exact_geometry_and_preserves_other_fields(self):
        r=[0]*60;r[2:28]=[3,240,4,20,30,1,1,640,480,9,832,640,0,120,640,480,0,0,24,2,0,0,0,7,0,99];r[28]=1;r[30]=2;r[32:38]=[800,600,9,1,1,120];r[40:56]=[9,832,640,0,0,832,640,0,0,24,2,0,0,0,7,0];r[56:60]=[832,640,1,1];raw=b'MNMEXI01'+struct.pack('<58I',*r[2:]);self.assertTrue(decode_xcb(raw)['resize_crop_applied'])
        for index,value in [(15,119),(16,639),(33,480),(34,0),(36,0),(38,1),(54,8),(56,640),(58,0),(59,0)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
            with self.assertRaises(ValueError):decode_xcb(bad)
    def test_missing_optional_witness_is_not_a_match(self):
        self.assertIsNone(compare_frame({},self.directory,1,b'',b'',2,2))


if __name__=='__main__':unittest.main()
