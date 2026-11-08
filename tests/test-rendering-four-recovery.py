"""Reject incomplete domains and false normal-completion credit at admission."""
import copy,importlib.util,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('four_recovery',ROOT/'tools/close-rendering-four-recovery.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Admission(unittest.TestCase):
 def cases(self):
  rows=[]
  for i,(software,size) in enumerate((v,s) for v in (False,True) for s in ([640,480],[800,600])):
   proof=dict(verified=True,cleanup=dict(verified=True),physical_public_restart=True,fresh_positive_input=True,old_rendering_accepted=False,failed_epoch_normal_completion_claimed=False,terminal_samples=8,terminal_stability_ns=600_000_000,distinct_world_nonces=['old'+str(i),'fresh'+str(i)],distinct_runner_identities=[dict(pid=i*2+1,start_ticks=1),dict(pid=i*2+2,start_ticks=2)])
   rows.append(dict(identity=str(i),domain=dict(software=software,size=size),proof=proof,pixels=1,acquisition=dict(record='a'+str(i)),comparison=dict(record='c'+str(i)),provider='bounded-test'))
  return rows
 def parent(self):
  plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes());rows=[dict(v,state='pending',admitted_execution_evidence=[]) for v in m.requirements(plan)]
  return plan,dict(success=True,dossier_identity=m.fingerprint(m.requirements(plan)),requirements=rows,passed_requirements=0)
 def test_only_the_four_requested_rows_change(self):
  plan,parent=self.parent();out=m.close_rows(plan,parent,self.cases())
  self.assertEqual(sum(v['state']=='passed' for v in out),4)
  self.assertEqual([v for v in out if v['id'] not in m.LEAVES],[v for v in parent['requirements'] if v['id'] not in m.LEAVES])
 def test_missing_duplicate_or_repeated_identity_is_not_a_matrix(self):
  rows=self.cases()
  for bad in [rows[:3],rows[:3]+[copy.deepcopy(rows[0])]]:
   with self.assertRaises(ValueError):m.validate_cases(bad)
  bad=self.cases();bad[1]['identity']=bad[0]['identity']
  with self.assertRaises(ValueError):m.validate_cases(bad)
 def test_failed_cleanup_or_false_normal_credit_refuses(self):
  for key,value in [('old_rendering_accepted',True),('failed_epoch_normal_completion_claimed',True),('fresh_positive_input',False),('terminal_stability_ns',599_999_999),('terminal_samples',7)]:
   rows=self.cases();rows[0]['proof'][key]=value
   with self.subTest(key=key),self.assertRaises(ValueError):m.validate_cases(rows)
  rows=self.cases();rows[0]['proof']['cleanup']['verified']=False
  with self.assertRaises(ValueError):m.validate_cases(rows)
 def test_generation_reuse_and_premature_readmission_refuse(self):
  rows=self.cases();rows[0]['proof']['distinct_world_nonces']=['same','same']
  with self.assertRaises(ValueError):m.validate_cases(rows)
  plan,parent=self.parent();next(v for v in parent['requirements'] if v['id'] in m.LEAVES)['state']='passed'
  with self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases())
 def test_parent_configuration_substitution_refuses(self):
  plan,parent=self.parent();parent['requirements'][0]['configuration']='unregistered-profile'
  with self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases())
if __name__=='__main__':unittest.main()
