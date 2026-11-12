#!/usr/bin/env python3
"""Actual retained caller/authority/copy/output controls, without a drawing fixture."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import sys
import unittest
import numpy as np

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition,commands,History
spec=importlib.util.spec_from_file_location('copy_closeout',ROOT/'tools/close-rendering-opaque-cropped-copy.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class CopyCloseoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader=Reader(ROOT);cls.report=cls.reader.json(m.PREFIX+'closeout-analysis-01.json')
        cls.parent=cls.reader.json(cls.report['input_parent']['record']);cls.plan=cls.reader.json('docs/native-rendering-closure.json')
        cls.case=next(v for v in cls.report['executions'] if v['scenario']=='normal-storage')
        cls.a=cls.reader.json(cls.case['pair']['acquisition']['record']);cls.c=cls.reader.json(cls.case['pair']['comparison']['record'])
        streams=[artifact(cls.reader,cls.a,Path(v['path']).name) for v in cls.a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        cls.observed=acquisition(streams,cls.a['session_identity']);cls.header,cls.routes,cls.routing=m.routes(cls.reader,cls.a)
        cls.diagnostics={v['sequence']:v for v in (json.loads(line) for line in artifact(cls.reader,cls.a,'copies.jsonl').splitlines())}
        wanted={next(v['copy']['sequence'] for v in cls.case['proof']['witnesses'] if v['kind']==k) for k in ('whole','crop')}
        history=History(m.base.OriginalWorlds(cls.reader,cls.c));cls.inputs={}
        for op,seq,p in commands(cls.observed['commands']):
            if seq in wanted:
                w=struct.unpack('<10I',p);cls.inputs[seq]=(history.handles[w[0]]['pixels'].copy(),history.handles[w[1]]['pixels'].copy(),w,cls.diagnostics[seq])
            history.apply(op,seq,p)
            if cls.inputs.keys()==wanted:break

    @classmethod
    def tearDownClass(cls):cls.reader.close()

    def test_only_four_rows_close(self):
        self.assertEqual(m.close_rows(self.plan,self.parent,self.report['executions']),self.report['requirements'])
        self.assertEqual((self.report['passed_requirements'],self.report['pending_requirements']),(50,106))

    def test_all_46_parent_rows_and_both_decisions_preserved(self):
        for before,after in zip(self.parent['requirements'],self.report['requirements']):
            if before['id'] in m.LEAVES:self.assertEqual((before['state'],after['state']),('pending','passed'))
            else:self.assertEqual(before,after)
        self.assertEqual(self.parent['decisions'],self.report['decisions']);self.assertFalse(self.report['historical_failures_retired'])

    def test_missing_duplicate_domain_and_size_refuse(self):
        for kind in ('missing','duplicate','size','identity'):
            runs=copy.deepcopy(self.report['executions'])
            if kind=='missing':runs.pop()
            elif kind=='duplicate':runs[-1]=runs[0]
            elif kind=='size':runs[-1]['domain'][1]=[320,200]
            else:runs[-1]['identity']=runs[0]['identity']
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,runs)

    def test_whole_and_campaign_crops_have_distinct_generations(self):
        self.assertEqual(self.report['new_retained_viewport_samples'],36)
        for case in self.report['executions']:
            kinds=('whole','crop') if case['scenario']=='normal-storage' else ('whole',)
            m.check_witnesses(case['proof']['witnesses'],kinds)

    def test_cpu_and_gpu_source_authority_have_complete_destinations(self):
        for case in self.report['executions']:
            self.assertEqual(set(case['proof']['authority_witnesses']),{'1','2'})
            for key,c in case['proof']['authority_witnesses'].items():
                self.assertEqual(c['route'],int(key));self.assertGreater(c['changed_pixels'],0)
                self.assertEqual(c['source_gpu_authority'],bool(c['caller'][10]&4))

    def test_actual_full_and_cropped_complete_destinations_pass(self):
        for source,before,w,diagnostic in self.inputs.values():
            expected,changed=m.checked_destination(source,before,w,diagnostic)
            self.assertGreater(changed,0);self.assertEqual(m.words_hash(expected),diagnostic['actual_sha256'])

    def test_changed_source_before_or_actual_output_refuses(self):
        source,before,w,d=next(iter(self.inputs.values()))
        for kind in ('source','before','actual','expected'):
            s=source.copy();b=before.copy();diagnostic=copy.deepcopy(d)
            if kind=='source':s.flat[0]^=1
            elif kind=='before':b.flat[0]^=1
            else:diagnostic[kind+'_sha256']='0'*64
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.checked_destination(s,b,w,diagnostic)

    def test_copy_bounds_key_alias_and_shape_refuse(self):
        source,before,w,d=next(iter(self.inputs.values()))
        for kind in ('bounds','key','alias','shape'):
            words=list(w);diagnostic=copy.deepcopy(d)
            if kind=='bounds':words[2]=source.shape[1]+1
            elif kind=='key':words[8]=1
            elif kind=='alias':words[1]=words[0]
            else:diagnostic['source_height']+=1
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.checked_destination(source,before,tuple(words),diagnostic)

    def test_source_coordinate_and_native_sample_tampering_refuse(self):
        rows=copy.deepcopy(self.case['proof']['witnesses'])
        for kind in ('coordinate','pixel','lease','nochange'):
            bad=copy.deepcopy(rows)
            if kind=='coordinate':bad[0]['copy_source_pixel']=[-1,0]
            elif kind=='pixel':bad[0]['actual_rgba'][0]^=1
            elif kind=='lease':bad[0]['lease']=0
            else:bad[0]['copy']['changed_pixels']=0
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.check_witnesses(bad)

    def test_wrong_whole_crop_kind_and_missing_generation_refuse(self):
        for kind in ('kind','generation','missing'):
            bad=copy.deepcopy(self.case['proof']['witnesses'])
            if kind=='kind':bad[0]['copy']['words'][2]+=1
            elif kind=='generation':bad[1]['generation']=bad[0]['generation']
            else:bad.pop()
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.check_witnesses(bad)

    def test_closed_original_callers_bind_every_copy_once(self):
        bound,refused=m.command_join(self.observed['commands'],self.routes)
        expected=sum(op==3 for op,seq,p in commands(self.observed['commands']))
        self.assertEqual(len(bound),expected);self.assertEqual(refused,self.case['proof']['refused_calls'])

    def test_missing_duplicate_and_changed_caller_interval_refuse(self):
        at=next(i for i,row in enumerate(self.routes) if row[40])
        for kind in ('missing','duplicate','handles','key'):
            rows=list(self.routes)
            if kind=='missing':rows.pop(at)
            elif kind=='duplicate':rows.append(rows[at])
            else:
                row=list(rows[at])
                if kind=='handles':row[42]+=1
                else:row[12]^=1 if row[11] else 0x8000
                rows[at]=tuple(row)
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.command_join(self.observed['commands'],rows)

    def test_refused_caller_cannot_emit_work(self):
        rows=list(self.routes);at=next(i for i,row in enumerate(rows) if row[40]);bad=list(rows[at]);bad[13]=0x88760096;rows[at]=tuple(bad)
        with self.assertRaises(ValueError):m.command_join(self.observed['commands'],rows)

    def test_known_refused_calls_and_absent_routes_keep_no_positive_credit(self):
        failures=[]
        for case in self.report['executions']:
            routing=case['routing'];self.assertEqual(routing['branches']['self_alias']['calls'],0);self.assertEqual(routing['branches']['attached_clipper']['calls'],0)
            self.assertEqual(routing['branches']['keyed']['prepared_gpu_calls'],0)
            failures+=case['proof']['refused_calls']
        self.assertEqual(len(failures),4)
        self.assertTrue(all(r[13]==0x88760096 and r[40:44]==[0,0,0,0] for r in failures))
        self.assertEqual(m.LEAVES,{'operation.copy-opaque','operation.image-cropping'})

    def test_current_native_sources_and_no_new_original_execution(self):
        self.assertTrue(all(self.reader.digest(n)==h for n,h in self.report['sources'].items()))
        self.assertEqual(self.report['original_game_runs'],0);self.assertEqual(self.report['new_original_reference_runs'],0)
        self.assertFalse(self.report['new_native_code']);self.assertFalse(self.report['new_game_fixture'])


if __name__=='__main__':unittest.main()
