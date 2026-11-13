#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_derived_paths import rebind


class Paths(unittest.TestCase):
    def setUp(self):
        self.prefix='research/runtime/family/continuation-47/'
        self.phase=dict(collector_prefix=self.prefix,bounds={'same':32},counter_bindings={'actual.sqlite3':'working/broader-player/bp47/family/counter.sqlite3'},broader_cases=[dict(id='new',reserved_original_games=1,report=self.prefix+'new-acquisition.json',comparison=self.prefix+'new-comparison-01.json',historical_primary={'record':'historical','sha256':'unchanged'},command=['python3','controller','--report',self.prefix+'new-acquisition.json'],comparison_command=['python3','compare','--report',self.prefix+'new-comparison-01.json'])])
    def test_reports_and_counters_rebind_together(self):
        q=rebind(self.phase);self.assertEqual(q['collector_prefix'],'research/runtime/family/continuation-48/');self.assertIn('/bp48/',q['counter_bindings']['actual.sqlite3'])
        self.assertEqual(q['broader_cases'][0]['command'][-1],q['broader_cases'][0]['report'])
        self.assertEqual(q['broader_cases'][0]['comparison_command'][-1],q['broader_cases'][0]['comparison'])
    def test_historical_inputs_and_bounds_stay_exact(self):
        before=copy.deepcopy(self.phase);q=rebind(self.phase);self.assertEqual(self.phase,before);self.assertEqual(q['broader_cases'][0]['historical_primary'],before['broader_cases'][0]['historical_primary']);self.assertEqual(q['bounds'],before['bounds'])
    def test_other_collector_namespace_refuses(self):
        self.phase['collector_prefix']='research/runtime/family/';
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_external_output_refuses(self):
        self.phase['broader_cases'][0]['report']='research/runtime/other/run.json'
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_nested_output_refuses(self):
        self.phase['broader_cases'][0]['comparison']=self.prefix+'nested/run.json'
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_counter_alias_refuses(self):
        self.phase['counter_bindings']['other']='working/broader-player/bp47/family/counter.sqlite3'
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_old_counter_namespace_refuses(self):
        self.phase['counter_bindings']['actual.sqlite3']='working/broader-player/bp40/family/counter.sqlite3'
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_multiple_start_case_refuses(self):
        self.phase['broader_cases'][0]['reserved_original_games']=2
        with self.assertRaises(ValueError):rebind(self.phase)
    def test_duplicate_acquisition_refuses(self):
        self.phase['broader_cases'].append(copy.deepcopy(self.phase['broader_cases'][0]))
        with self.assertRaises(ValueError):rebind(self.phase)


class Coordinators(unittest.TestCase):
    def test_all_domain_predicates_and_steps_unchanged(self):
        for role in ['domain','selected-domain']:
            old=(ROOT/('tools/run-broader-secondary-'+role+'-case.py')).read_text();new=(ROOT/('tools/run-broader-derived-'+role+'-case.py')).read_text()
            self.assertNotEqual(old,new);self.assertEqual(new.replace('run-broader-derived-','run-broader-secondary-'),old)


if __name__=='__main__':unittest.main()
