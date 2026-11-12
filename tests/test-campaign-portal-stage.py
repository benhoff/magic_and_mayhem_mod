#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
import tempfile
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from campaign_portal_stage import is_portal_choice,probe_points,known_follower,PortalStage
from campaign_target_admission import portal_destination
spec=importlib.util.spec_from_file_location('portal_driver',ROOT/'tools/campaign-portal-outcome-input.py');driver=importlib.util.module_from_spec(spec);spec.loader.exec_module(driver)


class Portal(unittest.TestCase):
    def fixture(self):
        return dict(slot=0,type=0,owner=0,active=1,health=40,x=47,y=27,z=5),dict(slot=4,type=24,owner=1,active=0,health=0,x=46,y=25,z=4)

    def test_retained_wrong_high_ground_is_refused(self):
        h,g=self.fixture();self.assertFalse(portal_destination(h,g,[48,27,8]))
        self.assertTrue(portal_destination(h,g,[46,25,4]))
        self.assertTrue(portal_destination(h,g,[46,26,4]))

    def test_opposite_view_changes_the_candidate_ray_and_stays_in_viewport(self):
        h,g=self.fixture()
        for size in ((640,480),(800,600)):
            first=probe_points(h,g,size,0);opposite=probe_points(h,g,size,2)
            self.assertNotEqual(first[0],opposite[0]);self.assertTrue(opposite)
            for point in first+opposite:self.assertTrue(48<=point[0]<size[0]-40 and 64<=point[1]<size[1]-90)
        with self.assertRaises(ValueError):probe_points(h,g,(640,480),4)

    def test_only_retired_enemy_boss_portal_choice_is_adapted(self):
        h,g=self.fixture();self.assertTrue(is_portal_choice('portal','victory',h,g))
        for choice,case in (('fireball','victory'),('portal','defeat'),('portal','unknown')):
            self.assertFalse(is_portal_choice(choice,case,h,g))
        for who,change in (('h',dict(health=0)),('h',dict(active=0)),('h',dict(type=14)),('h',dict(owner=1)),('h',dict(slot=False)),('g',dict(type=10)),('g',dict(owner=0)),('g',dict(health=20)),('g',dict(slot=True))):
            h,g=self.fixture();(h if who=='h' else g).update(change);self.assertFalse(is_portal_choice('portal','victory',h,g))

    def test_combat_arguments_and_result_remain_the_retained_policy(self):
        h,g=self.fixture();seen=[];calls=[]
        original=lambda *a,**k:(seen.append((a,k)) or 'fireball')
        base=SimpleNamespace(combat_choice=original);stage=SimpleNamespace(approach=lambda *a:calls.append(a),close=lambda:calls.append('closed'))
        def main():return base.combat_choice('victory',h,g,2,5,3,support_ordered=True)
        base.main=main
        self.assertEqual(driver.run(base,stage),'fireball');self.assertEqual(calls,['closed']);self.assertIs(base.combat_choice,original)
        self.assertEqual(seen[0][0],('victory',h,g,2,5,3));self.assertEqual(seen[0][1],dict(support_ordered=True))

    def test_portal_adapter_owns_the_input_and_suppresses_a_duplicate_legacy_click(self):
        h,g=self.fixture();calls=[];original=lambda *a,**k:'portal'
        base=SimpleNamespace(combat_choice=original);stage=SimpleNamespace(approach=lambda *a:calls.append(a),close=lambda:calls.append('closed'))
        base.main=lambda:base.combat_choice('victory',h,g,1,10,5)
        self.assertEqual(driver.run(base,stage),'idle');self.assertEqual(calls,[(h,g),'closed']);self.assertIs(base.combat_choice,original)

    def test_current_early_portal_branch_uses_the_guarded_stage_once(self):
        h,g=self.fixture();calls=[];original=lambda *a,**k:'portal'
        base=SimpleNamespace(combat_choice=original,portal_approach_adapter=None)
        stage=SimpleNamespace(approach=lambda *a:calls.append(a),close=lambda:calls.append('closed'))
        base.main=lambda:base.portal_approach_adapter(h,g)
        driver.run(base,stage)
        self.assertEqual(calls,[(h,g),'closed']);self.assertIsNone(base.portal_approach_adapter)
        self.assertIs(base.combat_choice,original)

    def test_early_branch_refusal_restores_the_adapter_and_releases_input(self):
        h,g=self.fixture();calls=[];previous=object();original=lambda *a,**k:'portal'
        base=SimpleNamespace(combat_choice=original,portal_approach_adapter=previous)
        def refusal(*a):raise RuntimeError('refused')
        stage=SimpleNamespace(approach=refusal,close=lambda:calls.append('closed'))
        base.main=lambda:base.portal_approach_adapter(h,g)
        with self.assertRaises(RuntimeError):driver.run(base,stage)
        self.assertIs(base.portal_approach_adapter,previous);self.assertIs(base.combat_choice,original)
        self.assertEqual(calls,['closed'])

    def test_failed_stage_restores_the_legacy_function_and_closes_owned_input(self):
        h,g=self.fixture();calls=[];original=lambda *a,**k:'portal';base=SimpleNamespace(combat_choice=original)
        def refusal(*a):raise RuntimeError('refused')
        stage=SimpleNamespace(approach=refusal,close=lambda:calls.append('closed'));base.main=lambda:base.combat_choice('victory',h,g,1,10,5)
        with self.assertRaises(RuntimeError):driver.run(base,stage)
        self.assertIs(base.combat_choice,original);self.assertEqual(calls,['closed'])


