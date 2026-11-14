"""The RAM binding may change only the exact derived output initializer."""
import ast
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('broader_ram_binding',ROOT/'tools/check-broader-recovery-ram.py');binding=importlib.util.module_from_spec(spec);spec.loader.exec_module(binding)
SOURCE=b'''from pathlib import Path
ROOT=Path('/owned')
def main(surface_loss=False):
 output_root=ROOT/('working/surface-loss/comparisons'if surface_loss else'working/recovery-closeout08/comparisons')
 assert required_input=='unchanged'
 return output_root, deadline
'''


class RamBindingGuards(unittest.TestCase):
    def test_both_modes_keep_input_assertion_and_deadline(self):
        code,_=binding.memory_output_code(SOURCE,'source.py');n=dict(BROADER_RAM_COMPARISON_ROOT=Path('/ram'),required_input='unchanged',deadline=300);exec(code,n)
        self.assertEqual(n['main'](),(Path('/ram'),300));self.assertEqual(n['main'](True),(Path('/ram'),300))
        n['required_input']='changed'
        with self.assertRaises(AssertionError):n['main']()

    def test_unknown_output_expression_refuses(self):
        with self.assertRaises(ValueError):binding.memory_output_code(SOURCE.replace(b'working/recovery-closeout08/comparisons',b'other'),'source.py')

    def test_missing_initializer_refuses(self):
        with self.assertRaises(ValueError):binding.memory_output_code(SOURCE.replace(b'output_root=',b'other='),'source.py')

    def test_duplicate_initializer_refuses(self):
        duplicate=SOURCE+SOURCE[SOURCE.index(b'def main'):].replace(b'def main',b'def other')
        with self.assertRaises(ValueError):binding.memory_output_code(duplicate,'source.py')

    def test_actual_producer_matches_reviewed_initializer(self):
        code,digest=binding.memory_output_code((ROOT/binding.SOURCE).read_bytes(),str(ROOT/binding.SOURCE));self.assertEqual(len(digest),64);self.assertIsNotNone(code)


if __name__=='__main__':unittest.main()
