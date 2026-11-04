#!/usr/bin/env python3
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_target_admission import candidate_after_hover,empty_candidate_after_hover,hover_nudge,support_attack_submission,support_selection_after,primary_selection_after,reserved_boss_target


class Candidate(unittest.TestCase):
    def row(self,**updates):
        return dict(dict(kind=1,sequence=20,world_counter=42,candidate_valid=True,candidate_slot_plus_one=5),**updates)

    def test_original_candidate_needs_independent_current_hover(self):
        row=self.row()
        self.assertIsNone(candidate_after_hover([row],None,4,10))
        self.assertEqual(candidate_after_hover([row],dict(world_counter=42),4,10),row)

    def test_latest_wrong_candidate_supersedes_old_match(self):
        self.assertIsNone(candidate_after_hover([self.row(),self.row(sequence=21,candidate_slot_plus_one=1)],dict(world_counter=42),4,10))

    def test_stale_unknown_null_and_wrong_slot_refuse(self):
        for changes in (dict(world_counter=41),dict(candidate_valid=False),dict(candidate_slot_plus_one=0),dict(sequence=10)):
            with self.subTest(changes=changes):
                self.assertIsNone(candidate_after_hover([self.row(**changes)],dict(world_counter=42),4,10))

    def test_candidate_return_is_not_a_world_hover_update(self):
        self.assertIsNone(candidate_after_hover([self.row(kind=3)],dict(world_counter=42),4,10))
        for kind in (2,3):
            self.assertIsNone(candidate_after_hover([self.row(),self.row(kind=kind,sequence=21)],dict(world_counter=42),4,10))

    def test_counter_wrap_is_forward_only(self):
        row=self.row(world_counter=1)
        self.assertEqual(candidate_after_hover([row],dict(world_counter=0xffffffff),4,10),row)
        self.assertIsNone(candidate_after_hover([self.row(world_counter=0xffffffff)],dict(world_counter=1),4,10))

    def test_paused_candidate_requires_the_actual_hover_return_token(self):
        ready=dict(world_counter=42,world_return_ordinal=100)
        self.assertIsNone(candidate_after_hover([self.row(world_return_ordinal=99)],ready,4,10))
        row=self.row(world_return_ordinal=100)
        self.assertEqual(candidate_after_hover([row],ready,4,10),row)

    def test_mixed_return_and_clock_proofs_do_not_fall_back(self):
        self.assertIsNone(candidate_after_hover([self.row()],dict(world_counter=42,world_return_ordinal=100),4,10))
        self.assertIsNone(candidate_after_hover([self.row(world_return_ordinal=100)],dict(world_counter=42),4,10))

    def test_empty_ground_requires_known_null_and_current_hover(self):
        ready=dict(world_counter=42,world_return_ordinal=100)
        row=self.row(candidate_slot_plus_one=0,world_return_ordinal=100)
        self.assertEqual(empty_candidate_after_hover([row],ready,10),row)
        self.assertIsNone(empty_candidate_after_hover([row],None,10))
        self.assertIsNone(empty_candidate_after_hover([self.row(candidate_slot_plus_one=0,candidate_valid=False,world_return_ordinal=100)],ready,10))

    def test_empty_ground_does_not_accept_an_actor_or_old_return(self):
        ready=dict(world_counter=42,world_return_ordinal=100)
        self.assertIsNone(empty_candidate_after_hover([self.row(world_return_ordinal=100)],ready,10))
        self.assertIsNone(empty_candidate_after_hover([self.row(candidate_slot_plus_one=0,world_return_ordinal=99)],ready,10))

    def test_actor_ids_are_bounded_numeric_observations(self):
        for slot in (-1,256,None):
            with self.assertRaises(ValueError):candidate_after_hover([],None,slot,0)

    def test_nudge_is_real_bounded_motion_and_preserves_requested_y(self):
        self.assertEqual(hover_nudge((470,226),(640,480)),(454,226))
        self.assertEqual(hover_nudge((320,218),(640,480)),(336,218))
        for point in ((0,100),(639,100),(400,300)):
            p=hover_nudge(point,(640,480));self.assertNotEqual(p,point);self.assertTrue(0<=p[0]<640);self.assertEqual(p[1],point[1])
        for point in ((640,100),(0,-1),(float('nan'),100)):
            with self.assertRaises(ValueError):hover_nudge(point,(640,480))

    def attack(self):
        source=dict(slot=3,type=14,owner=0,active=1,health=89)
        target=dict(slot=4,type=24,owner=1,active=1,health=100)
        a=dict(kind=2,sequence=11,call=1,mask=4,flags=35,primary=[3],records=[])
        b=dict(a,kind=3,sequence=12,flags=39,records=[dict(opcode=7,actor_slot=3,target_slot=4)])
        return source,target,[a,b]

    def test_selection_needs_a_new_complete_sole_primary_world(self):
        source,_,_=self.attack()
        row=dict(kind=1,sequence=21,flags=1,primary=[3])
        self.assertIs(support_selection_after([row],source,20),row)
        self.assertIsNone(support_selection_after([row],source,21))

    def test_generic_primary_witness_can_acknowledge_wizard_before_team_recall(self):
        row=dict(kind=1,sequence=21,flags=1,primary=[0])
        self.assertIs(primary_selection_after([row],0,20),row)
        self.assertIsNone(primary_selection_after([row],3,20))

    def test_generic_primary_witness_rejects_unbounded_and_boolean_actor_ids(self):
        for slot in (-1,256,True,None):
            with self.assertRaises(ValueError):primary_selection_after([],slot,0)

    def test_selection_refuses_unknown_pool_pending_callback_and_other_primary(self):
        source,_,_=self.attack()
        for changes in (dict(flags=0),dict(kind=2),dict(kind=3),dict(primary=[0]),dict(primary=[3,7])):
            row=dict(kind=1,sequence=21,flags=1,primary=[3]);row.update(changes)
            self.assertIsNone(support_selection_after([row],source,20))

    def test_later_selection_state_supersedes_an_earlier_sole_primary(self):
        source,_,_=self.attack()
        first=dict(kind=1,sequence=21,flags=1,primary=[3])
        for latest in (dict(first,sequence=22,primary=[0]),dict(first,sequence=22,kind=2)):
            self.assertIsNone(support_selection_after([first,latest],source,20))

    def test_world_sample_inside_an_unreturned_callback_is_not_selection_ack(self):
        source,_,_=self.attack();world=dict(kind=1,sequence=21,flags=1,primary=[3])
        entered=dict(kind=2,sequence=10,call=1)
        self.assertIsNone(support_selection_after([entered,world],source,20))
        returned=dict(kind=3,sequence=11,call=1)
        self.assertIs(support_selection_after([entered,returned,world],source,20),world)

    def test_inner_return_does_not_close_an_outer_pending_callback(self):
        source,_,_=self.attack();world=dict(kind=1,sequence=21,flags=1,primary=[3])
        rows=[dict(kind=2,sequence=10,call=1),dict(kind=2,sequence=11,call=2),dict(kind=3,sequence=12,call=2),world]
        self.assertIsNone(support_selection_after(rows,source,20))
        latest=dict(world,sequence=23)
        self.assertIs(support_selection_after(rows+[dict(kind=3,sequence=22,call=1),latest],source,20),latest)

    def test_selection_refuses_changed_dead_and_unbounded_source_roles(self):
        for changes in (dict(active=0),dict(health=0),dict(type=10),dict(owner=1),dict(slot=256),dict(slot=True)):
            source,_,_=self.attack();source.update(changes)
            self.assertIsNone(support_selection_after([dict(kind=1,sequence=21,flags=1,primary=[3])],source,20))
        source,_,_=self.attack()
        with self.assertRaises(ValueError):support_selection_after([],source,-1)

    def test_submission_is_distinct_from_combat_target_or_delivery(self):
        source,target,rows=self.attack();source['target']=0xffffffff
        result=support_attack_submission(rows,source,target,10)
        self.assertEqual(result['command']['target_slot'],4)
        self.assertFalse(result['command_delivery_proved'])

    def test_wrong_command_actor_target_selection_or_stale_pair_refuse(self):
        for field,value in (('opcode',5),('actor_slot',0),('target_slot',1)):
            source,target,rows=self.attack();rows[-1]['records'][0][field]=value
            self.assertIsNone(support_attack_submission(rows,source,target,10))
        for field,value in (('primary',[0]),('flags',34),('sequence',10)):
            source,target,rows=self.attack();rows[0][field]=value
            self.assertIsNone(support_attack_submission(rows,source,target,10))

    def test_dead_or_changed_roles_cannot_acknowledge_attack(self):
        for changes in (dict(health=0),dict(owner=1),dict(type=10),dict(active=0)):
            source,target,rows=self.attack();source.update(changes)
            self.assertIsNone(support_attack_submission(rows,source,target,10))


