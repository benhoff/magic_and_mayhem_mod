"""Require the exact specialized proof flags before broad current admission."""
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('current_world_checker',ROOT/'tools/check-broader-world.py');checker=importlib.util.module_from_spec(spec);spec.loader.exec_module(checker)


class CurrentWorldAdmissionGuards(unittest.TestCase):
    def test_every_family_requires_its_explicit_flag(self):
        for family,field in checker.FIELDS.items():
            flag='semantic_route_verified'if family in('victory','defeat')else'verified'
            with self.subTest(family=family):
                value={flag:True};self.assertIs(checker.specialized_semantics(family,{field:value}),value)
                for invalid in({},None,{flag:False},{flag:1},{'success':True}):
                    with self.assertRaises(ValueError):checker.specialized_semantics(family,{field:invalid})

    def test_missing_specialized_output_is_not_inferred(self):
        for family in checker.FIELDS:
            with self.subTest(family=family),self.assertRaises(ValueError):checker.specialized_semantics(family,{'success':True})

    def test_unknown_family_is_not_generic_success(self):
        with self.assertRaises(ValueError):checker.specialized_semantics('unobserved',{'proof':{'verified':True}})

    def test_pixel_success_cannot_replace_result_or_font_semantics(self):
        for family in('victory','defeat','font'):
            with self.subTest(family=family),self.assertRaises(ValueError):checker.specialized_semantics(family,{'comparison':{'success':True,'mismatching_pixels':0}})


if __name__=='__main__':unittest.main()
