#!/usr/bin/env python3
"""Production custody encoding and negative input admission; no original run."""
import os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from palette_observation import decode,decode_profiles

class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  retained=os.environ.get('MNM_PALETTE_CONTROL_OUTPUT');cls.tmp=None if retained else tempfile.TemporaryDirectory(prefix='palette-observation-',dir='/tmp');cls.out=Path(retained)if retained else Path(cls.tmp.name)
  if retained:cls.out.mkdir(parents=True,exist_ok=False)
  subprocess.run(['cc','-m32','-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',ROOT/'tests/palette-observation-fixture.c','-o',cls.out/'fixture'],check=True)
  for kind in ('normal','disabled','bad-signature','bad-import','foreign','unknown','retired','changed-table','shade-only','reuse','profile','refresh','refresh-failed','refresh-unknown','full-prefix','outside-world','positive-regression'):
   subprocess.run([cls.out/'fixture',cls.out/(kind+'.bin'),kind],check=True)
  subprocess.run(['cc','-m32','-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-no-pie',ROOT/'tests/palette-observation-abi-fixture.c',ROOT/'tests/palette-observation-abi.S',ROOT/'runtime/scene/palette_observation.S','-o',cls.out/'abi-fixture'],check=True)
  cls.abi=subprocess.run([cls.out/'abi-fixture',cls.out/'abi.bin'],check=True,capture_output=True)
  cls.raw=(cls.out/'normal.bin').read_bytes();cls.result=decode(cls.raw)
 @classmethod
 def tearDownClass(cls):
  if cls.tmp:cls.tmp.cleanup()
 def test_production_encoder(self):
  q=self.result;self.assertTrue(q['complete']);self.assertEqual(q['owner'],67);self.assertEqual(len(q['nodes']),2)
  self.assertFalse(q['cycling_rows_passed']);self.assertEqual(q['terminal'][11:13],[0,0])
 def test_real_selection_changes_and_duplicate_suppression(self):
  rows=[r['header']for r in self.result['rows']if r['header'][2]==5]
  self.assertEqual([r[8]for r in rows],[1,2,1]);self.assertEqual([r[5]for r in rows],[1,2,3]);self.assertEqual(self.result['terminal'][10],6)
 def test_lifetime(self):self.assertTrue(all(not n['live']and n['retirement']>n['birth']for n in self.result['nodes'].values()))
 def test_production_hook_assembly(self):
  q=decode((self.out/'abi.bin').read_bytes());self.assertEqual(len([r for r in q['rows']if r['header'][2]==2]),24);self.assertIn(b'28 production hook',self.abi.stdout)
  self.assertEqual({r['header'][16]for r in q['rows']if r['header'][2]==1},{0x37f,0x27f})
 def test_captured_floating_context(self):
  self.assertEqual(self.result['version'],3);self.assertTrue(all(n['floating_input'][:2]==[0x37f,0x1f80]for n in self.result['nodes'].values()))
 def test_historical_v1_remains_readable_without_inventing_context(self):
  raw=self.historical(1);at=64
  while at<len(raw):
   size,ordinal,kind=struct.unpack_from('<3I',raw,at)
   if kind in(1,2,3,4):struct.pack_into('<4I',raw,at+64,0,0,0,0)
   at+=size
  q=decode(raw);self.assertEqual(q['version'],1);self.assertTrue(all(n['floating_input']is None for n in q['nodes'].values()))
 def historical(self,version):
  from scene_actor_observation import crc32c
  raw=bytearray(self.raw[:64]);raw[:8]=b'MNMPAL01'if version==1 else b'MNMPAL02';struct.pack_into('<I',raw,8,version)
  for row in self.result['rows']:
   h=list(row['header']);body=row['payload']
   if h[2]==5:
    links=struct.unpack('<'+'I'*(2*h[27]),body);body=struct.pack('<'+'I'*h[27],*links[::2]);h[0]=128+len(body);h[30]=crc32c(body)
   if h[2]==6:h[9]=len(raw)
   raw.extend(struct.pack('<32I',*h));raw.extend(body)
  return raw
 def test_historical_v2_does_not_invent_refreshes(self):
  q=decode(self.historical(2));self.assertEqual(q['version'],2);self.assertTrue(all(n['revision']==0 and not n['refreshes']for n in q['nodes'].values()))
 def test_static_spatial_shade_alternation_does_not_expand_the_log(self):
  q=decode((self.out/'shade-only.bin').read_bytes());self.assertEqual(len([r for r in q['rows']if r['header'][2]==5]),4);self.assertEqual(q['terminal'][10],38)
 def test_reused_address_has_new_allocation_and_selection(self):
  q=decode((self.out/'reuse.bin').read_bytes());self.assertEqual(len(q['nodes']),3);self.assertTrue(all(not n['live']for n in q['nodes'].values()));self.assertEqual([r['header'][8]for r in q['rows']if r['header'][2]==5],[1,2,1,3])
 def test_disabled_and_signature_guard(self):
  for name in ('disabled','bad-signature','bad-import'):self.assertFalse((self.out/(name+'.bin')).exists())
 def test_actual_profile_arguments_and_return_transcript(self):
  q=decode((self.out/'profile.bin').read_bytes());p=decode_profiles((self.out/'profile.bin.profiles').read_bytes(),q);self.assertTrue(p['complete']);self.assertEqual(len(p['rows']),3);self.assertEqual([r['texts'][1]for r in p['rows']],[b'Reducer',b'SpecalValue',b'Darkest']);self.assertTrue(all(r['texts'][4]==b'.25'and r['node']==3 for r in p['rows']))
 def test_profile_stdcall_actual_hook_assembly(self):
  q=decode((self.out/'abi.bin').read_bytes());p=decode_profiles((self.out/'abi.bin.profiles').read_bytes(),q);self.assertEqual(len(p['rows']),3)
 def test_profile_refuses_unknown_parent(self):
  q=decode((self.out/'profile.bin').read_bytes());raw=bytearray((self.out/'profile.bin.profiles').read_bytes());struct.pack_into('<I',raw,64+8,999999)
  with self.assertRaises(ValueError):decode_profiles(raw,q)
 def test_profile_refuses_changed_input_text(self):
  q=decode((self.out/'profile.bin').read_bytes());raw=bytearray((self.out/'profile.bin.profiles').read_bytes());raw[128]^=1
  with self.assertRaises(ValueError):decode_profiles(raw,q)
 def test_complete_first_four_world_bindings_are_explicit(self):
  q=decode((self.out/'full-prefix.bin').read_bytes());self.assertEqual(q['full_world_prefix'],4);self.assertEqual(len([r for r in q['rows']if r['header'][2]==5]),6);self.assertEqual(q['terminal'][10],6)
 def test_refresh_versions_and_return_to_initial_content(self):
  q=decode((self.out/'refresh.bin').read_bytes());n=q['nodes'][2];self.assertEqual(n['revision'],2);self.assertEqual(len(n['refreshes']),2)
  self.assertNotEqual(n['refreshes'][0]['before'],n['refreshes'][0]['after']);self.assertEqual(n['initial_data'],n['data'])
  self.assertEqual([r['header'][7]for r in q['rows']if r['header'][2]==5],[0,0,0,1,2])
 def test_failed_refresh_preserves_revision_and_bytes(self):
  q=decode((self.out/'refresh-failed.bin').read_bytes());n=q['nodes'][2];self.assertEqual(n['revision'],0);self.assertEqual(n['data'],n['initial_data']);self.assertTrue(all(not r['result']for r in n['refreshes']))
 def test_refresh_unknown_target_is_refused(self):
  with self.assertRaises(ValueError):decode((self.out/'refresh-unknown.bin').read_bytes())
 def test_refresh_production_ret28_assembly(self):
  q=decode((self.out/'abi.bin').read_bytes());r=[v for n in q['nodes'].values()for v in n['refreshes']];self.assertEqual(len(r),4);self.assertEqual({v['floating_input'][0]for v in r},{0x37f,0x27f})
 def test_refresh_preimage_mismatch_is_refused(self):
  from scene_actor_observation import crc32c
  raw=bytearray((self.out/'refresh.bin').read_bytes());at=64
  while struct.unpack_from('<I',raw,at+8)[0]!=7:at+=struct.unpack_from('<I',raw,at)[0]
  size=struct.unpack_from('<I',raw,at)[0];raw[at+128+872+12]^=1;struct.pack_into('<I',raw,at+120,crc32c(raw[at+128:at+size]))
  with self.assertRaisesRegex(ValueError,'preimage'):decode(raw)
 def test_zero_queue_retirement_is_outside_world(self):
  q=decode((self.out/'outside-world.bin').read_bytes());self.assertTrue(q['complete']);self.assertTrue(all(r['header'][5]==0 for r in q['rows']if r['header'][2]in(3,4)))
 def test_positive_queue_regression_stays_refused(self):
  with self.assertRaisesRegex(ValueError,'queue regression'):decode((self.out/'positive-regression.bin').read_bytes())
 def test_foreign_owner(self):
  with self.assertRaises(ValueError):decode((self.out/'foreign.bin').read_bytes())
 def test_unknown_allocation(self):
  with self.assertRaises(ValueError):decode((self.out/'unknown.bin').read_bytes())
 def test_retired_allocation(self):
  with self.assertRaises(ValueError):decode((self.out/'retired.bin').read_bytes())
 def test_unobserved_mutation_is_refused(self):
  with self.assertRaisesRegex(ValueError,'recorded production'):decode((self.out/'changed-table.bin').read_bytes())
 def test_missing_terminal(self):
  with self.assertRaises(ValueError):decode(self.raw[:-128])
 def test_corrupt_owned_rgb(self):
  raw=bytearray(self.raw);raw[64+128]^=1
  with self.assertRaisesRegex(ValueError,'CRC'):decode(raw)
 def test_header_guard(self):
  raw=bytearray(self.raw);struct.pack_into('<I',raw,16,1)
  with self.assertRaises(ValueError):decode(raw)
 def test_reserved_field(self):
  raw=bytearray(self.raw);struct.pack_into('<I',raw,64+7*4,1)
  with self.assertRaisesRegex(ValueError,'reserved'):decode(raw)
 def test_incomplete_read_is_explicit(self):self.assertFalse(decode(self.raw[:-128],complete=False)['complete'])

if __name__=='__main__':unittest.main()
