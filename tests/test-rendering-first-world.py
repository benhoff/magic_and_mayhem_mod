#!/usr/bin/env python3
"""Adverse first-entry checks against actual retained source and native output."""
import copy
import importlib.util
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition
from resident_capture import records
spec=importlib.util.spec_from_file_location('first_world',ROOT/'tools/close-rendering-first-world.py')
m=importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class FirstWorldTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.r=Reader(ROOT)
        cls.q=cls.r.json(os.environ.get('MNM_FIRST_WORLD_RESULT',m.PREFIX+'closeout-analysis-05.json'))
        assert cls.q['success']
        cls.parent=cls.r.json(m.PARENT)
        cls.plan=cls.r.json('docs/native-rendering-closure.json')
        cls.case=cls.q['cases'][0]
        cls.a,cls.c=[m.checked(cls.r,cls.case['pair'][k]) for k in ('acquisition','comparison')]
        cls.observed=acquisition([artifact(cls.r,cls.a,Path(v['path']).name) for v in cls.a['artifacts']
            if '/capture/' in v['path'] and v['path'].endswith('.bin')],cls.a['session_identity'])
        cls.worlds=m.originals(cls.r,cls.c)
        with tempfile.TemporaryDirectory(dir='/tmp') as d:
            p=Path(d)/'worlds.bin';p.write_bytes(artifact(cls.r,cls.a,'worlds.bin'))
            for g,ch,raw,request in records(p):
                if not ch:
                    cls.rows=m.base.module('inspect-canvas-producers').decode(raw,False)[1]
                    cls.request=request
                    break
        identity=next(c for c in cls.parent['cases'] if c['reference']==cls.case['pair']['acquisition'])
        cls.original=m.object_bytes(cls.r,identity['installation']['original_executable_sha256'])
        cls.staged=m.object_bytes(cls.r,identity['staged_executable']['sha256'])

    @classmethod
    def tearDownClass(cls):cls.r.close()

    def test_exact_four_rows_preserve_all_parents_and_decisions(self):
        rows=m.close_rows(self.plan,self.parent,self.q['cases'])
        self.assertEqual(rows,self.q['requirements'])
        self.assertEqual(sum(a!=b for a,b in zip(rows,self.parent['requirements'])),4)
        self.assertEqual(self.q['decisions'],self.parent['decisions'])
        self.assertFalse(self.q['historical_failures_retired'])

    def test_every_required_domain_has_real_preparation_and_output(self):
        for c in self.q['cases']:
            self.assertTrue(c['caller']['verified']);self.assertTrue(c['output']['verified'])
            self.assertEqual(c['output']['generation'],1)
            self.assertEqual(sum(v['calls'] for v in c['caller']['callers']),c['caller']['rasters'])
            self.assertFalse(c['caller']['raw_row_snapshot_captured'])
            self.assertFalse(c['caller']['whole_original_preparation_recovered'])

    def test_missing_duplicate_size_identity_proof_and_admission_refuse(self):
        for kind in ('missing','duplicate','size','identity','caller','output','identity-admission','normal-admission'):
            cases=copy.deepcopy(self.q['cases'])
            if kind=='missing':cases.pop()
            elif kind=='duplicate':cases[-1]=cases[0]
            elif kind=='size':cases[-1]['domain'][1]=[123,456]
            elif kind=='identity':cases[-1]['identity']=cases[0]['identity']
            elif kind in ('caller','output'):cases[0][kind]['verified']=False
            else:cases[0]['admitted_identity' if kind=='identity-admission' else 'admitted_normal_lifetime']=False
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)

    def test_actual_first_queue_caller_contract(self):
        self.assertEqual(m.caller_state(self.rows,self.request,self.original,self.staged),self.case['caller'])

    def test_parent_count_cannot_hide_changed_rows_or_decisions(self):
        for kind in ('count','row','decision'):
            p=copy.deepcopy(self.parent)
            if kind=='count':p['passed_requirements']+=1
            elif kind=='row':next(v for v in p['requirements'] if v['state']=='passed')['state']='pending'
            else:p['decisions']['supported-player']='passed'
            with self.subTest(kind=kind),self.assertRaises(ValueError):m.close_rows(self.plan,p,self.q['cases'])

    def test_output_contract_cannot_be_relabelled_at_closure(self):
        for key,value in [('generation',2),('frame_size',[123,456]),('mismatching_pixels',1),('pixels',0),('expected_sha256','0'*64)]:
            cases=copy.deepcopy(self.q['cases']);cases[0]['output'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)

    def test_historical_native_source_phases_remain_visible(self):
        self.assertFalse(self.q['current_native_engine_execution'])
        self.assertEqual(self.q['original_game_runs'],0)
        self.assertTrue(self.q['analysis_sources_stable'])
        self.assertTrue(set(self.parent['sources'])<=set(self.q['sources']))
        self.assertTrue(all(self.q['sources'][n]==h for n,h in self.parent['sources'].items()))

    def test_unreviewed_caller_cannot_borrow_an_admitted_backend(self):
        rows=list(self.rows);entry=next(f for f,b in rows if f[2]==11)
        i=next(i for i,(f,b) in enumerate(rows) if f[2]==9 and f[1]>entry[1])
        f,b=rows[i];f=list(f);f[22]=0x500000;rows[i]=(tuple(f),b)
        # Rebind the wire checksum, so rejection must be semantic.
        request=bytearray(self.request)
        struct.pack_into('<I',request,76,m.base.source_hash(f,b))
        struct.pack_into('<I',request,48,m.base.fnv(request[64:]))
        with self.assertRaisesRegex(ValueError,'Unreviewed first-World caller'):
            m.caller_state(rows,bytes(request),self.original,self.staged)

    def test_descriptor_order_source_hash_entry_and_ax_refuse(self):
        for offset in (64,68,72,76):
            request=bytearray(self.request);request[offset]^=1
            struct.pack_into('<I',request,48,m.base.fnv(request[64:]))
            with self.subTest(offset=offset),self.assertRaises(ValueError):
                m.caller_state(self.rows,bytes(request),self.original,self.staged)

    def test_additional_writer_preparation_requires_its_own_contract(self):
        rows=list(self.rows);f=list(rows[-1][0]);f[2]=14;rows.append((tuple(f),bytes(20)))
        with self.assertRaises(ValueError):m.caller_state(rows,self.request,self.original,self.staged)

    def test_changed_staged_consumer_and_caller_bytes_refuse(self):
        for address in (0x5002a0,0x500aab,0x57e985):
            raw=bytearray(self.staged)
            signature=m.pe_bytes(raw,address,10)
            pos=raw.find(signature)
            # Locate by unique entire signature; original consumer entry can
            # occur more than once, so independently check the mapped slice.
            if raw.count(signature)!=1:
                pos=next(i for i in range(len(raw)) if raw[i:i+32]==m.pe_bytes(raw,address,32))
            raw[pos]^=1
            with self.subTest(address=address),self.assertRaises(ValueError):
                m.caller_state(self.rows,self.request,self.original,bytes(raw))

    def test_actual_complete_first_viewport_recomparison(self):
        self.assertEqual(m.first_output(self.a,self.c,self.observed,self.worlds,self.case['domain'][1]),self.case['output'])

    def test_missing_first_swap_never_selects_a_later_world(self):
        first=self.case['output']['paint']
        observed=dict(self.observed,swaps=[p for p in self.observed['swaps'] if p!=first])
        with self.assertRaises(ValueError):m.first_output(self.a,self.c,observed,self.worlds,self.case['domain'][1])

    def test_dependent_initial_history_refuses(self):
        for key,value in [('continuous_from_owned_initial_history',False),('native_player_pixels_read',True),('original_destination_capture_read',True)]:
            c=copy.deepcopy(self.c);c['original_reference'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):m.first_output(self.a,c,self.observed,self.worlds,self.case['domain'][1])

    def test_changed_actual_pixel_or_output_summary_refuses(self):
        ordinal=self.case['output']['paint'];paint,body=self.observed['paints'][ordinal]
        actual=bytearray(body() if callable(body) else body);actual[0]^=1
        observed=dict(self.observed,paints=dict(self.observed['paints']))
        observed['paints'][ordinal]=(paint,bytes(actual))
        with self.assertRaises(ValueError):m.first_output(self.a,self.c,observed,self.worlds,self.case['domain'][1])
        c=copy.deepcopy(self.c);next(v for v in c['comparison']['comparisons'] if v['paint']==ordinal)['actual_sha256']='0'*64
        with self.assertRaises(ValueError):m.first_output(self.a,c,self.observed,self.worlds,self.case['domain'][1])

    def test_generation_owner_size_and_complete_gpu_read_are_required(self):
        ordinal=self.case['output']['paint']
        for key,value in [('world','2'),('presentation_owner','worker'),('gpu_read_complete',False),('frame_width',999)]:
            observed=dict(self.observed,paints=dict(self.observed['paints']))
            paint,body=observed['paints'][ordinal];observed['paints'][ordinal]=(dict(paint,**{key:value}),body)
            with self.subTest(key=key),self.assertRaises(ValueError):m.first_output(self.a,self.c,observed,self.worlds,self.case['domain'][1])


if __name__=='__main__':unittest.main()
