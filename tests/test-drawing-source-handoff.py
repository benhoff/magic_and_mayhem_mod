import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_handoff import decode


def rows(client=False,crc=0x12345678):
    result=[]
    prefixes=[4,4,4,5,8,8,9]
    for phase in range(1,8):
        r=[0]*64;r[1:12]=[phase,10,1,2,3,4,2 if client else 1,40,40,80,3200]
        r[12]=crc if phase>=3 else 0;r[13]=5;r[14]=1;r[15]=prefixes[phase-1];r[17]=77 if client else 0;r[18]=8192 if client else 0;r[19]=0 if client else 1;r[20]=phase;result.append(r)
    terminal=[0]*64;terminal[1]=8;terminal[2]=10;terminal[14]=1;terminal[15]=9;terminal[16]=1;result.append(terminal)
    return result


def journal(records):
    h=[0]*16;h[2:7]=[1,256,0x40209ca7,131072,2048];raw=bytearray(struct.pack('<16I',*h));raw[:8]=b'MNMSHF01'
    for i,r in enumerate(records,1):r=list(r);r[0]=i;raw+=struct.pack('<64I',*r)
    return raw


class Handoff(unittest.TestCase):
    def test_complete_private_and_client_snapshot(self):
        for client in (False,True):
            result=decode(journal(rows(client)));self.assertEqual(result['handoffs'],1)
            self.assertFalse(result['original_aliases_retained_as_consumer_inputs'])
            self.assertFalse(result['original_pixels_used_as_native_inputs'])
            self.assertFalse(result['global_host_driver_alias_ownership'])

    def test_original_storage_cannot_be_the_private_owner(self):
        r=rows()
        for row in r[:-1]:row[19]=row[13]
        with self.assertRaises(ValueError):decode(journal(r))

    def test_zero_crc_is_a_value(self):self.assertEqual(decode(journal(rows(crc=0)))['bindings'][0]['crc32'],0)

    def test_missing_restore_or_completion(self):
        for index in (3,4,5,6,7):
            r=rows();del r[index]
            with self.assertRaises(ValueError):decode(journal(r))

    def test_changed_source_or_owner_identity(self):
        for field in (2,3,4,5,6,7,8,9,10,11,13,14,17,18,19):
            r=rows();r[4][field]+=1
            with self.assertRaises(ValueError):decode(journal(r))

    def test_callback_inside_snapshot(self):
        r=rows();r[2][15]+=1
        with self.assertRaises(ValueError):decode(journal(r))

    def test_value_change_after_transfer(self):
        r=rows();r[3][12]^=1
        with self.assertRaises(ValueError):decode(journal(r))

    def test_private_owner_reuse(self):
        first=rows();r=rows()
        for row in r[:-1]:row[3]=2;row[14]=2;row[15]+=10
        r[-1][14]=2;r[-1][15]+=10
        with self.assertRaises(ValueError):decode(journal(first[:-1]+r))

    def test_refusal_terminal_count_and_reserved(self):
        for index,field,value in ((2,1,9),(7,14,2),(2,21,1),(2,16,1)):
            r=rows();r[index][field]=value
            with self.assertRaises(ValueError):decode(journal(r))


if __name__=='__main__':unittest.main()
