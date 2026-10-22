import importlib.util
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('recovery',ROOT/'tools/inspect-original-recovery.py')
import sys
sys.path.insert(0,str(ROOT/'tools'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class References(unittest.TestCase):
    def kind(self,op,args):return m.reference_kind({'mnemonic':op,'operand':args},0x6e1f68)
    def test_direct_reads_are_distinct_from_stores_and_address_materialization(self):
        self.assertEqual(self.kind('mov','ecx,DWORD PTR ds:0x6e1f68'),'direct_load')
        self.assertEqual(self.kind('mov','DWORD PTR ds:0x6e1f68,ecx'),'direct_store')
        self.assertEqual(self.kind('mov','ecx,0x6e1f68'),'other_reference')
        self.assertEqual(self.kind('lea','ecx,ds:0x6e1f68'),'other_reference')
    def test_neighbouring_literals_and_indirect_writers_do_not_become_direct_refs(self):
        self.assertIsNone(self.kind('mov','eax,DWORD PTR ds:0x6e1f680'))
        self.assertIsNone(self.kind('mov','DWORD PTR [eax],ecx'))
        self.assertEqual(self.kind('cmp','DWORD PTR ds:0x6e1f68,0'),'other_reference')


if __name__=='__main__':unittest.main()
