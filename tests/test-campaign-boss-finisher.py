#!/usr/bin/env python3
"""Keep physical finishing priority bound to the actual original order pair."""
import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_boss_finisher import boss_finisher,finishing_pause_target,wizard_melee_finisher,wizard_melee_submission


class Finisher(unittest.TestCase):
    def fixture(self):
        hero=dict(slot=0,type=0,owner=0,active=1,health=44,mana=14*256,x=42,y=31)
        actors={0:hero,3:dict(slot=3,type=14,owner=0,active=1,health=74),
                4:dict(slot=4,type=24,owner=1,active=1,health=40,x=43,y=27),
                6:dict(slot=6,type=10,owner=1,active=1,health=110,x=43,y=30)}
        entry=dict(kind=2,call=3,sequence=10,primary=[3],flags=1,mask=4,thread=312)
        command=dict(opcode=7,actor_slot=3,target_slot=4)
        returned=dict(kind=3,call=3,sequence=11,primary=[3],flags=5,mask=4,thread=312,records=[command])
        proof=dict(command=command,entry=entry,returned=returned)
        return hero,actors,proof

    def test_wounded_boss_retains_input_priority_below_initial_eighty_hp_reserve(self):
        h,a,p=self.fixture();self.assertIs(boss_finisher(h,a,p),a[4])

    def test_retained_ten_hp_support_hit_keeps_finishing_priority(self):
        h,a,p=self.fixture();a[4].update(health=10,x=45,y=25);h.update(x=43,y=33)
        self.assertIs(boss_finisher(h,a,p),a[4])
        a[4]['health']=1
        self.assertIs(boss_finisher(h,a,p),a[4])
        a[4]['health']=20
        self.assertIsNone(boss_finisher(h,a,p))

    def test_one_finishing_pause_needs_the_released_same_boss_volley(self):
        h,a,p=self.fixture();first=dict(released=True,accepted_in_burst=4,target=dict(a[4],health=100))
        self.assertIs(finishing_pause_target(h,a,p,first,False),a[4])
        self.assertIsNone(finishing_pause_target(h,a,p,first,True))

    def test_unreleased_unbounded_or_other_boss_volley_refuses_second_pause(self):
        h,a,p=self.fixture();first=dict(released=True,accepted_in_burst=4,target=dict(a[4],health=100))
        for changed in (None,{},dict(first,released=False),dict(first,accepted_in_burst=True),
                        dict(first,accepted_in_burst=0),dict(first,accepted_in_burst=5),
                        dict(first,target=dict(a[4],slot=5)),dict(first,target=dict(a[4],owner=2))):
            self.assertIsNone(finishing_pause_target(h,a,p,changed,False))

    def test_second_pause_never_bypasses_current_target_or_order_guards(self):
        h,a,p=self.fixture();first=dict(released=True,accepted_in_burst=4,target=dict(a[4],health=100))
        a[4]['health']=20
        self.assertIsNone(finishing_pause_target(h,a,p,first,False))
        a[4]['health']=30
        self.assertIsNone(finishing_pause_target(h,a,{},first,False))

    def test_observed_regeneration_keeps_the_existing_committed_target(self):
        h,a,p=self.fixture();a[4]['health']=100
        self.assertIs(boss_finisher(h,a,p),a[4])

    def test_later_support_death_does_not_erase_an_actual_order_submission(self):
        h,a,p=self.fixture();a[3].update(active=0,health=0)
        self.assertIs(boss_finisher(h,a,p),a[4])

    def test_no_boolean_or_unrelated_order_can_supply_priority(self):
        h,a,p=self.fixture()
        for invalid in (None,True,{},dict(p,command=dict(opcode=5,actor_slot=3,target_slot=4)),
                        dict(p,command=dict(opcode=7,actor_slot=0,target_slot=4)),
                        dict(p,command=dict(opcode=7,actor_slot=3,target_slot=6))):
            with self.subTest(invalid=invalid):self.assertIsNone(boss_finisher(h,a,invalid))

    def test_missing_closed_receiver_identity_refuses(self):
        for key,change in (('entry',dict(primary=[0])),('returned',dict(primary=[0,3])),
                           ('returned',dict(call=4)),('returned',dict(kind=2)),
                           ('returned',dict(flags=1)),('returned',dict(mask=8)),
                           ('returned',dict(thread=444)),('returned',dict(records=[])),
                           ('returned',dict(sequence=9))):
            h,a,p=self.fixture();p[key].update(change)
            with self.subTest(change=change):self.assertIsNone(boss_finisher(h,a,p))

    def test_dead_foreign_insufficient_or_unbounded_wizard_refuses(self):
        for change in (dict(health=0),dict(active=0),dict(owner=1),dict(type=14),dict(mana=1535),dict(slot=True)):
            h,a,p=self.fixture();h.update(change)
            with self.subTest(change=change):self.assertIsNone(boss_finisher(h,a,p))

    def test_unbounded_dead_foreign_ambiguous_or_distant_boss_refuses(self):
        for change in (dict(health=101),dict(health=20),dict(health=0),dict(active=0),dict(owner=2),dict(type=10),dict(x=60)):
            h,a,p=self.fixture();a[4].update(change)
            with self.subTest(change=change):self.assertIsNone(boss_finisher(h,a,p))
        h,a,p=self.fixture();a[7]=dict(a[4],slot=7)
        self.assertIsNone(boss_finisher(h,a,p))


