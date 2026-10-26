import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from alternate_canvas_flow import inspect
def row(a,m,o):return dict(address=a,size=1,bytes='90',mnemonic=m,operand=o)
class Flow(unittest.TestCase):
 def test_cross_branch_alias(self):
  r=inspect([row(1,'mov','edi, 0x6e1f60'),row(2,'jmp','0x4'),row(4,'mov','dword ptr [edi + 8], eax')]);self.assertEqual(r['overlapping_writes'][0]['destination'],0x6e1f68)
 def test_join_conflict(self):
  r=inspect([row(1,'mov','edi, 0x6e1f68'),row(2,'jne','0x5'),row(3,'xor','edi, edi'),row(4,'jmp','0x5'),row(5,'mov','dword ptr [edi], eax')]);self.assertFalse(r['overlapping_writes']);self.assertEqual(r['unresolved_computed_stores'],1)
 def test_subregister_kills(self):
  r=inspect([row(1,'mov','edi, 0x6e1f68'),row(2,'mov','di, ax'),row(3,'mov','dword ptr [edi], eax')]);self.assertFalse(r['overlapping_writes'])
 def test_call_kills(self):
  r=inspect([row(1,'mov','edi, 0x6e1f68'),row(2,'call','0x1234'),row(3,'mov','dword ptr [edi], eax')]);self.assertFalse(r['overlapping_writes'])
 def test_overlap(self):
  r=inspect([row(1,'mov','qword ptr [0x6e1f64], xmm0')]);self.assertEqual(len(r['overlapping_writes']),1)
 def test_wide_float_overlap(self):
  r=inspect([row(1,'fstp','tbyte ptr [0x6e1f60]')]);self.assertEqual(r['overlapping_writes'][0]['width'],10)
 def test_loop_clobbers_count(self):
  r=inspect([row(1,'mov','ecx, 0x10'),row(2,'loop','0x2'),row(3,'rep stosd','dword ptr es:[edi], eax')]);self.assertIsNone(r['bulk_writers'][0]['count'])
 def test_bulk_retained(self):
  r=inspect([row(1,'mov','edi, 0x6e1f60'),row(2,'mov','ecx, 0x10'),row(3,'rep stosd','dword ptr es:[edi], eax')]);self.assertEqual(r['bulk_writers'][0]['destination'],0x6e1f60);self.assertEqual(r['bulk_writers'][0]['count'],16)
 def test_loop_loses_induction(self):
  r=inspect([row(1,'mov','edi, 0x6e1f68'),row(2,'inc','edi'),row(3,'jne','0x2'),row(4,'mov','dword ptr [edi], eax')]);self.assertFalse(r['overlapping_writes'])
 def test_external_entry(self):
  r=inspect([row(1,'mov','edi, 0x6e1f68'),row(2,'mov','dword ptr [edi], eax')],roots=[2]);self.assertFalse(r['overlapping_writes'])
 def test_escape(self):
  r=inspect([row(1,'mov','edi, 0x6e1f60'),row(2,'lea','eax, [edi + 8]'),row(3,'push','eax')]);self.assertEqual(r['escaped_pointer_addresses'][0]['pointer'],0x6e1f68)
if __name__=='__main__':unittest.main()
