import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_retained_map_source import decode


def pack(magic,size,rows,limits):
    h=[0]*16;h[:2]=struct.unpack('<2I',magic);h[2:5]=[1,size,0x40209ca7];h[5:5+len(limits)]=limits
    return struct.pack('<16I',*h)+b''.join(struct.pack('<'+'I'*(size//4),*r) for r in rows)


def fixture(closed=False):
    a=[0]*24;a[:5]=[1,1,1,1,18];b=a.copy();b[:2]=[2,2];b[10:12]=[1,1]
    t=[0]*24;t[:5]=[3,4,1,1,18];t[11]=1
    life=pack(b'MNMWLF01',96,[a,b,t],[131072,2048])
    g=[0]*32;g[:7]=[1,1,1,1,18,0,0];g[10:12]=[1,1];gr=g.copy();gr[:2]=[2,2];gr[8]=1;gr[14]=2
    m=[0]*32;m[:7]=[3,1,2,2,18,0,0];m[10]=2;mr=m.copy();mr[:2]=[4,2];mr[24:30]=[1600,960000,3,1,1,1]
    u=[0]*32;u[:7]=[5,1,3,3,18,0,0];u[10]=2;u[24:28]=[1600,960000,3,1];ur=u.copy();ur[:2]=[6,2]
    rt=[0]*32;rt[:5]=[7,4,1,3,18];rt[10]=3;rt[27]=1
    resources=pack(b'MNMWRM01',128,[g,gr,m,mr,u,ur,rt],[131072,2048])
    sh=[0]*16;sh[:2]=struct.unpack('<2I',b'MNMWIN02');sh[2:8]=[2,192,0x40209ca7,131072,32,2048]
    s=[0]*48;s[:7]=[1,1,1,1,18,0,0];s[10:12]=[4,1];s[28:30]=[1,2]
    sr=s.copy();sr[:2]=[2,2]
    st=[0]*48;st[:5]=[3,4,1,1,18];st[10]=4
    submissions=struct.pack('<16I',*sh)+b''.join(struct.pack('<48I',*r) for r in (s,sr,st))
    q=[0]*48;q[:16]=[1,3,1,18,0,1,1,0,2,2,1,3,1600,960000,not closed,1]
    q[16:24]=[6 if closed else 4,6 if closed else 4,2,2,10,11,1,0]
    q[24:40]=[64,1,19,7,0,0,8,800,600,800,4,4,0,0,18,0]
    qt=[0]*48;qt[:5]=[2,4,1,18,1]
    query=pack(b'MNMWQS01',192,[q,qt],[131072,2048])
    return query,resources,submissions,life


class RetainedMap(unittest.TestCase):
    def test_open_and_closed_are_points_without_ownership(self):
        for closed in (False,True):
            r=decode(*fixture(closed));self.assertEqual(r['matched'],1)
            self.assertEqual(r['matched_closed_maps'],int(closed))
            self.assertFalse(r['post_unmap_allocation_owned'])
            self.assertFalse(r['source_upload_identity_proved'])

    def test_changed_map_requires_explicit_unknown(self):
        q,*rest=fixture();q=bytearray(q)
        struct.pack_into('<I',q,64+17*4,6)
        with self.assertRaises(ValueError):decode(q,*rest)
        struct.pack_into('<I',q,64+15*4,2);struct.pack_into('<16I',q,64+24*4,*([0]*16))
        r=decode(q,*rest);self.assertEqual(r['unknown'],1)

    def test_wrong_epoch_extent_thread_prefix_and_reserve(self):
        for word,value in [(9,3),(10,2),(11,4),(14,0),(16,0),(18,0),(22,2),(23,1),(26,23),(33,799),(39,1),(40,1)]:
            q,*rest=fixture();q=bytearray(q);struct.pack_into('<I',q,64+word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):decode(q,*rest)

    def test_closure_and_refusal_cannot_admit_metadata(self):
        q,*rest=fixture();q=bytearray(q);struct.pack_into('<I',q,64+15*4,4)
        with self.assertRaises(ValueError):decode(q,*rest)
        q,*rest=fixture()
        with self.assertRaises(ValueError):decode(q[:-1],*rest)


if __name__=='__main__':unittest.main()
