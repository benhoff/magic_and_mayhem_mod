#!/usr/bin/env python3
"""Production font custody, actual i386 hook ABI and strict reader refusals."""
import os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from font_lifetime_observation import decode
class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  retained=os.environ.get('MNM_FONT_CONTROL_OUTPUT');cls.temp=None if retained else tempfile.TemporaryDirectory(prefix='font-custody-',dir='/tmp');cls.out=Path(retained)if retained else Path(cls.temp.name);cls.out.mkdir(parents=True,exist_ok=True)
  subprocess.run(['cc','-m32','-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable','-Wno-misleading-indentation','-no-pie',ROOT/'tests/font-lifetime-fixture.c',ROOT/'tests/font-lifetime-abi.S',ROOT/'runtime/scene/font_observation.S','-o',cls.out/'fixture'],check=True)
  cls.result=subprocess.run([cls.out/'fixture',cls.out/'positive.bin','positive'],check=True,capture_output=True)
  for name in ['disabled','bad-signature','not-resident','conflicting-observers']:subprocess.run([cls.out/'fixture',cls.out/(name+'.bin'),name],check=True)
  cls.raw=(cls.out/'positive.bin').read_bytes();cls.decoded=decode(cls.raw)
 @classmethod
 def tearDownClass(cls):
  if cls.temp:cls.temp.cleanup()
 def test_production_hook_abi(self):self.assertIn(b'24 production font hook ABI cases passed',self.result.stdout)
 def test_installed_generation_shape(self):self.assertEqual(len(self.decoded['fonts']),16);self.assertEqual(len(self.decoded['glyphs']),16);self.assertEqual({g['role']for g in self.decoded['glyphs']},{1,2,3,4})
 def test_original_reload_retention_is_explicit(self):self.assertEqual(self.decoded['original_reload_retained_generations'],8);self.assertEqual(sum(f['retired']for f in self.decoded['fonts'].values()),8);self.assertFalse(self.decoded['closes_acceptance'])
 def test_guard_refusals_and_disabled(self):
  for name in ['disabled','bad-signature','not-resident','conflicting-observers']:self.assertFalse((self.out/(name+'.bin')).exists())
 def test_conflicting_observers_do_not_patch_or_open(self):self.assertFalse((self.out/'conflicting-observers.bin').exists())
 def replace(self,index,field,value):
  raw=bytearray(self.raw);at=64
  for _ in range(index):at+=struct.unpack_from('<I',raw,at)[0]
  struct.pack_into('<I',raw,at+field*4,value);return raw
 def index(self,kind):return next(i for i,r in enumerate(self.decoded['rows'])if r['header'][2]==kind)
 def test_unknown_version(self):
  raw=bytearray(self.raw);struct.pack_into('<I',raw,8,99)
  with self.assertRaises(ValueError):decode(raw)
 def test_payload_corruption(self):
  raw=bytearray(self.raw);raw[200]^=1
  with self.assertRaisesRegex(ValueError,'CRC'):decode(raw)
 def test_missing_teardown(self):
  with self.assertRaises(ValueError):decode(self.raw[:-128])
 def test_terminal_live_owner(self):
  with self.assertRaisesRegex(ValueError,'unretired'):decode(self.replace(len(self.decoded['rows'])-1,10,1))
 def test_missing_font_generation(self):
  with self.assertRaises(ValueError):decode(self.replace(self.index(3),7,999))
 def test_wrong_glyph_ordinal(self):
  with self.assertRaises(ValueError):decode(self.replace(self.index(3),9,999))
 def test_changed_font_source(self):
  with self.assertRaisesRegex(ValueError,'source differs'):decode(self.replace(self.index(3),13,123))
 def test_wrong_current_generation(self):
  with self.assertRaisesRegex(ValueError,'owner differs'):decode(self.replace(self.index(3),21,999))
 def test_foreign_owner(self):
  with self.assertRaisesRegex(ValueError,'foreign owner'):decode(self.replace(self.index(3),3,999))
 def test_lost_observations(self):
  with self.assertRaisesRegex(ValueError,'loss'):decode(self.replace(len(self.decoded['rows'])-1,7,1))
 def test_unclosed_callback(self):
  with self.assertRaisesRegex(ValueError,'active call'):decode(self.replace(len(self.decoded['rows'])-1,11,1))
 def test_source_context_identity(self):
  self.assertEqual(self.decoded['version'],2);self.assertTrue(all(g['world_generation']==7 and g['source_ordinal']==3 for g in self.decoded['glyphs']))
  for field in (24,26):
   with self.subTest(field=field),self.assertRaisesRegex(ValueError,'context missing'):decode(self.replace(self.index(3),field,0))
 def test_reserved_fields(self):
  with self.assertRaisesRegex(ValueError,'reserved'):decode(self.replace(self.index(3),31,1))
if __name__=='__main__':unittest.main()
