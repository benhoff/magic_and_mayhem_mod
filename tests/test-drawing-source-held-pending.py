import struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_held_snapshot import decode


def journal(version=2,pending=2,admission=True):
    h=[0,0,version,192,0x40209ca7,2048,1024,1073741824]+[0]*8
    point=[64,1,19,7,0,0,1,800,600,800,1,0,pending,0,7,0]
    rows=[]
    for ordinal in (1,2):
        r=[0]*48;r[:4]=[ordinal,ordinal,10,7];r[5:7]=[2,3];r[8:11]=[ordinal,960000,1];r[11:13]=[0,0] if ordinal==1 else [2,2];r[13]=ordinal if admission else 0;r[16:32]=point;r[32:48]=point;rows.append(r)
    t=[0]*48;t[:3]=[3,4,1];t[8:10]=[2,1920000];rows.append(t)
    data=bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in rows));data[:8]=b'MNMSHB01';return data


class HeldPending(unittest.TestCase):
    def test_v2_requires_exact_admission_reference(self):
        r=decode(journal());self.assertEqual(r['wire_version'],2);self.assertEqual([f['pending_admission'] for f in r['frames']],[1,2]);self.assertFalse(r['source_storage_owned'])
        with self.assertRaises(ValueError):decode(journal(admission=False))

    def test_v1_retains_old_refusal(self):
        with self.assertRaises(ValueError):decode(journal(version=1))
        r=decode(journal(version=1,pending=1,admission=False));self.assertEqual(r['wire_version'],1)

    def test_v2_does_not_invent_extra_frame(self):
        with self.assertRaises(ValueError):decode(journal(pending=1))
        self.assertEqual(decode(journal(pending=1,admission=False))['admitted_planes'],2)

    def test_bad_version_gap_state_result_and_extent(self):
        with self.assertRaises(ValueError):decode(journal(version=3))
        for offset,value in [(64+7*4,2),(64+9*4,959999),(64+14*4,1),(64+29*4,1),(64+32*4,63)]:
            data=journal();struct.pack_into('<I',data,offset,value)
            with self.assertRaises(ValueError):decode(data)


if __name__=='__main__':unittest.main()
