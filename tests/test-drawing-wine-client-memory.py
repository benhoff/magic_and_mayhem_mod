import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_client_memory import decode
from drawing_api_session import sha


def fixture():
    h=[0]*16;h[2:7]=[1,192,0x40209ca7,131072,2048]
    m=[64,1,83,7,0,0,8,800,600,800,4,4,0,0,18,0]
    e=[0]*48;e[:16]=[1,1,1,1,18,9,1,10,0,0,2,3,0,1600,1,0];e[16:32]=m
    r=e.copy();r[0]=2;r[1]=2;r[15]=1;r[32:48]=m
    q=[0]*48;q[:16]=[3,3,1,2,18,9,1,11,0,0,2,3,0,1600,1,1];q[16:32]=m
    t=[0]*48;t[:15]=[4,4,1,1,18,1,0,12,0,0,1,0,0,0,1]
    raw=bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*x) for x in (e,r,q,t)));raw[:8]=b'MNMWCM01'
    entry=[0]*48;entry[1:4]=[1,1,1];entry[13]=0
    wire=bytes(64)+struct.pack('<48I',*entry)+bytes(192)
    obs=dict(journal_sha256=sha(wire),calls=[dict(call=1,method=1,thread=18,api_entry=9,api_depth=1,second_object=2,second_generation=3)])
    return raw,obs,wire


class ClientBinding(unittest.TestCase):
    def test_known_pitch_only_source(self):
        x=decode(*fixture());self.assertEqual(x['known_source_points'],1)
        self.assertFalse(x['source_upload_identity_proved'])
        self.assertFalse(x['source_ownership_proved'])

    def test_stale_changed_generation_or_submission(self):
        for word,value in [(11,4),(12,1),(15,2),(18,19),(21,1),(25,799)]:
            raw,obs,wire=fixture();struct.pack_into('<I',raw,64+2*192+word*4,value)
            with self.assertRaises(ValueError):decode(raw,obs,wire)

    def test_failed_setter_cannot_bind(self):
        raw,obs,wire=fixture();struct.pack_into('<I',raw,64+192+8*4,0x8876086c)
        with self.assertRaises(ValueError):decode(raw,obs,wire)

    def test_absent_source_is_explicit_unknown(self):
        raw,obs,wire=fixture()
        obs['calls'][0]['second_object']=0;obs['calls'][0]['second_generation']=0
        start=64+2*192
        for word in (10,11):struct.pack_into('<I',raw,start+word*4,0)
        for word in range(13,48):struct.pack_into('<I',raw,start+word*4,0)
        result=decode(raw,obs,wire)
        self.assertEqual(result['unknown_source_points'],1)

    def test_missing_point_or_changed_raw_submission(self):
        raw,obs,wire=fixture()
        with self.assertRaises(ValueError):decode(raw[:-1],obs,wire)
        with self.assertRaises(ValueError):decode(raw,obs,wire+b'x')
        obs['calls'][0]['call']=2
        with self.assertRaises(ValueError):decode(raw,obs,wire)


if __name__=='__main__':unittest.main()
