#!/usr/bin/env python3
"""All four setup guards bind exact phases without changing other controller code."""
import ast,copy,sys,types,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_canonical_plan_guard import adapt,ENTRYPOINTS,PREFIX,binding_class
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
class Guards(unittest.TestCase):
 def guard(self,tree):return next(n for n in ast.walk(tree)if isinstance(n,ast.Call)and len(n.args)==2 and isinstance(n.args[1],ast.Constant)and n.args[1].value in['Owned prospective plan required','Owned prospective picking plan required'])
 def check_family(self,family):
  controller,_=ENTRYPOINTS[family];raw=(ROOT/'tools'/controller).read_bytes();phase=PREFIX+'CONTROL-'+family+'.json';code,source,proof=adapt(raw,controller,phase);new=ast.parse(source);old=ast.parse(raw);call=self.guard(new)
  for value,accepted in[(phase,True),(phase+'.extra',False),('other/'+phase,False)]:
   namespace=dict(args=types.SimpleNamespace(plan=value),require=require,base=types.SimpleNamespace(require=require),Path=Path,PREFIX='old/');test=compile(ast.fix_missing_locations(ast.Module(body=[ast.Expr(value=copy.deepcopy(call))],type_ignores=[])),'guard','exec')
   if accepted:exec(test,namespace)
   else:
    with self.assertRaises(ValueError):exec(test,namespace)
  original=self.guard(old);call.args[0]=copy.deepcopy(original.args[0]);self.assertEqual(ast.dump(old),ast.dump(new));self.assertEqual(proof['changed_guards'],1)
 def test_attachment(self):self.check_family('attachment')
 def test_font(self):self.check_family('font')
 def test_palette(self):self.check_family('palette')
 def test_picking(self):self.check_family('picking')
 def test_drift_refuses_for_every_family(self):
  for controller,_ in ENTRYPOINTS.values():
   with self.subTest(controller=controller),self.assertRaises(ValueError):adapt((ROOT/'tools'/controller).read_bytes()+b'\n',controller,PREFIX+'phase.json')
 def test_unknown_controller_refuses(self):
  with self.assertRaises(ValueError):binding_class(object,'unknown.py')
 def test_existing_binding_is_preserved_as_parent(self):
  class Original:pass
  self.assertTrue(issubclass(binding_class(Original,'run-palette-cycle.py'),Original))
if __name__=='__main__':unittest.main()
