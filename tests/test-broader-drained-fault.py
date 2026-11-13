#!/usr/bin/env python3
"""Bounded admission against the real frozen native helper, without a game."""
import hashlib,importlib.util,json,mmap,os,struct,subprocess,sys,tempfile,threading,time,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('drained_fault',ROOT/'tools/run-drained-transition-fault.py');adapter=importlib.util.module_from_spec(spec);spec.loader.exec_module(adapter)
HELPER=ROOT/adapter.HELPER
class Admission(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  global HELPER
  cls.helper_temp=None
  if HELPER.is_file():
   if hashlib.sha256(HELPER.read_bytes()).hexdigest()!=adapter.HELPER_SHA256:raise ValueError('Existing current helper differs from the frozen diagnostic')
  else:
   import subprocess
   cls.helper_temp=tempfile.TemporaryDirectory(prefix='mnm-drain-helper-');HELPER=Path(cls.helper_temp.name)/'helper';subprocess.run(['g++','-std=c++17','-O2',str(ROOT/'tools/rendering-transition-fault.cpp'),'-o',str(HELPER)],check=True,capture_output=True)
 @classmethod
 def tearDownClass(cls):
  if cls.helper_temp:cls.helper_temp.cleanup()
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory(prefix='mnm-drain-control-');self.root=Path(self.temp.name);self.channel=self.root/'working/experiments/menu-observer/run-control/render-commands.bin';self.channel.parent.mkdir(parents=True);self.raw=bytearray(1048640);self.raw[:8]=b'MNMRDC03';struct.pack_into('<8I',self.raw,8,3,len(self.raw),37,124,1,0,0,123);self.channel.write_bytes(self.raw);self.command=[str(HELPER),str(self.root),str(self.channel),'37'];self.sha=hashlib.sha256(HELPER.read_bytes()).hexdigest()
 def tearDown(self):self.temp.cleanup()
 def test_wait_then_exact_state_reason_mutation(self):
  def drain():
   time.sleep(.06)
   with self.channel.open('r+b')as f:
    with mmap.mmap(f.fileno(),0)as m:struct.pack_into('<I',m,36,124)
  thread=threading.Thread(target=drain);thread.start()
  try:value=adapter.admit(self.command,self.sha)
  finally:thread.join()
  self.assertGreater(value['drain_admission']['attempts'],1);self.assertTrue(value['drain_admission']['prior_refusals']);expected=bytearray(self.raw);struct.pack_into('<I',expected,36,124);struct.pack_into('<2I',expected,24,3,5);self.assertEqual(self.channel.read_bytes(),expected);self.assertFalse(value['writer_claimed']);self.assertFalse(value['cursor_or_payload_written'])
 def test_permanent_pending_read_refuses_without_mutation(self):
  with self.assertRaises((ValueError,subprocess.TimeoutExpired)):adapter.admit(self.command,self.sha,timeout=.1)
  self.assertEqual(self.channel.read_bytes(),self.raw)
 def test_unknown_identity_refusal_is_immediate(self):
  self.command[-1]='38'
  with self.assertRaisesRegex(ValueError,'without retry'):adapter.admit(self.command,self.sha)
  self.assertEqual(self.channel.read_bytes(),self.raw)
 def test_wrong_helper_hash_refuses_before_invocation(self):
  with self.assertRaisesRegex(ValueError,'helper changed'):adapter.admit(self.command,'0'*64)
  self.assertEqual(self.channel.read_bytes(),self.raw)
 def test_finite_attempt_limit(self):
  with self.assertRaisesRegex(ValueError,'bound: 1'):adapter.admit(self.command,self.sha,maximum_attempts=1)
  self.assertEqual(self.channel.read_bytes(),self.raw)
 def test_already_drained_first_attempt(self):
  struct.pack_into('<I',self.raw,36,124);self.channel.write_bytes(self.raw);value=adapter.admit(self.command,self.sha);self.assertEqual(value['drain_admission']['attempts'],1);self.assertEqual(value['drain_admission']['prior_refusals'],[])
 def test_duplicate_fault_not_retried(self):
  struct.pack_into('<3I',self.raw,24,3,5,0);self.channel.write_bytes(self.raw)
  with self.assertRaisesRegex(ValueError,'without retry'):adapter.admit(self.command,self.sha)
  self.assertEqual(self.channel.read_bytes(),self.raw)
if __name__=='__main__':unittest.main()
