#!/usr/bin/env python3
"""Offline guards for the logging-only PE staging and capture comparison."""
import importlib.util
from pathlib import Path
import struct
import unittest
REPO=Path(__file__).resolve().parent.parent

def module(name,file):
    spec=importlib.util.spec_from_file_location(name,REPO/file)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
stage=module('shadow_stage','tools/prepare-shadow-experiment.py')
compare=module('shadow_compare','tools/compare-neighbor-shadow.py').compare

class ShadowTests(unittest.TestCase):
    def test_import_keeps_code_and_old_imports(self):
        # Synthetic PE with one import. No original game artifacts consumed.
        image=bytearray(0x800);image[:2]=b'MZ';struct.pack_into('<I',image,60,0x80)
        image[0x80:0x84]=b'PE\0\0';struct.pack_into('<HH',image,0x84,0x14c,1)
        struct.pack_into('<H',image,0x94,224);opt=0x98;table=opt+224
        struct.pack_into('<H',image,opt,0x10b)
        struct.pack_into('<II',image,opt+32,0x1000,0x200)
        struct.pack_into('<I',image,opt+60,0x400)
        struct.pack_into('<II',image,opt+104,0x1100,40)
        struct.pack_into('<8sIIIIIIHHI',image,table,b'.text\0\0\0',0x400,0x1000,0x400,0x400,0,0,0,0,0x60000020)
        image[0x400:0x500]=bytes(range(256));struct.pack_into('<5I',image,0x500,0x1180,0,0,0x1160,0x1190)
        render=stage.add_import(bytes(image),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl')
        self.assertIn(b'MnmRender.dll\0',render);self.assertIn(b'RenderAnchor\0',render)
        self.assertIn(b'.mnmgl\0\0',render)
        self.assertEqual(render[0x400:0x800],image[0x400:0x800])
        result=stage.add_import(bytes(image))
        self.assertEqual(result[0x400:0x800],image[0x400:0x800])
        self.assertEqual(struct.unpack_from('<H',result,0x86)[0],2)
        raw=struct.unpack_from('<I',result,table+40+20)[0]
        self.assertEqual(result[raw:raw+20],image[0x500:0x514])
        self.assertIn(b'MnmShadow.dll\0',result[raw:]);self.assertIn(b'ShadowAnchor\0',result[raw:])
        with self.assertRaises(ValueError):stage.add_import(result)
        image[0x84]=0x64
        with self.assertRaises(ValueError):stage.add_import(bytes(image))
    def record(self):
        return b'MNMEXP01'+struct.pack('<5I',0x1000,2,1,1,2)+bytes(98)+bytes([0x55])*36+bytes([0x55])*36+bytes(36)
    def expected(self):return {'budget_remaining':1,'candidate_count':2,'candidate_hex':(bytes([0x55])*36+bytes(36)).hex()}
    def test_full_payload_and_budget(self):
        record=self.record();expected=self.expected();self.assertEqual(compare(expected,record)['status'],'match')
        changed=bytearray(record);changed[-1]=1
        result=compare(expected,changed);self.assertEqual(result['differences'][0]['byte'],35)
        expected['budget_remaining']=0;self.assertEqual(compare(expected,record)['status'],'mismatch')
        changed=bytearray(record);changed[162]=0
        self.assertFalse(compare(self.expected(),changed)['prefix_preserved'])
    def test_invalid_evidence(self):
        for record in (b'',self.record()[:-1],self.record()+b'\0'):
            with self.assertRaises(ValueError):compare(self.expected(),record)
        expected=self.expected();expected['candidate_count']=3
        with self.assertRaises(ValueError):compare(expected,self.record())
if __name__=='__main__':unittest.main()
