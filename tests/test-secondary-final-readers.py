#!/usr/bin/env python3
import ast,copy,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_secondary_final_runner as runner
import broader_secondary_final_gate as gate
from broader_player_current_interaction_final_matrix import original_final_current
from broader_player_secondary_final_matrix import current

class Selection(unittest.TestCase):
 def test_every_other_AST_node_unchanged(self):
  for kind,name in runner.READERS.items():
   source=(ROOT/'tools'/name).read_text();tree=ast.parse(runner.selected_source(source,kind));inverse={v:k for k,v in runner.LITERALS.items()if k!=v}
   class Restore(ast.NodeTransformer):
    def visit_ImportFrom(self,node):
     if node.module==runner.MATRIX:node.module='broader_player_discovery_final_matrix'
     elif node.module==runner.GATE:node.module='broader_discovery_final_gate'
     return node
    def visit_Constant(self,node):
     if type(node.value)in(str,int)and node.value in inverse:node.value=inverse[node.value]
     return node
   if kind=='acceptance':
    fn=next(v for v in tree.body if isinstance(v,ast.FunctionDef)and v.name=='assess')
    start=next(i for i,v in enumerate(fn.body)if isinstance(v,ast.Assign)and isinstance(v.targets[0],ast.Name)and v.targets[0].id=='secondary')
    self.assertEqual(ast.dump(ast.Module(body=fn.body[start:start+3],type_ignores=[]),include_attributes=False),ast.dump(ast.parse(runner.INSERT),include_attributes=False));del fn.body[start:start+3]
   self.assertEqual(ast.dump(ast.parse(source),include_attributes=False),ast.dump(Restore().visit(tree),include_attributes=False))
 def test_foreign_reader_refuses(self):
  with self.assertRaises(ValueError):runner.selected_source('', 'other')
 def test_changed_constant_refuses(self):
  s=(ROOT/'tools'/runner.READERS['acceptance']).read_text().replace('len(rows)==197','len(rows)==196')
  with self.assertRaises(ValueError):runner.selected_source(s,'acceptance')
 def test_missing_history_insertion_refuses(self):
  s=(ROOT/'tools'/runner.READERS['acceptance']).read_text().replace('wanted=','unreviewed=')
  with self.assertRaises(ValueError):runner.selected_source(s,'acceptance')
 def test_final_current_code_unchanged(self):self.assertIs(current.__code__,original_final_current.__code__)
 def test_every_inherited_predicate_code_unchanged(self):
  for name in ['_FADING','_CALLER','_PRIOR','_WITNESS','_REBUILD','_UNCOVERED','_PAN','_COLLECTION_PROJECT','_COLLECTION','_STRICT_PROJECT','_STRICT']:
   self.assertIs(getattr(gate,name).__code__,getattr(gate.previous,name).__code__,name)
  self.assertIs(gate._DISCOVERY_PROJECT.__code__,gate.previous.project_rows.__code__)
  self.assertIs(gate._DISCOVERY.__code__,gate.previous.supplement.__code__)
 def test_prospective_map_only(self):
  value={gate.FAILED:1,'other':1};old=copy.deepcopy(value);self.assertEqual(gate.rebound(value),{gate.PILOT:1,'other':1});self.assertEqual(value,old)
 def test_wrong_reservation_refuses(self):
  with self.assertRaises(ValueError):gate.rebound({gate.FAILED:2})
 def test_duplicate_replacement_refuses(self):
  with self.assertRaises(ValueError):gate.rebound({gate.FAILED:1,gate.PILOT:1})
 def test_incomplete_actual_ledger_refuses(self):
  with self.assertRaises(ValueError):gate.project_rows({},[],{})
 def test_partial_old_ledger_still_refuses(self):
  with self.assertRaises(ValueError):gate._DISCOVERY_PROJECT({},[],{})
 def test_new_lineage_keeps_all_ancestors(self):
  self.assertIn('secondary-source-catalog-01.json',gate._lineage_source)
  self.assertIn("names=['discovery-source-catalog-01.json','strict-binding-source-catalog-01.json'",gate._lineage_source)

if __name__=='__main__':unittest.main()
