#!/usr/bin/env python3
"""Actual parser, source binding and recursive module controls; no game launch."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_animation_target import load_index,asset_offsets,POLICY
from broader_picking_animation_launch import binding_class,original_targets
spec=importlib.util.spec_from_file_location('animation_target_fixtures',Path(__file__).with_name('test-picking-animation-target.py'))
fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)

class Reader:
    def __init__(self,review=None):
        spr,ani,_=fixture.assets()
        self.payloads={'s':bytes(spr),'a':bytes(ani),'c':b'[CREATURE_0]\nTileSizeXY = 1\n[CREATURE_1]\n'}
        self.value=dict(kind='reviewed_finite_picking_animation_placement',success=True,policy=POLICY,
            actor=dict(slot=0,type=0,owner=0,footprint=1),assets={n:dict(record=k,sha256=self.digest(k))for n,k in [('spr','s'),('ani','a'),('CFG/creature.cfg','c')]},
            asset_pairs=[dict(sprite='spr',animation='ani')],offset_index={k:[list(p)for p in v]for k,v in asset_offsets(spr,ani).items()})
    def digest(self,n):return hashlib.sha256(self.read_bytes(n)).hexdigest()
    def read_bytes(self,n):return json.dumps(self.value,sort_keys=True).encode()if n=='review'else self.payloads[n]
    def json(self,n):assert n=='review';return copy.deepcopy(self.value)
    def __enter__(self):return self
    def __exit__(self,*a):pass
    def ref(self):return dict(record='review',sha256=self.digest('review'))
    def installation(self):return {n:dict(sha256=v['sha256'])for n,v in self.value['assets'].items()}

class IndexControls(unittest.TestCase):
    def test_exact_regeneration(self):
        r=Reader();self.assertEqual(load_index(r,r.ref(),r.installation()),r.value['offset_index'])
    def test_wrong_review_hash(self):
        r=Reader();ref=r.ref();ref['sha256']='0'*64
        with self.assertRaises(ValueError):load_index(r,ref,r.installation())
    def test_wrong_installation_hash(self):
        r=Reader();installation=r.installation();installation['spr']['sha256']='0'*64
        with self.assertRaises(ValueError):load_index(r,r.ref(),installation)
    def test_changed_retained_asset(self):
        r=Reader();r.payloads['a']+=b'x'
        with self.assertRaises(ValueError):load_index(r,r.ref(),r.installation())
    def test_forged_index(self):
        r=Reader();next(iter(r.value['offset_index'].values()))[0][0]+=1
        with self.assertRaises(ValueError):load_index(r,r.ref(),r.installation())
    def test_size_two_actor_refused(self):
        r=Reader();r.value['actor']['footprint']=2
        with self.assertRaises(ValueError):load_index(r,r.ref(),r.installation())
    def test_size_two_config_refused(self):
        r=Reader();r.payloads['c']=r.payloads['c'].replace(b'= 1',b'= 2');r.value['assets']['CFG/creature.cfg']['sha256']=r.digest('c')
        with self.assertRaises(ValueError):load_index(r,r.ref(),r.installation())
    def test_unknown_asset_refused(self):
        r=Reader();r.value['assets']['unexpected']=dict(record='s',sha256=r.digest('s'))
        with self.assertRaises(ValueError):load_index(r,r.ref(),r.installation())

class Base:
    def __init__(self):self.plan=dict(family='picking');self.root=Path('/unused')
    def patch(self,module):return module
    def receipt(self):return dict(game_actions_changed=False,response_deadlines_changed=False)

class BindingControls(unittest.TestCase):
    def binding(self):
        # Exercise the real patch/receipt methods without any installation IO.
        cls=binding_class(Base);b=cls.__new__(cls);Base.__init__(b)
        b.target_modules=set();b.animation_review=dict(record='review',sha256='0'*64)
        b.animation_targets=lambda records,history,position:None
        return b
    def module(self):return types.SimpleNamespace(__file__='/repo/tools/run-rendering-picking-closeout.py',targets=original_targets)
    def test_binding_and_idempotent_recursion(self):
        b=self.binding();m=self.module();self.assertIs(b.patch(m),m);self.assertIs(m.targets,b.animation_targets)
        b.patch(m);self.assertEqual(len(b.target_modules),1)
        self.assertTrue(b.receipt()['game_actions_changed']);self.assertFalse(b.receipt()['response_deadlines_changed'])
    def test_foreign_target_refused(self):
        b=self.binding();m=self.module();m.targets=lambda:None
        with self.assertRaises(ValueError):b.patch(m)
    def test_post_binding_change_refused(self):
        b=self.binding();m=self.module();b.patch(m);m.targets=original_targets
        with self.assertRaises(ValueError):b.patch(m)
    def test_unrelated_module_untouched(self):
        b=self.binding();m=self.module();m.__file__='/repo/tools/run-other.py';b.patch(m)
        self.assertIs(m.targets,original_targets)
    def test_no_binding_refused(self):
        with self.assertRaises(ValueError):self.binding().receipt()
    def test_duplicate_controller_refused(self):
        b=self.binding();first,second=self.module(),self.module();b.patch(first);b.patch(second)
        with self.assertRaises(ValueError):b.receipt()
    def test_exact_witness_contract_in_receipt(self):
        b=self.binding();b.patch(self.module());r=b.receipt()['picking_animation_target']
        self.assertEqual((r['quiet_source_samples'],r['exact_displayed_witnesses'],r['readiness_seconds']),(3,3,8))
        self.assertTrue(r['normal_retirement_required']);self.assertFalse(r['hit_feedback_search'])

if __name__=='__main__':unittest.main()
