#!/usr/bin/env python3
"""Guards using retained natural sprite/effect executions; no drawing fixture."""
import copy
import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from resident_capture import records
spec = importlib.util.spec_from_file_location('blend_direct',ROOT/'tools/close-rendering-blend-direct.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


class BlendDirectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.report = cls.reader.json(m.PREFIX+'closeout-analysis-02.json')
        cls.parent = cls.reader.json(cls.report['input_parent']['record'])
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        cls.case = next(v for v in cls.parent['executions'] if v['scenario'] == 'normal-storage')
        a = cls.reader.json(cls.case['pair']['acquisition']['record'])
        c = cls.reader.json(cls.case['pair']['comparison']['record'])
        cls.worlds = m.OriginalWorlds(cls.reader,c)
        with tempfile.TemporaryDirectory(dir='/tmp') as directory:
            path = Path(directory)/'source.bin'; path.write_bytes(artifact(cls.reader,a,'worlds.bin'))
            for g,chunk,raw,request in records(path):
                if g == 2 and not chunk:
                    cls.rows = m.base.module('inspect-canvas-producers').decode(raw,complete=False)[1]
                    cls.request = request; break
        cls.entry,cls.rasters = m.base.checked_request(cls.rows,cls.request)
        cls.selected = m.candidates(cls.rows,cls.rasters,cls.case['domain'][1],cls.worlds[2])

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def test_exact_four_rows_only(self):
        self.assertEqual(m.close_rows(self.plan,self.parent,self.report['executions']),self.report['requirements'])
        self.assertEqual((self.report['passed_requirements'],self.report['pending_requirements']),(46,110))
        self.assertEqual(self.report['newly_passed_requirements'],4)

    def test_prior_rows_and_decisions_preserved(self):
        for before,after in zip(self.parent['requirements'],self.report['requirements']):
            if before['id'] in m.LEAVES: self.assertEqual((before['state'],after['state']),('pending','passed'))
            else: self.assertEqual(before,after)
        self.assertEqual(self.parent['decisions'],self.report['decisions'])
        self.assertFalse(self.report['historical_failures_retired'])

    def test_missing_duplicate_size_identity_context_refuses(self):
        for kind in ('missing','duplicate','size','identity','context'):
            runs = copy.deepcopy(self.report['executions'])
            if kind == 'missing': runs.pop()
            elif kind == 'duplicate': runs[-1] = runs[0]
            elif kind == 'size': runs[-1]['domain'][1] = [320,200]
            elif kind == 'identity': runs[-1]['identity'] = runs[0]['identity']
            else: runs[-1]['scenario'] = 'normal-quick'
            with self.subTest(kind=kind),self.assertRaises(ValueError): m.close_rows(self.plan,self.parent,runs)

    def test_all_modes_three_generations_in_each_domain(self):
        self.assertEqual(self.report['new_retained_pixel_samples'],48)
        for case in self.report['executions']: m.check_samples(case['samples'])

    def test_missing_mode_generation_and_duplicate_refuse(self):
        for kind in ('mode','generation','duplicate'):
            rows = copy.deepcopy(self.report['executions'][0]['samples'])
            if kind == 'mode': rows = [v for v in rows if v['kind'] != 'blend-1']
            elif kind == 'generation': rows = [v for v in rows if v['generation'] != 4]
            else: rows[-1] = rows[0]
            with self.subTest(kind=kind),self.assertRaises(ValueError): m.check_samples(rows)

    def test_native_pixel_lineage_and_lease_changes_refuse(self):
        for kind in ('pixel','lineage','lease','source'):
            rows = copy.deepcopy(self.report['executions'][0]['samples'])
            if kind == 'pixel': rows[0]['actual_rgba'][0] ^= 1
            elif kind == 'lineage': rows[0]['lineage_world_pixel'] ^= 1
            elif kind == 'source': rows[0]['source']['descriptor'][0] ^= 1
            else: rows[0]['lease'] = 0
            with self.subTest(kind=kind),self.assertRaises(ValueError): m.check_samples(rows)

    def test_opaque_equal_blend_gets_no_history_credit(self):
        rows = copy.deepcopy(self.report['executions'][0]['samples'])
        v = next(v for v in rows if v['kind'] == 'blend-1'); v['source']['source']['word'] = v['source']['original_word']
        with self.assertRaises(ValueError): m.check_samples(rows)

    def test_real_empty_frames_are_noops(self):
        quick = next(v for v in self.parent['executions'] if v['scenario'] == 'normal-quick')
        acquired = self.reader.json(quick['pair']['acquisition']['record'])
        with tempfile.TemporaryDirectory(dir='/tmp') as directory:
            path = Path(directory)/'quick-source.bin'; path.write_bytes(artifact(self.reader,acquired,'worlds.bin'))
            for generation,chunk,raw,request in records(path):
                if generation == 2 and not chunk:
                    rows = m.base.module('inspect-canvas-producers').decode(raw,complete=False)[1]
                    rasters = m.base.checked_request(rows,request)[1]; break
        empty = [(f,b) for f,b,d in rasters if struct.unpack_from('<2I',b,4) == (0,0)]
        self.assertTrue(empty)
        for f,b in empty: self.assertEqual(m.spans(f,b),[])

    def test_last_writer_and_owned_source_lookups(self):
        writers = m.last_writers(self.rows,self.rasters,self.case['domain'][1])
        self.assertEqual(set(self.selected),m.KINDS)
        for kind,rows in self.selected.items():
            for v in rows:
                x,y = v['destination']; self.assertEqual(int(writers[y,x]),v['sequence'])
                f,b,d = next(r for r in self.rasters if r[0][1] == v['sequence'])
                self.assertEqual(m.source_word(f,b,x,y),v['source'])
                self.assertEqual(int(self.worlds[2][y,x]),v['original_word'])

    def test_direct_word_stride_palette_and_source_bounds(self):
        v = self.selected['direct'][0]; f,b,d = next(r for r in self.rasters if r[0][1] == v['sequence']); x,y = v['destination']
        self.assertEqual(m.source_word(f,b,x,y)['word'],v['original_word'])
        with self.assertRaises(ValueError): m.source_word(f,b,-100000,y)
        bad = list(f); bad[20] = 512
        with self.assertRaises(ValueError): m.source_word(bad,b,x,y)
        bad = bytearray(b); struct.pack_into('<I',bad,44+v['source']['source'][1]*8,len(b))
        with self.assertRaises(ValueError): m.source_word(f,bytes(bad),x,y)

    def test_unsupported_extra_writer_fails_closed(self):
        f = list(self.rasters[-1][0]); f[1] += 1; f[2] = 26
        with self.assertRaises(ValueError): m.last_writers(self.rows+[(tuple(f),b'')],self.rasters,self.case['domain'][1])

    def test_changed_owned_frame_request_refuses(self):
        rows = list(self.rows); i = next(i for i,(f,b) in enumerate(rows) if f[1] == self.selected['direct'][0]['sequence'])
        f,b = rows[i]; b = bytearray(b); b[-1] ^= 1; rows[i] = f,bytes(b)
        with self.assertRaises(ValueError): m.base.checked_request(rows,self.request)

    def test_independent_history_and_no_new_game_or_fixture(self):
        for case in self.report['executions']:
            c = self.reader.json(case['pair']['comparison']['record']); ref = c['original_reference']
            self.assertTrue(ref['continuous_from_owned_initial_history'])
            self.assertFalse(ref['native_player_pixels_read']); self.assertFalse(ref['original_destination_capture_read'])
        self.assertEqual(self.report['original_game_runs'],0); self.assertEqual(self.report['new_original_reference_runs'],0)
        self.assertFalse(self.report['new_native_code']); self.assertFalse(self.report['new_game_fixture'])

    def test_complete_current_source_closure(self):
        self.assertTrue(all(self.reader.digest(n) == h for n,h in self.report['sources'].items()))


if __name__ == '__main__': unittest.main()
