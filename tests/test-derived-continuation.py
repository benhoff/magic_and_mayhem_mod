#!/usr/bin/env python3
import copy
import importlib.util
from pathlib import Path
import sys
import unittest
from types import SimpleNamespace
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_derived_budget import validate_rows,FAILED,PILOT


class Rows(unittest.TestCase):
    def setUp(self):
        self.old=[dict(case_id='old'+str(i),report='old-report'+str(i),reserved_games=50 if i==0 else 1,state='admitted',acquisition_sha256='old-sha')for i in range(156)]
        self.old[-1].update(case_id=FAILED,state='failed',report='failed-'+FAILED)
        self.new={PILOT:1}|{'new'+str(i):1 for i in range(44)}
        self.rows=copy.deepcopy(self.old)
    def positive(self,cid):return dict(case_id=cid,report='report-'+cid,reserved_games=1,state='admitted',acquisition_sha256='new-sha')
    def refuses(self):
        with self.assertRaises(ValueError):validate_rows(self.rows,self.old,self.new)
    def test_exact156rows205charges(self):self.assertEqual(validate_rows(self.rows,self.old,self.new),205)
    def test_all45newpositive_rows201and250(self):
        self.rows.extend(self.positive(cid)for cid in self.new)
        self.assertEqual((len(self.rows),validate_rows(self.rows,self.old,self.new)),(201,250))
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
    def load(self,name,path):
        spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
    def test_original_request_main_and_secondary_main_are_unchanged(self):
        original=self.load('derived_original_state_machine',ROOT/'tools/run-broader-player-case.py')
        self.wrapper=self.load('derived_request_wrapper',ROOT/'tools/run-broader-derived-source-case.py')
        secondary=self.wrapper.request_module();selected=secondary.request_module()
        for name in ['co_code','co_consts','co_names','co_filename']:
            self.assertEqual(getattr(selected.main.__code__,name),getattr(original.main.__code__,name))
        self.assertIs(selected.cumulative_bound,self.wrapper.bound);self.assertIs(selected.reviewed_predecessors,self.wrapper.predecessors)
        reader=selected.Reader(ROOT);self.assertTrue(reader._memoize);reader.close()

class BeforeOperation(unittest.TestCase):
    def setUp(self):
        sp=importlib.util.spec_from_file_location('derived_preoperation_wrapper',ROOT/'tools/run-broader-derived-source-case.py');self.wrapper=importlib.util.module_from_spec(sp);sp.loader.exec_module(self.wrapper)
        self.phase=dict(family='interaction',scene_runtime_variant={},derived_native_admission={},broader_cases=[dict(id='case',report='acquisition')]);owner=self
        class Reader:
            def __init__(self,*a,**k):pass
            def __enter__(self):return self
            def __exit__(self,*a):pass
            def digest(self,n):return n
            def json(self,n):return dict(success=True,sources_stable=True,acquisition={'record':'acquisition','sha256':'acquisition'})if n=='validation'else owner.phase
        p=patch.object(self.wrapper,'Reader',Reader);p.start();self.addCleanup(p.stop)
    def test_changed_derived_inputs_refuse_before_reservation(self):
        with patch.object(sys,'argv',['driver','acquire','--plan','phase','--case','case']),patch.object(self.wrapper,'checked_configuration',side_effect=ValueError('changed')),patch.object(self.wrapper,'request_module')as core:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called()
    def test_bad_actual_admission_refuses_before_core_admit(self):
        with patch.object(sys,'argv',['driver','admit','--plan','phase','--case','case','--validation','validation']),patch.object(self.wrapper,'checked_configuration'),patch.object(self.wrapper,'verify_admission',side_effect=ValueError('changed')),patch.object(self.wrapper,'request_module')as core,patch.object(self.wrapper,'write_report')as writer:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called();writer.assert_not_called()
    def test_missing_derived_declaration_refuses(self):
        del self.phase['derived_native_admission']
        with patch.object(sys,'argv',['driver','acquire','--plan','phase','--case','case']),patch.object(self.wrapper,'request_module')as core:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called()
    def test_other_variant_family_refuses(self):
        self.phase['family']='pause'
        with patch.object(sys,'argv',['driver','acquire','--plan','phase','--case','case']),patch.object(self.wrapper,'request_module')as core:
            with self.assertRaises(ValueError):self.wrapper.main()
            core.assert_not_called()
    def test_base_context_delegates_without_derived_selection(self):
        self.phase.pop('scene_runtime_variant');self.phase['family']='pause';core=SimpleNamespace(main=lambda:0)
        with patch.object(sys,'argv',['driver','acquire','--plan','phase','--case','case']),patch.object(self.wrapper,'request_module',return_value=core),patch.object(self.wrapper,'checked_configuration')as guard:
            self.assertEqual(self.wrapper.main(),0);guard.assert_not_called()

if __name__=='__main__':unittest.main()