class ObservedStage(unittest.TestCase):
    def actors(self):
        h=dict(slot=0,type=0,owner=0,active=1,health=40,x=47,y=27,z=5)
        b=dict(slot=4,type=24,owner=1,active=0,health=0,x=46,y=25,z=4)
        return h,b

    def test_follower_cover_requires_the_previously_observed_original_reference(self):
        h,_=self.actors();z=dict(slot=3,type=14,owner=0,active=1,health=70)
        legacy=dict(support_follow=dict(source=z,reference_observed=dict(source=3,reference=0,source_health=90,target_health=196)))
        self.assertIs(known_follower({0:h,3:z},h,legacy),z)
        self.assertIsNone(known_follower({0:h,3:z},h,{}))
        for change in (dict(source=4),dict(reference=1),dict(reference=False),dict(source_health=0),dict(target_health=0)):
            bad=dict(support_follow=dict(legacy['support_follow'],reference_observed=dict(legacy['support_follow']['reference_observed'],**change)))
            self.assertIsNone(known_follower({0:h,3:z},h,bad))
        self.assertIsNone(known_follower({0:h,3:dict(z,health=0)},h,legacy))

    def mock_stage(self,path):
        h,b=self.actors();stage=PortalStage(path,(0,0,640,480),(640,480),path/'portal.json')
        stage.open=lambda:None;stage.actors=lambda:{0:h};stage.orders=lambda:[dict(kind=1,sequence=1,flags=1,primary=[0])]
        calls=[];stage.key=lambda key:calls.append(key);stage.right=lambda:self.fail('Refused ground must not send a right click')
        return stage,h,b,calls

    def test_all_refused_probes_release_the_pause_without_move_credit(self):
        with tempfile.TemporaryDirectory() as d:
            stage,h,b,calls=self.mock_stage(Path(d));reports=iter([None,dict(observed=True)])
            stage.report=lambda:next(reports);stage.hover=lambda *a,**k:None
            stage.approach(h,b)
            self.assertFalse(stage.paused);self.assertEqual(calls.count(0x20),2);self.assertEqual(stage.result['moves'],[])

    def test_decode_refusal_releases_the_pause_before_propagating(self):
        with tempfile.TemporaryDirectory() as d:
            stage,h,b,calls=self.mock_stage(Path(d));stage.report=lambda:None
            def refusal(*a,**k):raise ValueError('bad observation')
            stage.hover=refusal
            with self.assertRaises(ValueError):stage.approach(h,b)
            self.assertFalse(stage.paused);self.assertEqual(calls.count(0x20),2);self.assertEqual(stage.result['moves'],[])


if __name__=='__main__':unittest.main()
