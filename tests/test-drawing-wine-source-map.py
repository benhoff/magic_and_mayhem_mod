from pathlib import Path
import struct,sys,tempfile,unittest,zlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_source_map import decode


def wire(directory):
    h=[0]*16;h[2:]=[1,256,0x40209ca7,16384,16,2,4,0x80000000,116,3635200,40,40,400,280]
    s=[0]*64;s[:19]=[1,1,1,3,4,0,5,1,0,7,0,0,0,123,0,456,2,2,1]
    s[19:29]=[116,0,0,0,0,13,40,40,1,3200];s[29:39]=[84,3360,1,3200,1,4,1,0,0x80000000,0]
    payload=bytes((x*3+y*5+7)&255 for y in range(40) for x in range(80));s[36]=zlib.crc32(payload)
    (directory/'wine-source-map-000001.565').write_bytes(payload)
    t=[0]*64;t[:7]=[2,4,1,1,1,1,0]
    return h,s,t


def encode(parts):
    raw=bytearray(b''.join(struct.pack('<'+str(len(p))+'I',*p) for p in parts));raw[:8]=b'MNMWMS01';return bytes(raw)


class MappedSources(unittest.TestCase):
    def setUp(self):self.temp=tempfile.TemporaryDirectory();self.directory=Path(self.temp.name)
    def tearDown(self):self.temp.cleanup()
    def test_owned_dense_payload(self):
        p=wire(self.directory);r=decode(encode(p),self.directory);self.assertEqual(r['retained_payloads'],1);self.assertEqual(r['map_attempts'],[1,0]);self.assertFalse(r['all_source_storage_owned']);self.assertFalse(r['every_submission_pixels_retained'])
    def test_requires_closed_nonfatal_stream(self):
        p=wire(self.directory);raw=encode(p)
        for bad in [raw[:-1],raw[:320],raw+b'\0',b'WRONGHDR'+raw[8:]]:
            with self.assertRaises(ValueError):decode(bad,self.directory)
        for status in (8,9,10):
            p=wire(self.directory);p[1][10]=status
            with self.assertRaises(ValueError):decode(encode(p),self.directory)
    def test_release_read_flags_attempt_and_pitch(self):
        for field,value in [(2,0),(4,0),(14,0x8876086c),(29,79),(29,32769),(31,2),(31,17),(34,3),(37,0x40000000),(39,1)]:
            p=wire(self.directory);p[1][field]=value
            with self.assertRaises(ValueError):decode(encode(p),self.directory)
    def test_format_access_geometry_generation(self):
        for field,value in [(7,0),(9,0),(17,1),(18,2),(19,117),(20,1),(24,14),(25,41),(27,2),(35,0)]:
            p=wire(self.directory);p[1][field]=value
            with self.assertRaises(ValueError):decode(encode(p),self.directory)
    def test_payload_corruption_truncation_and_extra_file(self):
        for raw in [b'',b'\0'*3200,b'\0'*3201]:
            p=wire(self.directory);(self.directory/'wine-source-map-000001.565').write_bytes(raw)
            with self.assertRaises(ValueError):decode(encode(p),self.directory)
        p=wire(self.directory);(self.directory/'wine-source-map-000002.565').write_bytes(b'')
        with self.assertRaises(ValueError):decode(encode(p),self.directory)
    def test_terminal_counts_and_refused_payload(self):
        for field,value in [(0,3),(2,0),(3,2),(4,0),(5,0),(7,1)]:
            p=wire(self.directory);p[2][field]=value
            with self.assertRaises(ValueError):decode(encode(p),self.directory)
        p=wire(self.directory);p[1][10]=6
        with self.assertRaises(ValueError):decode(encode(p),self.directory)
    def test_two_dimensional_slice_pitch_is_not_assumed(self):
        p=wire(self.directory);p[1][30]=0;self.assertTrue(decode(encode(p),self.directory)['success'])

    def test_bounded_cpu_geometry_and_configuration(self):
        p=wire(self.directory);p[0][2:]=[2,256,0x40209ca7,16384,8,1,4,0x80000000,116,4194304,800,600,0,0];p[1][24]=14;p[2][6]=3200
        raw=bytearray(encode(p));raw[:8]=b'MNMWMS02';r=decode(bytes(raw),self.directory)
        self.assertEqual(r['wire_version'],2);self.assertEqual(r['map_attempts'],[1]);self.assertTrue(r['every_observed_submission_pixels_retained'])
        for field,value in [(6,0),(6,8193),(11,0),(11,2147483649),(12,801),(14,1)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,field*4,value)
            with self.assertRaises(ValueError):decode(bytes(bad),self.directory)
        for field,value in [(24,3),(25,801),(26,601),(28,3199),(35,2)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,64+field*4,value)
            with self.assertRaises(ValueError):decode(bytes(bad),self.directory)
        bad=bytearray(raw);struct.pack_into('<I',bad,11*4,3199)
        with self.assertRaises(ValueError):decode(bytes(bad),self.directory)
        bad=bytearray(raw);struct.pack_into('<I',bad,64+256+6*4,3199)
        with self.assertRaises(ValueError):decode(bytes(bad),self.directory)

    def test_version_three_padded_descriptor_storage(self):
        p=wire(self.directory);p[0][2:]=[3,256,0x40209ca7,16384,8,1,4,0x80000000,116,4194304,800,600,0,0];p[1][28]=3360;p[2][6]=3200
        raw=bytearray(encode(p));raw[:8]=b'MNMWMS03';self.assertEqual(decode(bytes(raw),self.directory)['retained_bytes'],3200)
        for size in (3199,40*32768+1):
            bad=bytearray(raw);struct.pack_into('<I',bad,64+28*4,size)
            with self.assertRaises(ValueError):decode(bytes(bad),self.directory)
        raw[:8]=b'MNMWMS02';struct.pack_into('<I',raw,2*4,2)
        with self.assertRaises(ValueError):decode(bytes(raw),self.directory)

    def test_owned_payload_cannot_be_a_symlink(self):
        p=wire(self.directory);payload=self.directory/'wine-source-map-000001.565';target=self.directory/'outside.565';payload.rename(target);payload.symlink_to(target)
        with self.assertRaises(ValueError):decode(encode(p),self.directory)


if __name__=='__main__':unittest.main()
