#!/usr/bin/env python3
"""The history flag affects only the declared exact player and no game inputs."""
import copy,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_command_history_launch import history_environment
class Selection(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);self.binary=self.root/'player';self.binary.touch();self.env=dict(MNM_ACCEPTANCE_NORMAL='1',MNM_ACCEPTANCE_CAMPAIGN='1',MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(self.root/'capture'),MNM_ACCEPTANCE_CAPTURE_ID='exact-case',DISPLAY='private',UNCHANGED='original')
 def test_exact_player_changes_only_one_flag(self):
  before=copy.deepcopy(self.env);out,selected=history_environment(self.root,self.binary,[str(self.binary),'--original'],self.env);self.assertTrue(selected);self.assertEqual(out,self.env|{'MNM_ACCEPTANCE_COMMAND_HISTORY':'1'});self.assertEqual(self.env,before)
 def test_public_shell_receives_same_policy(self):
  out,selected=history_environment(self.root,self.binary,[str(self.root/'run-native.sh'),'--original'],self.env);self.assertTrue(selected);self.assertEqual(out['MNM_ACCEPTANCE_COMMAND_HISTORY'],'1')
 def test_other_programs_and_relative_names_are_unchanged(self):
  for command in [[str(self.root/'other')],['player'],[]]:
   out,selected=history_environment(self.root,self.binary,command,self.env);self.assertFalse(selected);self.assertEqual(out,self.env)
 def test_exact_alias_of_player_is_supported(self):
  alias=self.root/'alias';alias.symlink_to(self.binary);self.assertTrue(history_environment(self.root,self.binary,[str(alias)],self.env)[1])
 def test_missing_lossless_preconditions_refuse(self):
  for key in('MNM_ACCEPTANCE_NORMAL','MNM_ACCEPTANCE_CAMPAIGN','MNM_ACCEPTANCE_CAPTURE_DIRECTORY','MNM_ACCEPTANCE_CAPTURE_ID'):
   value=self.env.pop(key)
   with self.assertRaises(ValueError):history_environment(self.root,self.binary,[str(self.binary)],self.env)
   self.env[key]=value
 def test_conflicting_policy_is_not_silently_overridden(self):
  self.env['MNM_ACCEPTANCE_COMMAND_HISTORY']='0'
  with self.assertRaises(ValueError):history_environment(self.root,self.binary,[str(self.binary)],self.env)
 def test_existing_history_flag_is_idempotent(self):
  self.env['MNM_ACCEPTANCE_COMMAND_HISTORY']='1';out,selected=history_environment(self.root,self.binary,[str(self.binary)],self.env);self.assertTrue(selected);self.assertEqual(out,self.env)
class BindingSelection(unittest.TestCase):
 setUp=Selection.setUp
 def test_real_nested_binding_passes_one_flag_once_and_restores(self):
  import hashlib,subprocess
  from unittest.mock import patch
  from broader_command_history_launch import WorldBinding
  calls=[]
  def original(command,*args,**kwargs):calls.append((command,kwargs));return object()
  plan=dict(player_binary=dict(path='player',sha256=hashlib.sha256(self.binary.read_bytes()).hexdigest()),counter_bindings={})
  with patch.object(subprocess,'Popen',original):
   binding=WorldBinding(self.root,plan,'case.json','llvmpipe','phase.json');binding.install()
   try:
    subprocess.Popen([str(self.binary),'unchanged'],env=self.env)
    self.assertEqual(len(calls),1);self.assertEqual(calls[0][0],[str(self.binary),'unchanged']);self.assertEqual(calls[0][1]['env'],self.env|{'MNM_ACCEPTANCE_COMMAND_HISTORY':'1'})
    self.assertEqual(len(binding.receipt()['command_history_selection']['launches']),1)
   finally:binding.restore()
   self.assertIs(subprocess.Popen,original)
 def test_changed_binary_refuses_before_launch(self):
  import subprocess
  from unittest.mock import patch
  from broader_command_history_launch import WorldBinding
  plan=dict(player_binary=dict(path='player',sha256='wrong'),counter_bindings={})
  with patch.object(subprocess,'Popen') as original:
   binding=WorldBinding(self.root,plan,'case.json','llvmpipe','phase.json');binding.install()
   try:
    with self.assertRaises(ValueError):subprocess.Popen([str(self.binary)],env=self.env)
    original.assert_not_called()
   finally:binding.restore()
if __name__=='__main__':unittest.main()
