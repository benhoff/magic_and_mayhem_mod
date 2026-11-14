#!/usr/bin/env python3
"""Terminal UI identity controls use synthetic bounded wire observations."""
import ast,inspect,struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_terminal_ui_identity import TerminalUi,selected_decoder_source,MESSAGE
from rendering_ui_acceptance import decode

def frames(*items):
    rows=[]
    for i,(returned,source,tick) in enumerate(items,1):
        w=[0]*96;w[:8]=[i,0,1,returned,tick,77,0,256];w[45]=4
        w[56:64]=[1,32,0x40209ca7,source,0,123,0,77];rows.append(struct.pack('<96I',*w))
    return b'MNMIFR01'+struct.pack('<2I',1,384)+b''.join(rows)
def ui(returned=3,source=1,top=27,tick=102):
    w=[0]*128;w[:6]=[1,1,0,tick,77,returned];w[16]=top;w[50]=12
    w[56:64]=[1,32,0x40209ca7,source,0,123,0,77]
    return w
def wire(*rows):return b'MNMUIA01'+struct.pack('<2I',3,512)+b''.join(struct.pack('<128I',*w)for w in rows)
class Tests(unittest.TestCase):
    def make(self,w=None,f=None,**kw):
        return TerminalUi(wire(w or ui()),f or frames((1,1,100),(2,1,101),(3,1,102)),kw.get('completed',1),kw.get('nonce','123'),kw.get('normal',True))
    def test_exact_terminal_join_and_no_credit(self):
        q=self.make();self.assertEqual(q(q.raw)['rows'][0]['world_returns'],3)
        self.assertEqual(q.diagnostics[0]['frame_input_row'],3)
        self.assertEqual(q.diagnostics[0]['source_World_credit'],0);self.assertEqual(q.diagnostics[0]['action_witness_credit'],0)
    def test_original_rejects_extended_identity(self):
        with self.assertRaises(ValueError):decode(wire(ui()))
    def test_unchanged_old_valid_rows(self):
        for returned,top in ((1,2),(2,27)):
            q=self.make(ui(returned=returned,top=top));self.assertEqual(q(q.raw),decode(q.raw));self.assertEqual(q.diagnostics,[])
    def test_active_world_refused(self):
        q=self.make(ui(top=2))
        with self.assertRaises(ValueError):q(q.raw)
    def test_unmatched_return_refused(self):
        q=self.make(ui(returned=4))
        with self.assertRaises(ValueError):q(q.raw)
    def test_wrong_nonce_refused(self):
        q=self.make(nonce='124')
        with self.assertRaises(ValueError):q(q.raw)
    def test_wrong_thread_refused(self):
        w=ui();w[4]=w[63]=78;q=self.make(w)
        with self.assertRaises(ValueError):q(q.raw)
    def test_wrong_source_extent_refused(self):
        with self.assertRaises(ValueError):self.make(completed=2)
    def test_nonfinal_source_refused(self):
        q=self.make(f=frames((1,1,100),(3,1,102),(4,2,103)),completed=2)
        with self.assertRaises(ValueError):q(q.raw)
    def test_tick_before_frame_refused(self):
        q=self.make(ui(tick=101))
        with self.assertRaises(ValueError):q(q.raw)
    def test_tick_after_next_world_refused(self):
        q=self.make(ui(tick=104),frames((1,1,100),(3,1,102),(4,1,103)))
        with self.assertRaises(ValueError):q(q.raw)
    def test_duplicate_frame_identity_refused(self):
        q=self.make(f=frames((1,1,100),(3,1,102),(3,1,102)))
        with self.assertRaises(ValueError):q(q.raw)
    def test_later_active_ui_refused(self):
        a=ui();b=ui(returned=1,top=2,tick=103);b[0]=2
        q=TerminalUi(wire(a,b),frames((1,1,100),(3,1,102)),1,'123',True)
        with self.assertRaises(ValueError):q(q.raw)
    def test_reserved_guard_preserved(self):
        w=ui();w[36]=1;q=self.make(w)
        with self.assertRaises(ValueError):q(q.raw)
    def test_partial_ui_refused(self):
        with self.assertRaises(ValueError):TerminalUi(wire(ui())[:-1],frames((1,1,100)),1,'123',True)
    def test_not_normally_retired_refused(self):
        with self.assertRaises(ValueError):self.make(normal=False)
    def test_foreign_bytes_refused(self):
        q=self.make()
        with self.assertRaises(ValueError):q(wire(ui(top=26)))
    def test_pending_mode_refused(self):
        q=self.make()
        with self.assertRaises(ValueError):q(q.raw,allow_pending=True)
    def test_exact_other_AST_preserved(self):
        original=inspect.getsource(decode);tree=ast.parse(selected_decoder_source(original))
        class Restore(ast.NodeTransformer):
            def visit_Call(self,node):
                if isinstance(node.func,ast.Name)and node.func.id=='require'and len(node.args)==2 and isinstance(node.args[1],ast.Constant)and node.args[1].value==MESSAGE:
                    node.args[0]=ast.parse("frame and (w[5]==frame['generation'] or w[16]!=2 and w[5]==frame['generation']+1)",mode='eval').body
                return self.generic_visit(node)
        self.assertEqual(ast.dump(Restore().visit(tree),include_attributes=False),ast.dump(ast.parse(original),include_attributes=False))
    def test_changed_original_guard_refused(self):
        source=inspect.getsource(decode).replace("frame['generation']+1","frame['generation']+2")
        with self.assertRaises(ValueError):selected_decoder_source(source)
if __name__=='__main__':unittest.main()
