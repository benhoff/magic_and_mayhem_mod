from pathlib import Path
import copy,struct,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_version import decode


def fixture(alias=False):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSVG01');h[2:8]=[1,128,0x40209ca7,131072,2048,4]
    rows=[]
    def add(phase):
        r=[0]*32;r[0]=len(rows)+1;r[1]=phase;r[2]=5;r[3]=100+len(rows);rows.append(r);return r
    def snapshot(r):
        r[4:14]=[1,2,3,4,1,0,1,1,0,1];r[22]=4
    s=add(3);snapshot(s)
    p=add(4);snapshot(p);p[14]=9
    w=add(5);snapshot(w);w[14:16]=[9,10];w[19:22]=[1 if alias else 2,1,0];w[23]=int(alias)
    a=add(1);a[8]=w[19];a[11:13]=[2,1];a[18]=3
    b=add(2);b[8]=w[19];b[11:13]=[3,0];b[18]=3
    x=add(6);snapshot(x);x[14:16]=[9,10];x[19:22]=[w[19],3,0];x[23]=int(alias);x[11]=3 if alias else 1
    t=add(7);t[4]=1;t[13]=1
    return h,rows


def encode(h,rows):return struct.pack('<16I',*h)+b''.join(struct.pack('<32I',*r) for r in rows)


class SourceVersion(unittest.TestCase):
    def test_independent_epoch_flow_and_limits(self):
        h,r=fixture();x=decode(encode(h,r));self.assertTrue(x['all_observed_snapshot_workers_checked']);self.assertEqual(x['bindings'][0]['command'],9)
        self.assertFalse(x['original_source_atomicity_proved']);self.assertFalse(x['gpu_completion_proved']);self.assertFalse(x['unobserved_direct_writes_accounted'])
    def test_same_resource_own_destination_is_explicit(self):
        h,r=fixture(True);x=decode(encode(h,r));self.assertTrue(x['bindings'][0]['aliased_destination']);self.assertEqual(x['bindings'][0]['return_epoch'],3)
    def test_epoch_or_active_writer_changes_refuse(self):
        for idx,word,value in [(0,11,2),(2,11,2),(2,12,1),(5,11,2),(4,12,1),(3,11,3)]:
            h,r=fixture();r[idx][word]=value
            with self.subTest(idx=idx,word=word),self.assertRaises(ValueError):decode(encode(h,r))
    def test_generation_subresource_payload_and_command_changes_refuse(self):
        for idx,word,value in [(1,7,5),(2,9,1),(2,13,2),(2,14,11),(5,15,11),(5,22,5)]:
            h,r=fixture();r[idx][word]=value
            with self.assertRaises(ValueError):decode(encode(h,r))
    def test_missing_return_or_unclosed_writes_refuse(self):
        h,r=fixture();r.pop(-2)
        for i,row in enumerate(r,1):row[0]=i
        with self.assertRaises(ValueError):decode(encode(h,r))
        h,r=fixture();r[-2][1]=8;r[-2][16]=9
        with self.assertRaises(ValueError):decode(encode(h,r))
    def test_envelope_terminal_reserved_and_duplicate_identity(self):
        for idx,word,value in [(0,24,1),(1,1,3),(-1,13,2),(-1,0,3)]:
            h,r=fixture();r[idx][word]=value
            with self.assertRaises(ValueError):decode(encode(h,r))
        h,r=fixture();h[7]=3
        with self.assertRaises(ValueError):decode(encode(h,r))

    def test_open_write_at_boundary_is_retained_after_all_reads_complete(self):
        h,r=fixture();h[:2]=struct.unpack('<2I',b'MNMSVG02');h[2]=2
        end=r.pop();a=[0]*32;a[1]=1;a[2]=5;a[8]=1;a[11:13]=[2,1];a[18]=1
        boundary=[0]*32;boundary[1]=9;boundary[2]=5;boundary[8]=1;boundary[11:13]=[2,1]
        end[14:16]=[1,1];r += [a,boundary,end]
        for i,row in enumerate(r,1):row[0]=i
        x=decode(encode(h,r));self.assertTrue(x['all_observed_snapshot_workers_checked']);self.assertFalse(x['all_global_write_leases_closed']);self.assertEqual(x['open_write_leases'][0]['writers'],1)
        r[-1][14]=0
        with self.assertRaises(ValueError):decode(encode(h,r))

    def test_mutation_end_kind_must_match_a_begin(self):
        h,r=fixture();r[4][18]=4
        with self.assertRaises(ValueError):decode(encode(h,r))


if __name__=='__main__':unittest.main()
