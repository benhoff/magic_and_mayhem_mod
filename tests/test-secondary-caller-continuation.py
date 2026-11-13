#!/usr/bin/env python3
import copy
import importlib.util
from pathlib import Path
import sys
import unittest
from types import SimpleNamespace
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_secondary_budget import validate_rows,FAILED,PILOT


class Rows(unittest.TestCase):
    def setUp(self):
        self.old=[dict(case_id='old'+str(i),report='old-report'+str(i),reserved_games=50 if i==0 else 1,state='admitted',acquisition_sha256='old-sha')for i in range(153)]
        self.old[-1].update(case_id=FAILED,state='failed',report='failed-'+FAILED)
        self.new={PILOT:1}|{'new'+str(i):1 for i in range(44)}
        self.rows=copy.deepcopy(self.old)
    def positive(self,cid):return dict(case_id=cid,report='report-'+cid,reserved_games=1,state='admitted',acquisition_sha256='new-sha')
    def refuses(self):
        with self.assertRaises(ValueError):validate_rows(self.rows,self.old,self.new)
    def test_exact153rows202charges(self):self.assertEqual(validate_rows(self.rows,self.old,self.new),202)
    def test_all45newpositive_rows198and247(self):
        self.rows.extend(self.positive(cid)for cid in self.new)
        self.assertEqual((len(self.rows),validate_rows(self.rows,self.old,self.new)),(198,247))
    def test_failed_old_row_cannot_be_promoted(self):self.rows[-1]['state']='admitted';self.refuses()
    def test_failed_old_hash_cannot_be_refreshed(self):self.rows[-1]['acquisition_sha256']='changed';self.refuses()
    def test_failed_old_charge_cannot_be_refunded(self):self.rows[-1]['reserved_games']=0;self.refuses()
    def test_failed_old_row_cannot_be_omitted(self):self.rows.pop();self.refuses()
    def test_unknown_new_id_refuses(self):self.rows.append(self.positive('unknown'));self.refuses()
    def test_duplicate_new_id_refuses(self):self.rows.extend([self.positive(PILOT),self.positive(PILOT)]);self.refuses()
    def test_new_requested_row_stops_expansion(self):
        row=self.positive(PILOT);row['state']='requested';self.rows.append(row);self.refuses()
    def test_new_failed_row_stops_expansion(self):
        row=self.positive(PILOT);row['state']='failed';self.rows.append(row);self.refuses()
    def test_no_spare_reservation(self):self.new['spare']=1;self.refuses()
    def test_old_case_cannot_be_retried_in_new_domain(self):self.new.pop(PILOT);self.new[FAILED]=1;self.refuses()
    def test_missing_positive_acquisition_hash_refuses(self):
        row=self.positive(PILOT);row['acquisition_sha256']=None;self.rows.append(row);self.refuses()
    def test_new_case_cannot_charge_another_start(self):
        row=self.positive(PILOT);row['reserved_games']=2;self.rows.append(row);self.refuses()


class Request(unittest.TestCase):
    def test_original_main_code_and_guards_unchanged(self):
        def load(name,path):
            spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
        original=load('secondary_original_request_control',ROOT/'tools/run-broader-player-case.py')
        wrapper=load('secondary_current_request_control',ROOT/'tools/run-broader-secondary-source-case.py')
        selected=wrapper.request_module()
        self.assertEqual(selected.main.__code__.co_code,original.main.__code__.co_code)
        self.assertEqual(selected.main.__code__.co_consts,original.main.__code__.co_consts)
        self.assertEqual(selected.main.__code__.co_names,original.main.__code__.co_names)
        self.assertEqual(selected.main.__code__.co_filename,original.main.__code__.co_filename)
        reader=selected.Reader(ROOT);self.assertTrue(reader._memoize);reader.close()
        self.assertIs(selected.cumulative_bound,wrapper.bound)
        self.assertIs(selected.reviewed_predecessors,wrapper.predecessors)



class BeforeOperation(unittest.TestCase):
    def setUp(self):
        spec=importlib.util.spec_from_file_location('secondary_preoperation_control',ROOT/'tools/run-broader-secondary-source-case.py')
        self.wrapper=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.wrapper)
        self.phase=dict(subprocess_scene_binding=dict(startup='patches/secondary-scene-python-startup/sitecustomize.py',helper='tools/broader_secondary_subprocess.py',source_waivers=[],sources={}),family='interaction',scene_runtime_variant={'record':'build','sha256':'build'},broader_cases=[{'id':'case','report':'acquisition'}])
        owner=self
        class FakeReader:
            def __init__(self,*a,**kw):pass
            def __enter__(self):return self
            def __exit__(self,*a):pass
            def digest(self,n):return n
            def json(self,n):return owner.phase
        self.reader=patch.object(self.wrapper,'Reader',FakeReader);self.reader.start();self.addCleanup(self.reader.stop)
    def test_bad_materialized_variant_refuses_before_reservation(self):
        with patch.object(self.wrapper.sys,'argv',['driver','acquire','--plan','phase','--case','case']), patch.object(self.wrapper,'verify_variant',side_effect=ValueError('changed')), patch.object(self.wrapper,'request_module') as core:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called()
    def test_bad_staged_join_refuses_before_admission(self):
        with patch.object(self.wrapper.sys,'argv',['driver','admit','--plan','phase','--case','case','--validation','validation']), patch.object(self.wrapper,'verify_variant',return_value={}), patch.object(self.wrapper,'verify_staged',side_effect=ValueError('wrong staged DLL')), patch.object(self.wrapper,'request_module') as core, patch.object(self.wrapper,'write_report') as writer:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called();writer.assert_not_called()
    def test_bad_actual_child_refuses_before_admission(self):
        with patch.object(self.wrapper.sys,'argv',['driver','admit','--plan','phase','--case','case','--validation','validation']),patch.object(self.wrapper,'verify_variant',return_value={}),patch.object(self.wrapper,'verify_staged',return_value={}),patch.object(self.wrapper,'verify_selection',side_effect=ValueError('wrong child selection')),patch.object(self.wrapper,'request_module')as core,patch.object(self.wrapper,'write_report')as writer:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called();writer.assert_not_called()
    def test_variant_in_foreign_family_refuses(self):
        self.phase['family']='pause'
        with patch.object(self.wrapper.sys,'argv',['driver','acquire','--plan','phase','--case','case']), patch.object(self.wrapper,'request_module') as core:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called()
    def test_unmodified_base_phase_delegates_original_operation(self):
        self.phase.pop('scene_runtime_variant');self.phase['family']='pause';calls=[]
        core=SimpleNamespace(main=lambda:calls.append('old-main')or 0)
        with patch.object(self.wrapper.sys,'argv',['driver','acquire','--plan','phase','--case','case']),patch.object(self.wrapper,'request_module',return_value=core),patch.object(self.wrapper,'verify_variant')as verify:
            self.assertEqual(self.wrapper.main(),0)
            verify.assert_not_called();self.assertEqual(calls,['old-main'])

if __name__=='__main__':unittest.main()
