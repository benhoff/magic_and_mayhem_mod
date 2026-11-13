#!/usr/bin/env python3
"""Exact attachment setup guard controls; no Native or Original execution."""
import ast,copy,hashlib,sys,tempfile,types,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_attachment_plan_path import adapt,SOURCE_SHA256,PREFIX
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
class Guard(unittest.TestCase):
 def setUp(self):
  self.raw=(ROOT/'tools/run-scene-attachment.py').read_bytes();self.phase=PREFIX+'CONTROL-attachment-phase.json'
  self.code,self.source,self.proof=adapt(self.raw,'run-scene-attachment.py',self.phase)
 def guards(self,plan=None,report='owned/result.json'):
  tree=ast.parse(self.source);main=next(n for n in tree.body if isinstance(n,ast.FunctionDef)and n.name=='main')
  nodes=[n for n in main.body if isinstance(n,ast.Expr)and isinstance(n.value,ast.Call)and isinstance(n.value.func,ast.Name)and n.value.func.id=='require'][:2]
  self.assertEqual(len(nodes),2)
  with tempfile.TemporaryDirectory()as tmp:
   namespace=dict(args=types.SimpleNamespace(plan=self.phase if plan is None else plan,report=report),PREFIX='owned/',ROOT=Path(tmp),Path=Path,require=require)
   exec(compile(ast.fix_missing_locations(ast.Module(body=copy.deepcopy(nodes),type_ignores=[])),'guard-controls','exec'),namespace)
 def test_exact_hash_bound_current_phase_passes(self):self.guards()
 def test_report_prefix_is_still_enforced(self):
  with self.assertRaises(ValueError):self.guards(report='foreign/result.json')
 def test_collector_prefix_plan_is_not_a_substitute_for_current_phase(self):
  with self.assertRaises(ValueError):self.guards(plan='owned/another-plan.json')
 def test_other_current_phase_cannot_pass(self):
  with self.assertRaises(ValueError):self.guards(plan=PREFIX+'other.json')
 def test_current_phase_with_suffix_cannot_pass(self):
  with self.assertRaises(ValueError):self.guards(plan=self.phase+'.other')
 def test_traversal_phase_refuses_adaptation(self):
  with self.assertRaises(ValueError):adapt(self.raw,'run-scene-attachment.py',PREFIX+'../other.json')
 def test_absolute_phase_refuses_adaptation(self):
  with self.assertRaises(ValueError):adapt(self.raw,'run-scene-attachment.py','/'+self.phase)
 def test_foreign_phase_namespace_refuses_adaptation(self):
  with self.assertRaises(ValueError):adapt(self.raw,'run-scene-attachment.py','research/runtime/foreign/plan.json')
 def test_source_drift_refuses(self):
  with self.assertRaises(ValueError):adapt(self.raw+b'\n','run-scene-attachment.py',self.phase)
 def test_other_controller_refuses(self):
  with self.assertRaises(ValueError):adapt(self.raw,'run-scene-combat.py',self.phase)
 def test_all_other_code_and_guards_are_unchanged(self):
  old,new=ast.parse(self.raw),ast.parse(self.source)
  def guard(tree):return next(n for n in ast.walk(tree)if isinstance(n,ast.Call)and len(n.args)==2 and isinstance(n.args[1],ast.Constant)and n.args[1].value=='Owned prospective plan required')
  guard(new).args[0].values[0]=copy.deepcopy(guard(old).args[0].values[0]);self.assertEqual(ast.dump(old),ast.dump(new))
 def test_unique_source_and_phase_are_retained_in_proof(self):
  self.assertEqual(self.proof['source_sha256'],SOURCE_SHA256)
  self.assertEqual(self.proof['adapted_source_sha256'],hashlib.sha256(self.source).hexdigest())
  self.assertEqual(self.proof['phase'],self.phase);self.assertEqual(self.proof['changed_guards'],1)
if __name__=='__main__':unittest.main()
