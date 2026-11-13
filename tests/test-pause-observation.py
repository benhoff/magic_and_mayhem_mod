#!/usr/bin/env python3
"""Explicit optional API journal baseline retains all other pause proof guards."""
import ast,inspect,importlib.util,sys,unittest
from pathlib import Path
from unittest.mock import Mock,patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_pause_observation as selected
spec=importlib.util.spec_from_file_location('original_pause_controls',Path(__file__).with_name('test-interaction-public-pause.py'));old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
class ExistingGuards(old.Proof):
    def assess(self):
        q=selected.bind(lambda raw:dict(wire_version=3,rows=self.rows))
        q.__globals__['artifact']=lambda *args:b'fixture';q.__globals__['optional_keys']=lambda *args:self.keys
        return q(None,self.a)
class Extras(unittest.TestCase):
    def setup_proof(self):
        p=old.Proof();p.setUp();return p
    def assess(self,p):
        f=selected.bind(lambda raw:dict(wire_version=3,rows=p.rows));f.__globals__['artifact']=lambda *args:b'fixture';f.__globals__['optional_keys']=lambda *args:None;return f(None,p.a)
    def test_absent_is_explicit(self):
        q=self.assess(self.setup_proof());self.assertTrue(q['verified']);self.assertTrue(all(not x['original_API_poll_proved']and not x['original_key_message_handler_proved']for x in q['original_Space_key_proofs']))
    def test_optional_absent(self):self.assertIsNone(selected.optional_keys(None,{'artifacts':[]}))
    def test_ambiguous_journal_refused(self):
        with self.assertRaises(ValueError):selected.optional_keys(None,{'artifacts':[{'path':'x/ui-acceptance.bin.keys'}]*2})
    def test_present_is_strictly_decoded(self):
        with patch.object(selected,'artifact',return_value=b'bad'):
            with self.assertRaises(ValueError):selected.optional_keys(None,{'artifacts':[{'path':'x/ui-acceptance.bin.keys'}]})
    def test_AST_reversal(self):
        tree=ast.parse(selected.selected_source(inspect.getsource(selected.original)))
        class Restore(ast.NodeTransformer):
            def visit_If(self,node):
                if ast.dump(node.test)==ast.dump(ast.parse('key_trace is not None',mode='eval').body):return node.body
                return self.generic_visit(node)
            def visit_Call(self,node):
                if isinstance(node.func,ast.Name)and node.func.id=='optional_keys':return ast.parse(selected.KEY).body[0].value
                return self.generic_visit(node)
        self.assertEqual(ast.dump(Restore().visit(tree),include_attributes=False),ast.dump(ast.parse(inspect.getsource(selected.original)),include_attributes=False))
    def test_changed_API_predicate_refused(self):
        with self.assertRaises(ValueError):selected.selected_source(inspect.getsource(selected.original).replace('len(edges)==2','len(edges)==3'))
    def test_absent_still_requires_physical(self):
        p=self.setup_proof();del p.actions[0]
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_alive(self):
        p=self.setup_proof();p.rows[6]['wizard']['health']=0
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_paused_counter(self):
        p=self.setup_proof();p.rows[6]['counter']=100
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_resume(self):
        p=self.setup_proof();p.rows[-1]['counter']=p.rows[-2]['counter']
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_nonce(self):
        p=self.setup_proof();p.a['resident']['nonce']='foreign'
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_down_up(self):
        p=self.setup_proof();p.actions[0]['down_up']=False
        with self.assertRaises(ValueError):self.assess(p)
    def test_absent_still_requires_focus(self):
        p=self.setup_proof();p.actions[0]['host_focus']['host_focused']=False
        with self.assertRaises(ValueError):self.assess(p)
    def test_actual_pause_wire_version_required(self):
        p=self.setup_proof();f=selected.bind(lambda raw:dict(wire_version=2,rows=p.rows));f.__globals__['artifact']=lambda *args:b'fixture'
        with self.assertRaises(ValueError):f(None,p.a)
if __name__=='__main__':unittest.main()
