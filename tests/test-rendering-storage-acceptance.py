#!/usr/bin/env python3
"""Both sizes and explicit target admission are mandatory for storage credit."""
import importlib.util,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from rendering_storage_acceptance import progress
spec=importlib.util.spec_from_file_location('normal_controls',ROOT/'tests/test-rendering-normal-acceptance.py');fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)


class StorageTests(unittest.TestCase):
    def setUp(self):self.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text());self.reader=fixture.Reader()
    def pair(self,mode,software=False):
        p=self.reader.pair('normal-storage',mode);a=self.reader.data[p['acquisition']['record']];a['session_identity']=str(mode+1)*32
        if software:a['configuration']['software_flag']=True;a['resident']['renderer']='llvmpipe (controlled)'
        self.reader.refresh(p);return p
    def test_each_target_needs_both_sizes(self):
        high=self.pair(0,True);r=progress(self.plan,self.reader,[('player-software',high)])
        self.assertEqual(r['passed_requirements'],0)
        low=self.pair(1,True);r=progress(self.plan,self.reader,[('player-software',high),('player-software',low)])
        self.assertEqual((r['passed_requirements'],r['pending_requirements']),(4,152))
        self.assertTrue(all(x['configuration']=='player-software' and x['kind']=='journey' for x in r['requirements'] if x['state']=='passed'))
        self.assertEqual(set(r['decisions'].values()),{'pending'})
        self.assertFalse(r['historical_failures_retired'])
    def test_software_flag_and_provider_cannot_be_inferred_from_default(self):
        p=self.pair(0)
        with self.assertRaises(ValueError):progress(self.plan,self.reader,[('player-software',p)])
    def test_same_execution_or_unknown_target_refuses(self):
        p=self.pair(0)
        with self.assertRaises(ValueError):progress(self.plan,self.reader,[('player-default',p),('player-default',p)])
        with self.assertRaises(ValueError):progress(self.plan,self.reader,[('invented-target',p)])
    def test_both_targets_close_only_eight_named_leaves(self):
        groups=[('player-default',self.pair(0)),('player-default',self.pair(1))]
        for mode,size in ((2,[800,600]),(3,[640,480])):
            pair=self.pair(mode,True);self.reader.data[pair['comparison']['record']]['world_domain']['extents']=[size];self.reader.refresh(pair)
            groups.append(('player-software',pair))
        r=progress(self.plan,self.reader,groups)
        self.assertEqual((r['passed_requirements'],r['pending_requirements']),(8,148))
        self.assertEqual(len(r['requirements']),156)
        self.assertTrue(all(x['kind']=='journey' for x in r['requirements'] if x['state']=='passed'))


if __name__=='__main__':unittest.main()