class WizardMelee(unittest.TestCase):
    def fixture(self):
        h=dict(slot=0,type=0,owner=0,active=1,health=121,mana=1024,x=44,y=28,z=4)
        b=dict(slot=4,type=24,owner=1,active=1,health=9,x=43,y=28,z=4)
        damage=(10,2,312,1000,0,0,0,121,4,24,1,11,9,1,1,6)
        pose=(11,1,312,1500)+(0,)*12
        cmd=dict(opcode=7,length=14,actor_slot=0,target_slot=4)
        entry=dict(kind=2,sequence=21,tick=1600,thread=312,call=2,mask=4,flags=1,primary=[0],records=[])
        returned=dict(entry,kind=3,sequence=22,flags=5,records=[cmd])
        world=dict(kind=1,sequence=23,flags=1,primary=[0])
        return h,b,[damage,pose],[entry,returned,world]

    def test_observed_nine_hp_wizard_melee_can_continue_without_mana(self):
        h,b,rows,_=self.fixture()
        self.assertIs(wizard_melee_finisher(h,{0:h,4:b},rows),b)

    def test_initial_wizard_and_boss_roles_health_mana_and_range_are_bounded(self):
        for which,change in (('h',dict(health=79)),('h',dict(mana=1536)),('h',dict(mana=-1)),
                            ('h',dict(owner=1)),('h',dict(type=14)),('h',dict(active=0)),('h',dict(slot=False)),
                            ('b',dict(health=20)),('b',dict(health=0)),('b',dict(active=0)),('b',dict(owner=0)),
                            ('b',dict(type=10)),('b',dict(x=51)),('b',dict(slot=True))):
            h,b,rows,_=self.fixture();(h if which=='h' else b).update(change)
            self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows))
        h,b,rows,_=self.fixture()
        self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b,5:dict(b,slot=5)},rows))

    def test_initial_damage_must_be_recent_actual_same_thread_wizard_to_same_boss(self):
        for index,value in ((1,1),(2,313),(3,2**32-10000),(4,3),(5,14),(6,1),(7,0),
                            (8,5),(9,10),(10,0),(11,9),(12,0),(12,20),(13,0)):
            h,b,rows,_=self.fixture();bad=list(rows[0]);bad[index]=value;rows[0]=tuple(bad)
            self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows))
        h,b,rows,_=self.fixture()
        self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows[1:]))

    def test_attack_ack_requires_closed_fresh_sole_wizard_opcode7_length14(self):
        h,b,_,orders=self.fixture();proof=wizard_melee_submission(orders,h,b,20)
        self.assertEqual(proof['command'],orders[1]['records'][0]);self.assertFalse(proof['command_delivery_proved'])
        for change in (dict(opcode=5),dict(opcode=10),dict(length=19),dict(actor_slot=3),dict(actor_slot=False),dict(target_slot=5)):
            h,b,_,orders=self.fixture();orders[1]['records'][0].update(change)
            self.assertIsNone(wizard_melee_submission(orders,h,b,20))
        self.assertIsNone(wizard_melee_submission(orders,h,b,22))

    def test_attack_ack_refuses_open_mixed_foreign_and_wrong_receiver_prefixes(self):
        for index,change in ((0,dict(primary=[3])),(1,dict(primary=[0,3])),(1,dict(thread=313)),
                             (1,dict(call=3)),(1,dict(mask=8)),(1,dict(flags=1)),(2,dict(primary=[3]))):
            h,b,_,orders=self.fixture();orders[index].update(change)
            self.assertIsNone(wizard_melee_submission(orders,h,b,20))
        h,b,_,orders=self.fixture()
        self.assertIsNone(wizard_melee_submission(orders[:1],h,b,20))

    def test_ack_continuation_is_thirty_seconds_without_restarting_attack(self):
        h,b,rows,orders=self.fixture();proof=wizard_melee_submission(orders,h,b,20)
        h['health']=40;rows[-1]=(11,1,312,31000)+(0,)*12
        self.assertIs(wizard_melee_finisher(h,{0:h,4:b},rows,proof),b)
        h['health']=39
        self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows,proof))
        h['health']=121;rows[-1]=(11,1,312,32000)+(0,)*12
        self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows,proof))
        proof['command']['opcode']=10;rows[-1]=(11,1,312,31000)+(0,)*12
        self.assertIsNone(wizard_melee_finisher(h,{0:h,4:b},rows,proof))


if __name__=='__main__':unittest.main()
