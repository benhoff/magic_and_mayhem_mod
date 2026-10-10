#!/usr/bin/env python3
"""Independent producer-envelope bounds: extended queues need explicit V3."""
import importlib.util
from pathlib import Path
import struct
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('producer_inspector',ROOT/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class ExtendedEnvelope(unittest.TestCase):
    def wire(self,version=3,queues=32,records=131072):
        h=[0]*16;h[:2]=struct.unpack('<2I',('MNMPRO0'+str(version)).encode());h[2:7]=[version,64,0x40209ca7,records,queues]
        raw=bytearray(struct.pack('<16I',*h))
        for q in range(1,queues+1):
            for kind in (11,12):
                r=[0]*24;r[:3]=[96,(q-1)*2+(kind==12)+1,kind];r[14]=q;raw.extend(struct.pack('<24I',*r))
        return raw
    def test_closed_prefix_and_legacy_limits(self):
        h,rows=module.decode(self.wire());self.assertEqual(h[6],32);self.assertEqual(len(rows),64)
        module.decode(self.wire(1,16,65536));module.decode(self.wire(2,16,262144))
        for wire in (self.wire(1,32,65536),self.wire(2,32,262144),self.wire(3,16),self.wire(3,33),self.wire(3,32,262144),self.wire()[:-96]):
            with self.assertRaises(ValueError):module.decode(wire)
    def test_raw_diagnostic_sample_extension_is_explicit(self):
        spec=importlib.util.spec_from_file_location('startup_inspector',ROOT/'tools/inspect-startup-queues.py');startup=importlib.util.module_from_spec(spec);spec.loader.exec_module(startup)
        h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSTQ01');h[2:7]=[1,64,64,32,32];h[13]=36
        raw=struct.pack('<16I',*h)
        with self.assertRaises(ValueError):startup.decode(raw)
        self.assertEqual(startup.decode(raw,sample_limit=32)['sample'],32)
        h[6]=33
        with self.assertRaises(ValueError):startup.decode(struct.pack('<16I',*h),sample_limit=32)

    def test_owned_source_references_preserve_full_packet(self):
        source=struct.pack('<10I',40,*([0]*9));definition=[0]*24
        definition[:3]=[136,1,25];definition[14]=9;definition[18]=definition[19]=40
        reference=[0]*24;reference[:3]=[96,2,9];reference[4]=1;reference[15]=1;reference[19]=40
        def encoded(d=definition,r=reference):return self.wire()[:64]+struct.pack('<24I',*d)+source+struct.pack('<24I',*r)
        h,rows=module.decode(encoded(),False)
        self.assertIs(rows[0][1],rows[1][1]);self.assertEqual(rows[1][0][0],136)
        self.assertEqual(rows[1][0][4],0);self.assertEqual(rows[1][0][18],40)
        for word,value in [(4,99),(17,1),(19,41),(18,1)]:
            changed=list(reference);changed[word]=value
            with self.subTest(word=word),self.assertRaises(ValueError):module.decode(encoded(r=changed),False)
        changed=list(definition);changed[14]=8
        with self.assertRaises(ValueError):module.decode(encoded(d=changed),False)

    def test_unknown_operation_and_version_do_not_expand(self):
        wire=self.wire();struct.pack_into('<I',wire,64+8,24)
        with self.assertRaises(ValueError):module.decode(wire)
        wire=self.wire();struct.pack_into('<I',wire,8,1)
        with self.assertRaises(ValueError):module.decode(wire)

if __name__=='__main__':unittest.main()
