"""Actual retained owner-thread clip inputs and adverse caller/plan joins."""
import copy
import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from native_copy_routes import decode
from native_copy_clip_regions_scoped import decode_owner
import native_copy_clip_regions as wire
from rendering_capture import capture
from rendering_pilot import commands

spec=importlib.util.spec_from_file_location('clip_check',ROOT/'tools/check-rendering-clipping.py')
checker=importlib.util.module_from_spec(spec);spec.loader.exec_module(checker)


class ActualBindings(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r, tempfile.TemporaryDirectory(dir='/tmp') as td:
            a=r.json(checker.PREFIX+'default-800-07-acquisition.json')
            p=Path(td)/'routes.bin';p.write_bytes(artifact(r,a,'copy-routes-v2.bin'))
            cls.header,cls.callers=decode(p)
            cls.raw=artifact(r,a,'native-copy-clip-regions-v1.bin')
            states=[v for v in a['states'] if 'thread' in v and 'event' not in v]
            cls.owner=states[0]['thread']
            cls.observations,cls.scope=decode_owner(cls.raw,cls.callers,cls.owner)
            cls.rows=[v for v in cls.callers if v[3]==cls.owner]
            stream=next(v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands'))
            raw=b''.join(b for h,b in capture(r.read_bytes(stream['record'])) if h['kind']=='fragment')
            cls.copies={s:dict(words=struct.unpack('<10I',p)) for op,s,p in commands(raw) if op==3}

    def test_actual_owner_is_complete_without_global_census_credit(self):
        result=checker.bind_regions(self.observations,self.rows,self.copies)
        self.assertGreater(len(result),100)
        self.assertEqual({v['observation']['phase'] for v in result.values()},{1,2})
        self.assertTrue(self.scope['owner_records_complete'])
        self.assertFalse(self.scope['global_census_accepted'])
        self.assertGreater(self.scope['other_thread_refusals'],0)
        with self.assertRaises(ValueError):wire.decode(self.raw)

    def test_owner_reader_refuses_other_error_bits_counts_threads_and_tail(self):
        for word,value in ((6,0),(7,self.scope['other_thread_refusals']+1),
                           (9,0),(9,5),(9,12),(10,0),(11,self.owner+1),(12,1)):
            raw=bytearray(self.raw);struct.pack_into('<I',raw,word*4,value)
            with self.subTest(word=word,value=value),self.assertRaises(ValueError):decode_owner(raw,self.callers,self.owner)
        for offset in (128+8,len(self.raw)-1):
            raw=bytearray(self.raw);raw[offset]^=1
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode_owner(raw,self.callers,self.owner)

    def test_original_caller_identities_and_regions_cannot_be_replaced(self):
        first=next(i for i,o in enumerate(self.observations) if o['phase'] in (1,2))
        for key in ('thread','target_alias','source_alias','target_canonical','source_canonical','flags','result','plan_pieces'):
            rows=copy.deepcopy(self.observations);rows[first][key]+=1
            with self.subTest(key=key),self.assertRaises(ValueError):checker.bind_regions(rows,self.rows,self.copies)
        rows=copy.deepcopy(self.observations);rows[first]['regions'][0][2]-=1
        with self.assertRaises(ValueError):checker.bind_regions(rows,self.rows,self.copies)

    def test_actual_copy_words_and_cpu_snapshot_are_bound(self):
        result=checker.bind_regions(self.observations,self.rows,self.copies)
        caller=next(r for r in self.rows if r[0] in result and r[40])
        copies=copy.deepcopy(self.copies);w=list(copies[caller[40]]['words']);w[6]+=1
        copies[caller[40]]['words']=tuple(w)
        with self.assertRaises(ValueError):checker.bind_regions(self.observations,self.rows,copies)
        rows=copy.deepcopy(self.observations)
        i=next(i for i,v in enumerate(rows) if v['phase']==1)
        j=next(j for j in range(i+1,len(rows)) if rows[j]['phase']==4)
        rows[j]['clipper']+=1
        with self.assertRaises(ValueError):checker.bind_regions(rows,self.rows,self.copies)

    def test_single_domain_cannot_promote_a_row(self):
        proof={k:True for k in ('primary_identity_verified','normal_lifetime_verified',
             'positive_complete_pixel_witnesses','actual_clip_region_verified',
             'required_domains_verified','caller_clip_plan_verified')}
        self.assertTrue(checker.row_ready(proof))
        for key in proof:
            with self.subTest(key=key):self.assertFalse(checker.row_ready(dict(proof,**{key:False})))


if __name__=='__main__':unittest.main()