class Reserve(unittest.TestCase):
    def actors(self):
        hero=dict(slot=0,type=0,owner=0,active=1,health=168,mana=36*256)
        return hero,{0:hero,3:dict(slot=3,type=14,owner=0,active=1,health=90),4:dict(slot=4,type=24,owner=1,active=1,health=100),6:dict(slot=6,type=11,owner=1,active=1,health=30,target=0)}

    def test_supported_boss_reserve_survives_an_escort_priority(self):
        hero,actors=self.actors();self.assertIs(reserved_boss_target(hero,actors,True),actors[4])
        hero['mana']=30*256;self.assertIs(reserved_boss_target(hero,actors,True),actors[4])

    def test_budget_follows_observed_boss_health_but_never_grants_cast(self):
        hero,actors=self.actors();hero['mana']=30*256-1
        self.assertIsNone(reserved_boss_target(hero,actors,True))
        actors[4]['health']=80;hero['mana']=24*256
        self.assertIs(reserved_boss_target(hero,actors,True),actors[4])
        actors[4]['health']=121;hero['mana']=50*256
        self.assertIsNone(reserved_boss_target(hero,actors,True))

    def test_route_interruptions_low_health_and_unarmed_route_keep_existing_policy(self):
        hero,actors=self.actors()
        self.assertIsNone(reserved_boss_target(hero,actors,False))
        self.assertIsNone(reserved_boss_target(hero,actors,True,True))
        hero['health']=79;self.assertIsNone(reserved_boss_target(hero,actors,True))

    def test_ambiguous_dead_or_changed_roles_refuse_reserve(self):
        for slot,changes in [(3,dict(health=0)),(3,dict(type=10)),(4,dict(health=0)),(4,dict(owner=2)),(0,dict(owner=1))]:
            hero,actors=self.actors();actors[slot].update(changes)
            self.assertIsNone(reserved_boss_target(hero,actors,True))
        hero,actors=self.actors();actors[7]=dict(actors[3],slot=7)
        self.assertIsNone(reserved_boss_target(hero,actors,True))


if __name__=='__main__':unittest.main()
