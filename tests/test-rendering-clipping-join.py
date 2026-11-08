"""Actual four-case clipping join and exact parent-row preservation."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('clip_join',ROOT/'tools/close-rendering-clipped-operations.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class Override:
    def __init__(self,reader,name,body):self.reader,self.name,self.body=reader,name,body
    def json(self,name):return self.body if name==self.name else self.reader.json(name)
    def digest(self,name):return self.reader.digest(name)


class ActualJoin(unittest.TestCase):
    def setUp(self):
        self.reader=Reader(ROOT);self.addCleanup(self.reader.close)
        self.parent=self.reader.json('research/runtime/rendering-enabled-intros-results-20261107/integrated-acceptance-03.json')
        self.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        self.names=[m.PREFIX+n for n in ('default-800-07-comparison-05.json',
            'default-640-11-comparison-02.json','software-800-12-comparison-02.json',
            'software-640-14-comparison-01.json')]

    def test_exactly_two_rows_and_existing_gate_attachments_are_preserved(self):
        result=m.join(self.reader,self.parent,self.names,self.plan)
        self.assertEqual(result['passed_requirements'],self.parent['passed_requirements']+2)
        self.assertEqual(result['decisions'],self.parent['decisions'])
        self.assertEqual(len(result['requirements']),156)
        for old,new in zip(self.parent['requirements'],result['requirements']):
            if old['id']==m.LEAF:self.assertEqual(new['state'],'passed')
            elif old['id'] in ('gate.identity','gate.normal-lifetime'):
                self.assertEqual(new['admitted_execution_evidence'][:len(old['admitted_execution_evidence'])],old['admitted_execution_evidence'])
                self.assertEqual(len(new['admitted_execution_evidence']),len(old['admitted_execution_evidence'])+2)
                self.assertEqual(old['state'],new['state'])
            else:self.assertEqual(new,old)

    def test_missing_or_repeated_domains_refuse(self):
        for names in (self.names[:-1],self.names[:3]+[self.names[0]]):
            with self.assertRaises(ValueError):m.join(self.reader,self.parent,names,self.plan)

    def test_incomplete_component_and_actual_proof_flags_refuse(self):
        name=self.names[0];original=self.reader.json(name)
        for flag in ('success','analysis_completed','analysis_sources_stable','complete_output_verified','native_clip_regions_verified'):
            body=copy.deepcopy(original);body[flag]=False
            with self.subTest(flag=flag),self.assertRaises(ValueError):m.join(Override(self.reader,name,body),self.parent,self.names,self.plan)
        for flag in ('primary_identity_verified','normal_lifetime_verified','actual_clip_region_verified','caller_clip_plan_verified'):
            body=copy.deepcopy(original);body['case']['proof'][flag]=False
            with self.subTest(flag=flag),self.assertRaises(ValueError):m.join(Override(self.reader,name,body),self.parent,self.names,self.plan)

    def test_changed_component_source_and_vacuous_planes_refuse(self):
        name=self.names[0];original=self.reader.json(name)
        body=copy.deepcopy(original);body['analysis_sources'][next(iter(body['analysis_sources']))]='0'*64
        with self.assertRaises(ValueError):m.join(Override(self.reader,name,body),self.parent,self.names,self.plan)
        for flag,value in (('changed_words',0),('preserved_exterior_words',0),('actual_clip_region_snapshot_retained',False)):
            body=copy.deepcopy(original);body['case']['raw_pixel_witnesses'][0][flag]=value
            with self.subTest(flag=flag),self.assertRaises(ValueError):m.join(Override(self.reader,name,body),self.parent,self.names,self.plan)

    def test_other_threads_and_global_diagnostics_never_get_output_credit(self):
        result=m.join(self.reader,self.parent,self.names,self.plan)
        self.assertFalse(result['whole_copy_diagnostic_accepted'])
        for name in self.names:
            c=self.reader.json(name)['case']
            self.assertFalse(c['region_scope']['global_census_accepted'])
            self.assertTrue(c['region_scope']['owner_records_complete'])
            self.assertTrue(all(w['caller'][3]==c['region_scope']['owner_thread'] for w in c['raw_pixel_witnesses']))
            self.assertEqual({w['caller'][15] for w in c['raw_pixel_witnesses']},{1,2})
            self.assertTrue(all(w['imported_world'] for w in c['raw_pixel_witnesses'] if w['caller'][15]==2))


if __name__=='__main__':unittest.main()
