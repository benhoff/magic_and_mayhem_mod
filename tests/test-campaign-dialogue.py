#!/usr/bin/env python3
"""Quiet actor state hints, independent from passive sampler sequencing."""
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_dialogue import ActorQuietWindow

class QuietActorTests(unittest.TestCase):
    def actor(self,**changes):
        return dict(dict(sequence=1,slot=0,type=0,owner=0,active=1,health=192,x=14,y=67,z=4,behavior=2,action=2,target=0xffffffff,mana=12800),**changes)
    def test_new_samples_do_not_restart_quiet_window(self):
        policy=ActorQuietWindow()
        for i in range(20):self.assertFalse(policy.observe({0:self.actor(sequence=i+1)},i*.3))
        self.assertTrue(policy.observe({0:self.actor(sequence=21)},6))
    def test_actual_actor_changes_restart_window(self):
        for change in (dict(health=191),dict(mana=12000),dict(x=15),dict(action=3),dict(owner=1),dict(type=24)):
            p=ActorQuietWindow();self.assertFalse(p.observe({0:self.actor()},0))
            changed={0:self.actor(**change)};self.assertFalse(p.observe(changed,5))
            self.assertFalse(p.observe(changed,10.99));self.assertTrue(p.observe(changed,11))
    def test_actor_set_changes_restart_window(self):
        p=ActorQuietWindow();p.observe({0:self.actor()},0)
        both={0:self.actor(),1:self.actor(slot=1,type=10,owner=2)}
        self.assertFalse(p.observe(both,5));self.assertFalse(p.observe({0:self.actor()},10))
        self.assertFalse(p.observe({0:self.actor()},15.99));self.assertTrue(p.observe({0:self.actor()},16))
    def test_empty_state_never_requests_input(self):
        p=ActorQuietWindow();self.assertFalse(p.observe({},0));self.assertFalse(p.observe({},100))
    def test_budget_is_bounded_across_state_changes(self):
        p=ActorQuietWindow(seconds=1,dismissals=2);p.observe({0:self.actor()},0)
        self.assertTrue(p.observe({0:self.actor()},1));self.assertFalse(p.observe({0:self.actor(x=15)},2))
        self.assertTrue(p.observe({0:self.actor(x=15)},3))
        with self.assertRaises(ValueError):p.observe({0:self.actor(x=15)},4)
    def test_invalid_policy_and_regressing_clock_refuse(self):
        for seconds,limit in ((0,1),(-1,1),(float('inf'),1),(1,0),(1,97)):
            with self.assertRaises(ValueError):ActorQuietWindow(seconds,limit)
        p=ActorQuietWindow();p.observe({0:self.actor()},3)
        for now in (2,float('nan')):
            with self.assertRaises(ValueError):p.observe({0:self.actor()},now)

if __name__=='__main__':unittest.main()
