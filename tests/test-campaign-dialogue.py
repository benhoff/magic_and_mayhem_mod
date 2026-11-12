#!/usr/bin/env python3
"""Quiet actor state hints, independent from passive sampler sequencing."""
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_dialogue import ActorQuietWindow,terminal_return_hint

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

class TerminalReturnTests(unittest.TestCase):
    def fixtures(self):
        hero=dict(slot=0,type=0,owner=0,active=1,health=76)
        boss=dict(slot=4,type=24,owner=1,active=1,health=20)
        # Original spell-event tuple: kind,tick,target,HP before/after,
        # applied flag, attribution owner, spell, caster slot/owner.
        row=(1,3,0,1000,4,24,1,40,20,1,0,0,71,0,0,0)
        return hero,boss,row
    def test_fresh_player_damage_allows_one_hint(self):
        hero,boss,row=self.fixtures()
        self.assertEqual(terminal_return_hint(hero,boss,[row],11000),row)
        self.assertIsNone(terminal_return_hint(hero,boss,[row],11001))
        self.assertIsNone(terminal_return_hint(hero,boss,[row],999))
        self.assertIsNone(terminal_return_hint(hero,boss,[row],1000,already=True))
        wrapped=list(row);wrapped[3]=0xfffffff0;wrapped=tuple(wrapped)
        self.assertEqual(terminal_return_hint(hero,boss,[wrapped],20),wrapped)
    def test_unrelated_or_non_damaging_events_refuse(self):
        hero,boss,row=self.fixtures()
        for index,value in ((1,2),(4,5),(5,10),(6,0),(7,20),(8,0),(9,0),(11,1),(12,70),(14,1),(15,1)):
            changed=list(row);changed[index]=value
            self.assertIsNone(terminal_return_hint(hero,boss,[tuple(changed)],1000),(index,value))
        self.assertIsNone(terminal_return_hint(hero,boss,[],1000))
    def test_changed_actor_roles_or_terminal_state_refuse(self):
        hero,boss,row=self.fixtures()
        for field,value in (('active',0),('health',0),('type',1),('owner',1)):
            self.assertIsNone(terminal_return_hint(dict(hero,**{field:value}),boss,[row],1000))
        for field,value in (('active',0),('health',0),('health',40),('type',10),('owner',0),('slot',5)):
            self.assertIsNone(terminal_return_hint(hero,dict(boss,**{field:value}),[row],1000))

if __name__=='__main__':unittest.main()
