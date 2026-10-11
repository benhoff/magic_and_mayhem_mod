#!/usr/bin/env python3
"""Explicit observation modes cannot silently discard or invent core evidence."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_cohorts import cohort,require_bindings


class CohortTests(unittest.TestCase):
    def test_historical_core_default_is_explicit(self):
        self.assertEqual(cohort({}),'core')

    def test_core_api_and_menu_control_remain_separate(self):
        self.assertEqual(cohort({'api_observation_enabled':True}),'core_api')
        self.assertEqual(cohort({'call_census_enabled':False}),'menu_control')

    def test_api_only_requires_all_three_exact_flags(self):
        self.assertEqual(cohort({'api_observation_enabled':True,'call_census_enabled':False,'api_only':True}),'api_only')
        for metadata in [{'api_only':True},{'api_only':True,'api_observation_enabled':True},{'api_only':True,'call_census_enabled':False},{'api_observation_enabled':True,'call_census_enabled':False}]:
            with self.assertRaises(ValueError):cohort(metadata)

    def test_flags_reject_truthy_non_boolean_values(self):
        for name in ['api_observation_enabled','call_census_enabled','api_only']:
            for value in [0,1,'false',[],None]:
                with self.assertRaises(ValueError):cohort({name:value})

    def test_api_only_requires_actual_core_module_absence(self):
        require_bindings({'modules':{'core':{'base':0}},'core_leave':0},'api_only')
        for base,leave in [(0x700000,0),(0,0x700100),(0x700000,0x700100)]:
            with self.assertRaises(ValueError):require_bindings({'modules':{'core':{'base':base}},'core_leave':leave},'api_only')

    def test_core_api_requires_exact_loaded_core_leave(self):
        require_bindings({'modules':{'core':{'base':0x700000}},'core_leave':0x700100},'core_api')
        for base,leave in [(0,0),(0x700000,0),(0,0x700100)]:
            with self.assertRaises(ValueError):require_bindings({'modules':{'core':{'base':base}},'core_leave':leave},'core_api')

    def test_non_api_cohorts_cannot_claim_api_bindings(self):
        for selected in ['core','menu_control','unknown']:
            with self.assertRaises(ValueError):require_bindings({'modules':{'core':{'base':0}},'core_leave':0},selected)


if __name__=='__main__':unittest.main()
