#!/usr/bin/env python3
"""Analysis routing controls preserve acquisition and other coordinator guards."""
import ast,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_observed_pause_dispatch import comparison,admission,selected_source,OLD_COMPARE,NEW_COMPARE,OLD_ADMIT,NEW_ADMIT
class Tests(unittest.TestCase):
    def test_comparison_only_tool_changes(self):
        cmd=['python3',OLD_COMPARE,'--acquisition','exact','--analysis-plan','phase','--report','out'];got=comparison(cmd);self.assertEqual(got,['python3',NEW_COMPARE,*cmd[2:]]);self.assertEqual(cmd[1],OLD_COMPARE)
    def test_admission_only_tool_changes(self):
        cmd=['python3',OLD_ADMIT,'admit','--plan','exact','--case','unique','--validation','full'];self.assertEqual(admission(cmd),['python3',NEW_ADMIT,*cmd[2:]]);self.assertEqual(cmd[1],OLD_ADMIT)
    def test_acquire_cannot_route(self):
        with self.assertRaises(ValueError):admission(['python3',OLD_ADMIT,'acquire'])
    def test_foreign_compare_refuses(self):
        with self.assertRaises(ValueError):comparison(['python3','foreign'])
    def test_foreign_admit_refuses(self):
        with self.assertRaises(ValueError):admission(['python3','foreign','admit'])
    def test_nonlist_refuses(self):
        with self.assertRaises(ValueError):comparison(('python3',OLD_COMPARE))
    def test_AST_reversal(self):
        path=Path(__file__).resolve().parents[1]/'tools/run-broader-derived-domain-case.py';source=path.read_text();tree=ast.parse(selected_source(source))
        class Restore(ast.NodeTransformer):
            def visit_Call(self,node):
                if isinstance(node.func,ast.Name)and node.func.id in('_observed_compare','_observed_admit'):
                    self.assertion=None;return node.args[0]
                return self.generic_visit(node)
        self.assertEqual(ast.dump(Restore().visit(tree),include_attributes=False),ast.dump(ast.parse(source),include_attributes=False))
    def test_missing_compare_refuses(self):
        with self.assertRaises(ValueError):selected_source("execute('admit', command)")
    def test_duplicate_admit_refuses(self):
        with self.assertRaises(ValueError):selected_source("execute('compare', command)\nexecute('admit', command)\nexecute('admit', command)")
    def test_final_current_code_preserved(self):
        from broader_player_terminal_final_matrix import current
        from broader_player_discovery_final_matrix import current as original
        self.assertIs(current.__code__,original.__code__)
if __name__=='__main__':unittest.main()
