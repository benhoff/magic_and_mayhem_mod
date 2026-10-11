#!/usr/bin/env python3
"""Complete history admission, source provenance and temporal boundary guards."""
import copy
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_sequence as sequence
from drawing_corpus import ROOT,sha
from rendering_coverage import evidence_record

class SequenceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog=json.loads((ROOT/sequence.CATALOG).read_text())
        cls.register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        cls.raw,cls.assets,cls.checkpoints=sequence.admit(ROOT,cls.catalog,cls.register)

    def test_portable_entire_history(self):
        self.assertEqual(len(self.raw),70671130)
        self.assertEqual(len(self.checkpoints),1062)
        self.assertEqual(len(self.assets),16)
        self.assertEqual(self.catalog['boundary']['queues'],16)
        self.assertEqual(self.catalog['boundary']['sequence'],46647)
        self.assertEqual(sum(c['width']*c['height'] for c in self.checkpoints),self.catalog['comparison_pixels'])

    def test_native_source_history_excludes_observed_raster_results(self):
        import struct
        native=sequence.raster_inputs(self.raw)
        expected=bytearray(self.raw);at=64;removed=0
        while at<len(expected):
            size,_,kind=struct.unpack_from('<3I',expected,at)
            if kind==9:
                removed+=int(struct.unpack_from('<I',expected,at+84)[0]!=0)
                struct.pack_into('<I',expected,at+84,0)
            at+=size
        self.assertGreater(removed,0)
        self.assertEqual(native,bytes(expected))
        self.assertEqual(sha(self.raw),self.catalog['historical_stream']['sha256'])

    def test_retention_changes_refuse(self):
        for key,value in (('checkpoints',self.checkpoints[:-1]),('boundary',dict(self.catalog['boundary'],sequence=1))):
            catalog=copy.deepcopy(self.catalog);catalog[key]=value
            with self.assertRaisesRegex(ValueError,'retention identity'):
                sequence.admit(ROOT,catalog,self.register)

    def revised(self,catalog):
        def record(root,ev,builds):
            return catalog if ev['id']==sequence.RETENTION else evidence_record(root,ev,builds)
        return patch('rendering_coverage.evidence_record',side_effect=record)

    def test_even_new_retention_cannot_omit_checkpoints(self):
        catalog=copy.deepcopy(self.catalog);catalog['checkpoints'].pop(500)
        with self.revised(catalog),self.assertRaisesRegex(ValueError,'omits or duplicates'):
            sequence.admit(ROOT,catalog,self.register)

    def test_chunk_reordering_refuses(self):
        catalog=copy.deepcopy(self.catalog);catalog['input_chunks'].reverse()
        with self.revised(catalog),self.assertRaisesRegex(ValueError,'reconstruct'):
            sequence.admit(ROOT,catalog,self.register)

    def test_checkpoint_identity_refuses(self):
        catalog=copy.deepcopy(self.catalog);catalog['checkpoints'][0]['queue']+=1
        with self.revised(catalog),self.assertRaisesRegex(ValueError,'checkpoint identity'):
            sequence.admit(ROOT,catalog,self.register)

    def test_original_evidence_kind_required(self):
        register=copy.deepcopy(self.register)
        next(e for e in register['evidence'] if e['id']==self.catalog['evidence'])['kind']='native_integration'
        with self.assertRaisesRegex(ValueError,'independent original'):
            sequence.admit(ROOT,self.catalog,register)

    def test_artifacts_are_content_deduplicated(self):
        paths={c['oracle']['path'] for c in self.checkpoints}
        self.assertLess(len(paths),len(self.checkpoints))
        self.assertEqual(len({c['oracle']['raw_sha256'] for c in self.checkpoints}),789)
        self.assertLess(sum((ROOT/p).stat().st_size for p in paths),64*1024*1024)

    def test_lossless_delta_refusals(self):
        import gzip,tempfile
        from drawing_corpus import sha
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);first=b'\x01\x00\x02\x00';second=b'\x03\x00\x02\x00'
            delta=bytes(a^b for a,b in zip(first,second))
            def entry(name,data):
                packed=gzip.compress(data,mtime=0);(root/name).write_bytes(packed)
                return {'path':name,'encoding':'gzip','sha256':sha(packed),'raw_sha256':sha(data)}
            a={'ordinal':1,'canvas':2,'width':2,'height':1,'oracle':entry('a.gz',first)}
            b={'ordinal':2,'canvas':2,'width':2,'height':1,'oracle':dict(entry('b.gz',delta),
                encoding='xor-gzip',delta_sha256=sha(delta),raw_sha256=sha(second),base_checkpoint=1)}
            self.assertEqual([raw for _,raw in sequence.oracle_planes(root,[a,b])],[first,second])
            for change,message in (({'base_checkpoint':2},'base identity'),({'raw_sha256':'0'*64},'decoded oracle hash')):
                bad=copy.deepcopy(b);bad['oracle'].update(change)
                with self.assertRaisesRegex(ValueError,message):list(sequence.oracle_planes(root,[a,bad]))

    def test_composition_and_producer_continuity_do_not_claim_presentation(self):
        self.assertTrue(self.catalog['observations']['contiguous_producer_records'])
        self.assertFalse(self.catalog['observations']['contiguous_presented_frames'])
        self.assertFalse(self.catalog['observations']['actions_bound'])
        catalog=copy.deepcopy(self.catalog);catalog['observations']['boundary']='presented_frame'
        with self.revised(catalog),self.assertRaisesRegex(ValueError,'cannot claim presented'):
            sequence.admit(ROOT,catalog,self.register)

if __name__=='__main__':unittest.main()
