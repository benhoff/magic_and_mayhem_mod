#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_cursor_journal import decode
from native_cursor_launch import stage,authorize

class CursorJournal(unittest.TestCase):
    def data(self):
        header=[0x434d4e4d,0x31574f52,1,64,0x40209ca7,18,32768]+[0]*9
        install=[1,1,7]+[0]*13
        terminal=[2,10,7,0,0,1,3,1,2,0,0,0,0,0,4,1]
        return struct.pack('<48I',*header,*install,*terminal)
    def test_complete(self):
        result=decode(self.data());self.assertEqual(result['queue_accepted'],3);self.assertEqual(result['pending_jobs'],0)
    def test_refusals(self):
        data=self.data();bad=[b'',data[:-1],data[:64],b'badmagic'+data[8:]]
        for at,value in [(8,2),(28,1),(64,7),(68,11),(128+4,9),(128+12,1),(128+16,1),(128+20,0),(128+24,5)]:
            b=bytearray(data);struct.pack_into('<I',b,at,value);bad.append(bytes(b))
        for b in bad:
            with self.subTest(size=len(b)),self.assertRaises(ValueError):decode(b)
    def test_launch(self):
        root=Path('/owned/new-session');metadata=dict(native_draw=True,**stage(root,True));env={'MNM_NATIVE_CURSOR_OWNERSHIP':'1','MNM_NATIVE_CURSOR_LOG':'foreign'}
        authorize(root,metadata,env);self.assertEqual(env['MNM_NATIVE_CURSOR_OWNERSHIP'],'2');self.assertTrue(env['MNM_NATIVE_CURSOR_LOG'].endswith('native-cursor-lifetime.bin'))
        for bad in [dict(native_draw=True),dict(metadata,native_cursor_ownership=1),dict(metadata,native_cursor_journal='/elsewhere')]:
            with self.assertRaises(ValueError):authorize(root,bad,{})
        authorize(root,dict(native_draw=False,**stage(root,False)),env);self.assertNotIn('MNM_NATIVE_CURSOR_OWNERSHIP',env)
if __name__=='__main__':unittest.main()
