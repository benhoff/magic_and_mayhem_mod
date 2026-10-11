#!/usr/bin/env python3
"""CPU target admission cannot become graphics or core-absent evidence."""
import importlib.util
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_cohorts import call_role
ROOT=Path(__file__).resolve().parents[1]


def module(path):
    spec=importlib.util.spec_from_file_location('cpu_test_'+path.replace('-','_'),ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


class CpuCohortTests(unittest.TestCase):
    def test_historical_graphics_and_nonobserver_defaults_remain_explicit(self):
        self.assertEqual(call_role({}),'none');self.assertEqual(call_role({'api_observation_enabled':True}),'graphics_api');self.assertEqual(call_role({'api_observation_enabled':True,'call_census_enabled':False,'api_only':True}),'graphics_api')

    def test_cpu_role_requires_admitted_original_active_core(self):
        self.assertEqual(call_role({'api_observation_enabled':True,'call_boundary_role':'cpu_backend_dispatch'}),'cpu_backend_dispatch')
        for metadata in [{'call_boundary_role':'cpu_backend_dispatch'},{'api_observation_enabled':True,'call_census_enabled':False,'api_only':True,'call_boundary_role':'cpu_backend_dispatch'},{'api_observation_enabled':True,'call_census_enabled':False,'call_boundary_role':'cpu_backend_dispatch'}]:
            with self.assertRaises(ValueError):call_role(metadata)

    def test_role_flags_truthy_alias_and_unknown_labels_refuse(self):
        for role in [True,False,None,1,'cpu','graphics','unknown',[]]:
            with self.assertRaises(ValueError):call_role({'api_observation_enabled':True,'call_boundary_role':role})
        for metadata in [{'api_observation_enabled':True,'call_boundary_role':'none'},{'call_boundary_role':'graphics_api'},{'api_observation_enabled':1,'call_boundary_role':'cpu_backend_dispatch'}]:
            with self.assertRaises(ValueError):call_role(metadata)

    def test_cpu_builder_refuses_other_cohorts_before_artifact_access(self):
        builder=module('tools/build-drawing-cpu-census.py')
        for selected in ['api_only','core','menu_control','unknown',None]:
            with self.assertRaises(ValueError):builder.build(Path('/missing-cpu-fixture.exe'),selected)

    def test_cpu_staging_refuses_mixed_and_core_absent_modes_before_artifact_access(self):
        prepare=module('tools/prepare-drawing-census.py')
        for flags in [{'api':True},{'api_only':True},{'menu_only':True},{'api':True,'api_only':True}]:
            with self.assertRaises(ValueError):prepare.prepare(Path('/missing-cpu-installation'),cpu=True,**flags)


if __name__=='__main__':unittest.main()
