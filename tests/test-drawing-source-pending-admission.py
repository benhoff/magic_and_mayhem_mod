import struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_pending_admission import decode


def record():
    h=[0,0,1,64,0x40209ca7,128,2,17,61,392,0,176235,30,2,3,1]
    selected=[1,1,7,392,0,0x6ba0d,61,6,2,61,1,0,1,1,0,0]
    foreign=[2,2,32,432,0,0xa4b01,5,6,1,5,0,0,0,1,0,0]
    data=bytearray(struct.pack('<48I',*h,*selected,*foreign));data[:8]=b'MNMSPA01';return data


class Admission(unittest.TestCase):
    def test_exact_diagnostic_does_not_assert_alias_ownership(self):
        r=decode(record());self.assertTrue(r['source_copy_admitted']);self.assertFalse(r['disjoint_backing_proved']);self.assertFalse(r['source_ownership_proved']);self.assertTrue(r['page_protection_requires_independent_lease_evidence'])

    def test_versioned_copy_route(self):
        data=record();struct.pack_into('<I',data,128+8,39)
        with self.assertRaises(ValueError):decode(data)
        struct.pack_into('<I',data,8,2);self.assertEqual(decode(data)['wire_version'],2)
        struct.pack_into('<I',data,128+8,33)
        with self.assertRaises(ValueError):decode(data)

    def test_policy_and_envelope(self):
        for offset,value in [(14,1),(15,0),(6,1),(13,1),(7,0)]:
            data=record();struct.pack_into('<I',data,offset*4,value)
            with self.assertRaises(ValueError):decode(data)
        for data in (record()[:-1],record()+bytes(64)):
            with self.assertRaises(ValueError):decode(data)

    def test_reserved_primitive_owner_unknown_shape(self):
        for index,value in [(2,1),(3,392),(3,0),(4,1),(6,7),(7,4),(8,0),(9,0),(13,0),(14,1)]:
            data=record();struct.pack_into('<I',data,128+index*4,value)
            with self.assertRaises(ValueError):decode(data)

    def test_same_source_and_duplicate_selected(self):
        for index,value in [(8,61),(9,61)]:
            data=record();struct.pack_into('<I',data,128+index*4,value)
            with self.assertRaises(ValueError):decode(data)
        data=record();data[128:192]=data[64:128];struct.pack_into('<II',data,128,2,2)
        with self.assertRaises(ValueError):decode(data)


if __name__=='__main__':unittest.main()
