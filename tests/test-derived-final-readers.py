#!/usr/bin/env python3
import ast,copy,inspect,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_derived_final_runner as runner
import broader_derived_final_gate as gate
from broader_player_derived_final_matrix import current
from broader_player_current_interaction_final_matrix import original_final_current
class Selection(unittest.TestCase):
 def test_every_other_final_reader_AST_node_unchanged(self):
  for kind,name in runner.READERS.items():
   source=(ROOT/'tools'/name).read_text();tree=ast.parse(runner.selected_source(source,kind));inverse={v:k for k,v in runner.LITERALS.items()}
   if kind=='acceptance':
    f=next(n for n in tree.body if isinstance(n,ast.FunctionDef)and n.name=='assess');i=next(i for i,n in enumerate(f.body)if isinstance(n,ast.Assign)and isinstance(n.targets[0],ast.Name)and n.targets[0].id=='derived');self.assertEqual(ast.dump(ast.Module(body=f.body[i:i+7],type_ignores=[]),include_attributes=False),ast.dump(ast.parse(runner.INSERT),include_attributes=False));del f.body[i:i+7]
   class Restore(ast.NodeTransformer):
    def visit_ImportFrom(self,node):
     if node.module==runner.MATRIX:node.module='broader_player_secondary_final_matrix'
     elif node.module==runner.GATE:node.module='broader_secondary_final_gate'
     return node
    def visit_Constant(self,node):
     if type(node.value)in(str,int)and node.value in inverse:node.value=inverse[node.value]
     return node
   self.assertEqual(ast.dump(ast.parse(runner.previous_source(source,kind)),include_attributes=False),ast.dump(Restore().visit(tree),include_attributes=False))
 def test_prior_supplement_changes_only_new_qualified_pilot_receipt_and_counts(self):
  original=inspect.getsource(gate.previous.supplement);tree=ast.parse(gate.selected_prior_source(original));inverse={v:k for k,v in gate.PILOT_CHANGES.items()}
  class Restore(ast.NodeTransformer):
   def visit_Constant(self,node):
    if type(node.value)in(str,int)and node.value in inverse:node.value=inverse[node.value]
    return node
  self.assertEqual(ast.dump(ast.parse(original),include_attributes=False),ast.dump(Restore().visit(tree),include_attributes=False));self.assertEqual(gate._SECONDARY.__globals__['PILOT'],gate.PILOT);self.assertEqual(gate.previous.supplement.__globals__['PILOT'],gate.FAILED)
 def test_changed_prior_pilot_literal_refuses(self):
  with self.assertRaises(ValueError):gate.selected_prior_source(inspect.getsource(gate.previous.supplement).replace('==154','==153'))
 def test_prior_other_predicate_code_objects_unchanged(self):
  for name in ['_FADING','_CALLER','_PRIOR','_WITNESS','_REBUILD','_UNCOVERED','_PAN','_COLLECTION_PROJECT','_COLLECTION','_STRICT_PROJECT','_STRICT','_DISCOVERY_PROJECT','_DISCOVERY']:
   self.assertIs(getattr(gate,name).__code__,getattr(gate.previous,name).__code__,name)
  self.assertIs(gate._SECONDARY_PROJECT.__code__,gate.previous.project_rows.__code__)
 def test_prior_budget_validator_changes_only_prospective_pilot_binding(self):
  self.assertIs(gate._PRIOR_SECONDARY_VALIDATE.__code__,gate.previous.validate_rows.__code__);self.assertEqual(gate._PRIOR_SECONDARY_VALIDATE.__globals__['PILOT'],gate.PILOT)
 def test_final_full_Native_predicate_code_object_unchanged(self):self.assertIs(current.__code__,original_final_current.__code__)
 def test_actual_incomplete_ledger_refuses(self):
  with self.assertRaises(ValueError):gate.project_rows({},[],{})
 def test_prior_incomplete_ledger_still_refuses(self):
  with self.assertRaises(ValueError):gate._SECONDARY_PROJECT({},[],{})
 def test_new_prospective_identity_only(self):
  original={gate.FAILED:1,'other':1};before=copy.deepcopy(original);self.assertEqual(gate.latest_reservations(original),{gate.PILOT:1,'other':1});self.assertEqual(original,before)
 def test_duplicate_new_pilot_refuses(self):
  with self.assertRaises(ValueError):gate.latest_reservations({gate.FAILED:1,gate.PILOT:1})
 def test_wrong_prospective_charge_refuses(self):
  with self.assertRaises(ValueError):gate.latest_reservations({gate.FAILED:2})
 def test_deep_identity_mapping_composes_both_replacements(self):
  original={gate.previous.FAILED:1,'other':1};self.assertEqual(gate.rebound(original),{gate.PILOT:1,'other':1});self.assertIn(gate.previous.FAILED,original)
 def test_derived_lineage_retains_both_prior_secondary_catalogs(self):
  self.assertIn('derived-source-catalog-03.json',gate._lineage_source);self.assertIn("names=['derived-source-catalog-02.json','derived-source-catalog-01.json','secondary-source-catalog-02.json','secondary-source-catalog-01.json'",gate._lineage_source)
if __name__=='__main__':unittest.main()
