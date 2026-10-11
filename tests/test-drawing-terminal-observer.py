#!/usr/bin/env python3
"""Pinned terminal prefix composition and pre-staging CPU admission guards."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


builder=load('terminal_builder_test','tools/build-drawing-terminal-census.py')
staging=load('terminal_staging_test','tools/prepare-drawing-terminal-census.py')


class TerminalObserverTests(unittest.TestCase):
    def setUp(self):
        self.proof=ROOT/builder.PROOF;proof=json.loads(self.proof.read_text());self.raw=bytearray();self.offsets={};self.code={}
        for entry in builder.TARGETS:
            self.offsets[entry]=len(self.raw);self.raw.extend(bytes.fromhex(proof['original_entry_anchors'][hex(entry)]))
        self.offset=lambda raw,address,length:self.offsets[address]
        self.base=[{'tag':0,'entry':0x596cb8,'length':5}]

    def rows(self,raw=None,base=None,code=None,proof=None):
        return builder.terminal_rows(self.raw if raw is None else raw,self.base if base is None else base,
            self.code if code is None else code,self.proof if proof is None else proof,self.offset)

    def test_exact_five_mov_prefixes_keep_default_rows_and_ret0(self):
        rows=self.rows();self.assertEqual(rows[0],self.base[0]);self.assertEqual([r['entry'] for r in rows[1:]],list(builder.TARGETS));self.assertEqual([r['tag'] for r in rows],list(range(6)))
        self.assertEqual([r['length'] for r in rows[1:]],[5,5,6,6,5]);self.assertTrue(all(r['pop']==0 and r['site'] is False and r['relocations']==[] for r in rows[1:]))

    def test_any_changed_full_wrapper_body_refuses_even_unchanged_prefix(self):
        for entry in builder.TARGETS:
            raw=self.raw.copy();at=self.offsets[entry];raw[at+8]^=1
            with self.assertRaises(ValueError):self.rows(raw=raw)

    def test_overlap_with_existing_core_hook_refuses(self):
        for entry in builder.TARGETS:
            with self.assertRaises(ValueError):self.rows(base=[{'tag':0,'entry':entry+1,'length':5}])

    def test_incoming_jump_into_prefix_interior_refuses(self):
        for entry in builder.TARGETS:
            code={0x600100:{'address':0x600100,'mnemonic':'jmp','operand':hex(entry+1),'bytes':'eb00'}}
            with self.assertRaises(ValueError):self.rows(code=code)

    def test_incoming_jump_to_untouched_continuation_is_admitted(self):
        entry=builder.TARGETS[0];code={0x600100:{'address':0x600100,'mnemonic':'jmp','operand':hex(entry+5),'bytes':'eb00'}}
        self.assertEqual(len(self.rows(code=code)),6)

    def test_capacity_cannot_overflow_native_hook_table(self):
        base=[{'tag':i,'entry':0x600000+i*0x100,'length':5} for i in range(252)]
        with self.assertRaises(ValueError):self.rows(base=base)

    def test_unknown_original_proof_refuses(self):
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'proof.json';p.write_text('{}')
            with self.assertRaises(ValueError):self.rows(proof=p)

    def test_invalid_review_hash_refuses_before_any_source_game_access(self):
        for digest in ['',None,'0'*63,'z'*64]:
            with self.assertRaises(ValueError):staging.prepare(Path('/absent-game'),Path('/absent-plan'),digest)

    def test_outside_repo_plan_refuses_before_file_access(self):
        with self.assertRaises(ValueError):staging.approved_plan(Path('/absent-plan'),'0'*64)

    def test_stale_plan_hash_refuses(self):
        with tempfile.TemporaryDirectory(dir=ROOT/'working/tests') as directory:
            p=Path(directory)/'plan.json';p.write_text('{}')
            with self.assertRaises(ValueError):staging.approved_plan(p,'0'*64)

    def test_wrong_plan_core_role_count_and_non_cpu_namespace_refuse(self):
        record={'terminal_flow_observation':True,'cohort':'core_api','core_hook_count':88,
            'hooks':[{'tag':i,'site':0x400100+i*16,'target_mode':{'kind':'absolute_memory','address':0x6903dc}} for i in range(128)],'refused':[]}
        with tempfile.TemporaryDirectory(dir=ROOT/'working/tests') as directory:
            p=Path(directory)/'plan.json'
            def read(r):
                raw=json.dumps(r).encode();p.write_bytes(raw);return staging.approved_plan(p,hashlib.sha256(raw).hexdigest())
            self.assertEqual(len(read(record)[1]['hooks']),128)
            for field,value in [('terminal_flow_observation',False),('cohort','api_only'),('core_hook_count',83),('refused',[{}])]:
                r=copy.deepcopy(record);r[field]=value
                with self.assertRaises(ValueError):read(r)
            r=copy.deepcopy(record);r['hooks'][0]['target_mode']['address']=0x5c5308
            with self.assertRaises(ValueError):read(r)
            r=copy.deepcopy(record);r['hooks'][0]['tag']=1
            with self.assertRaises(ValueError):read(r)


if __name__=='__main__':unittest.main()
