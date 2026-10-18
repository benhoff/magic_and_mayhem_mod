#!/usr/bin/env python3
"""Synthetic PE controls for caller-boundary analysis, without game artifacts."""
import importlib.util
from pathlib import Path
import struct
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('entry_boundaries', ROOT/'tools/inspect-pixel-route-entry-boundaries.py')
pe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pe)


def fixture():
    data = bytearray(0x1200)
    data[:2] = b'MZ'
    struct.pack_into('<I',data,60,0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<HH',data,0x84,0x14c,1)
    struct.pack_into('<HH',data,0x94,224,0x102)
    optional = 0x98
    struct.pack_into('<H',data,optional,0x10b)
    struct.pack_into('<I',data,optional+16,0x1024)
    struct.pack_into('<I',data,optional+28,0x500000)
    struct.pack_into('<I',data,optional+60,0x200)
    struct.pack_into('<I',data,optional+92,16)
    section = optional+224
    data[section:section+8] = b'.text\0\0\0'
    struct.pack_into('<4I',data,section+8,0x1000,0x1000,0x1000,0x200)
    struct.pack_into('<I',data,section+36,0x60000020)
    return data


def raw(rva):
    return rva-0x1000+0x200


def directory(data,index,rva,size):
    struct.pack_into('<II',data,0x98+96+index*8,rva,size)


def word(data,rva,value):
    struct.pack_into('<I',data,raw(rva),value)


class Boundaries(unittest.TestCase):
    def test_camera_search_contains_complete_registered_body(self):
        self.assertEqual(pe.WINDOWS['minimap/camera3'], (0x5532f0, 0x5536bc))

    def test_exports_include_ordinal_only_and_forwarders(self):
        d=fixture();directory(d,0,0x1100,0x100)
        struct.pack_into('<6I',d,raw(0x1110),7,3,1,0x1160,0x1170,0x1180)
        for i,value in enumerate((0x1024,0x1028,0x1190)):word(d,0x1160+i*4,value)
        word(d,0x1170,0x11a0);struct.pack_into('<H',d,raw(0x1180),0)
        d[raw(0x1190):raw(0x1190)+9]=b'OTHER.#7\0'
        d[raw(0x11a0):raw(0x11a0)+6]=b'entry\0'
        rows=pe.PE32(d).exports()
        self.assertEqual([(r['ordinal'],r['target']) for r in rows],[(7,0x501024),(8,0x501028),(9,None)])
        self.assertEqual(rows[0]['names'],['entry']);self.assertEqual(rows[2]['forwarder'],'OTHER.#7')

    def test_bad_export_ordinal_rejects(self):
        d=fixture();directory(d,0,0x1100,0x100)
        struct.pack_into('<6I',d,raw(0x1110),0,1,1,0x1160,0x1170,0x1180)
        struct.pack_into('<H',d,raw(0x1180),1)
        with self.assertRaisesRegex(ValueError,'ordinal'):pe.PE32(d).exports()

    def test_tls_uses_va_and_stops_at_zero(self):
        d=fixture();directory(d,9,0x1200,24);word(d,0x120c,0x501240)
        word(d,0x1240,0x501024);word(d,0x1244,0x501028)
        self.assertEqual(pe.PE32(d).tls_callbacks(),[0x501024,0x501028])

    def test_tls_truncation_and_bad_array_reject(self):
        for size,array in ((20,0x501240),(24,0x600000)):
            with self.subTest(size=size):
                d=fixture();directory(d,9,0x1200,size);word(d,0x120c,array)
                with self.assertRaises(ValueError):pe.PE32(d).tls_callbacks()

    def test_import_name_and_ordinal(self):
        d=fixture();directory(d,1,0x1300,40)
        struct.pack_into('<5I',d,raw(0x1300),0x1380,0,0,0x1360,0x13a0)
        d[raw(0x1360):raw(0x1360)+10]=b'Chaos.exe\0'
        word(d,0x1380,0x13c0);word(d,0x1384,0x80000007)
        d[raw(0x13c2):raw(0x13c2)+6]=b'entry\0'
        self.assertEqual(pe.PE32(d).imports(),[dict(module='Chaos.exe',symbols=[{'name':'entry'},{'ordinal':7}])])

    def test_import_requires_descriptor_terminator(self):
        d=fixture();directory(d,1,0x1300,20)
        struct.pack_into('<5I',d,raw(0x1300),0x1380,0,0,0x1360,0x13a0)
        d[raw(0x1360):raw(0x1360)+2]=b'X\0'
        with self.assertRaisesRegex(ValueError,'directory'):pe.PE32(d).imports()

    def test_relocation_extent_and_target(self):
        d=fixture();directory(d,5,0x1400,12)
        struct.pack_into('<IIHH',d,raw(0x1400),0x1000,12,0x3050,0)
        word(d,0x1050,0x501024)
        r=pe.PE32(d).relocations()
        self.assertEqual((r['blocks'],r['highlow']),(1,1));self.assertEqual(r['targets'][0]['target'],0x501024)
        word(d,0x1404,7)
        with self.assertRaisesRegex(ValueError,'relocation'):pe.PE32(d).relocations()

    def test_raw_mapping_rejects_virtual_tail_and_wrap(self):
        image=pe.PE32(fixture())
        self.assertEqual(image.offset(0x1ffc,4),0x11fc)
        for rva,size in ((0x1ffd,4),(0xffffffff,4),(-1,1)):
            with self.subTest(rva=rva),self.assertRaises(ValueError):image.offset(rva,size)

    def test_header_truncation_and_directories_reject(self):
        d=fixture()
        with self.assertRaises(ValueError):pe.PE32(d[:0x190])
        struct.pack_into('<I',d,0x98+92,17)
        with self.assertRaisesRegex(ValueError,'directories'):pe.PE32(d)

    def test_candidates_keep_representations_and_negative_rel32(self):
        d=fixture();word(d,0x1080,0x501024);word(d,0x1084,0x1028);word(d,0x1088,0x22c)
        d[raw(0x1040)]=0xe8;struct.pack_into('<i',d,raw(0x1041),0x102c-0x1045)
        r=pe.inspect_module(d,pe.PE32(d),{'test/route':(0x501020,0x501030)})
        self.assertEqual({v['representation'] for v in r['stored_byte_candidates']},{'preferred_va','target_rva','target_file_offset'})
        self.assertTrue(any(v['preferred_site']==0x501040 and v['target']==0x50102c for v in r['rel32_byte_candidates']))
        self.assertEqual(r['boundary_candidates'],[dict(route='test/route',kind='entry',preferred_target=0x501024)])
        self.assertIs(r['global_unreachable_proved'],False)

    def test_empty_candidates_do_not_prove_global_unreachability(self):
        d=fixture();word(d,0x1010,0)
        r=pe.inspect_module(d,pe.PE32(d),{'test/route':(0x501800,0x501810)})
        self.assertEqual(r['stored_byte_candidates'],[]);self.assertEqual(r['boundary_candidates'],[])
        self.assertIs(r['global_unreachable_proved'],False)

    def test_overlapping_rva_and_file_offset_interpretations_are_retained(self):
        d=fixture();word(d,0x1080,0x1024)
        r=pe.inspect_module(d,pe.PE32(d),{'first':(0x501020,0x501030),'second':(0x501e20,0x501e30)})
        interpretations={(v['route'],v['representation']) for v in r['stored_byte_candidates'] if v['rva']==0x1080}
        self.assertEqual(interpretations,{('first','target_rva'),('second','target_file_offset')})


if __name__=='__main__':unittest.main()
