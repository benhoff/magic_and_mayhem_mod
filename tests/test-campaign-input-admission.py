import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'tools'))
from campaign_input_admission import DialogueReadiness, spell_selection_points, validate_readiness_trace, fresh_hover, logical_input_point, pursuit_waypoint, living_engaged_targets, ground_waypoint, resolve_additional_summons


class AdmissionTest(unittest.TestCase):
    def test_selection_finishes_on_requested_toggle_from_every_initial_state(self):
        points = {'fireball': (477, 575), 'zombie': (523, 575)}
        for desired in points:
            for initial in (None, 'fireball', 'zombie'):
                selected = initial
                for point in spell_selection_points(desired):
                    clicked = next(s for s, p in points.items() if point == p)
                    selected = None if clicked == selected else clicked
                self.assertEqual(selected, desired)

    def test_unknown_loadout_is_refused(self):
        with self.assertRaises(ValueError):
            spell_selection_points('cure')

    def test_modal_restarts_full_quiet_window(self):
        gate = DialogueReadiness(False)
        self.assertEqual(gate.observe(dict(native=False, original=False), 0), 'wait')
        self.assertEqual(gate.observe(dict(native=True, original=True), 10), 'dismiss')
        self.assertEqual(gate.observe(dict(native=False, original=False), 12), 'wait')
        self.assertEqual(gate.observe(dict(native=False, original=False), 23), 'wait')
        self.assertEqual(gate.observe(dict(native=False, original=False), 24), 'ready')

    def test_dual_route_disagreement_does_not_click_or_admit(self):
        gate = DialogueReadiness(False, quiet_seconds=0)
        self.assertEqual(gate.observe(dict(native=True, original=False), 0), 'wait')
        self.assertEqual(gate.dismissals, 0)

    def test_resident_stale_cpu_image_is_not_readiness_oracle(self):
        gate = DialogueReadiness(True, quiet_seconds=0)
        self.assertEqual(gate.observe(dict(native=False, original=True), 0), 'wait')
        self.assertEqual(gate.observe(dict(native=False, original=True), 1), 'ready')

    def test_one_blank_sample_never_admits(self):
        self.assertEqual(DialogueReadiness(True, quiet_seconds=0).observe(dict(native=False, original=False), 100), 'wait')

    def test_observation_and_dismissal_bounds_are_independent(self):
        gate = DialogueReadiness(True, observations=3, dismissals=1)
        gate.observe(dict(native=True, original=True), 0)
        with self.assertRaises(RuntimeError):
            gate.observe(dict(native=True, original=True), 1)
        gate = DialogueReadiness(True, observations=2)
        for now in (0, 1):
            gate.observe(dict(native=False, original=False), now)
        with self.assertRaises(RuntimeError):
            gate.observe(dict(native=False, original=False), 2)

    def test_clock_and_malformed_observations_refuse(self):
        gate = DialogueReadiness(False)
        gate.observe(dict(native=False, original=False), 10)
        for visible, now in [(dict(native=False, original=False), 9), (dict(native=0, original=False), 11), (dict(native=False), 11)]:
            with self.assertRaises(ValueError):
                gate.observe(visible, now)

    def test_elapsed_bound_cannot_be_extended_by_new_dialogue(self):
        gate = DialogueReadiness(True, seconds=10)
        gate.observe(dict(native=True, original=True), 0)
        with self.assertRaises(RuntimeError):
            gate.observe(dict(native=False, original=False), 10)

    def readiness_record(self):
        rows = [dict(stage='dialogue-check', capture='first', texts=dict(native='', original=''),
                     visible=dict(native=False, original=False), observed_monotonic=0,
                     quiet_seconds=12, decision='wait'),
                dict(stage='dialogue-check', capture='second', texts=dict(native='', original=''),
                     visible=dict(native=False, original=False), observed_monotonic=12,
                     quiet_seconds=12, decision='ready')]
        return dict(input_readiness_policy='continuous-quiet-distinct-spell-v1',
                    captures=[dict(name='first'), dict(name='second')], dialogue_checks=rows)

    def test_v2_passive_budget_preserves_quiet_and_legacy_bound(self):
        record = self.readiness_record()
        record['input_readiness_policy'] = 'continuous-quiet-distinct-spell-v2'
        rows = []
        for now in range(13):
            row = dict(record['dialogue_checks'][0], capture=str(now), observed_monotonic=now,
                       decision='ready' if now == 12 else 'wait')
            rows.append(row)
        record.update(dialogue_checks=rows, captures=[dict(name=str(n)) for n in range(13)])
        self.assertTrue(validate_readiness_trace(record, True)['continuous_quiet_verified'])
        record['input_readiness_policy'] = 'continuous-quiet-distinct-spell-v1'
        with self.assertRaises(RuntimeError):
            validate_readiness_trace(record, True)
        gate = DialogueReadiness(True, observations=48)
        for now in range(12):
            self.assertEqual(gate.observe(dict(native=True, original=True), now), 'dismiss')
        with self.assertRaises(RuntimeError):
            gate.observe(dict(native=True, original=True), 12)

    def test_retained_trace_requires_complete_quiet_interval(self):
        record = self.readiness_record()
        self.assertTrue(validate_readiness_trace(record, False)['continuous_quiet_verified'])
        record['dialogue_checks'][-1]['observed_monotonic'] = 11
        with self.assertRaises(ValueError):
            validate_readiness_trace(record, False)

    def test_retained_trace_rejects_missing_capture_and_ocr_disagreement(self):
        record = self.readiness_record()
        record['captures'].pop()
        with self.assertRaises(ValueError):
            validate_readiness_trace(record, False)
        record = self.readiness_record()
        record['dialogue_checks'][-1]['texts']['native'] = 'Hermes'
        with self.assertRaises(ValueError):
            validate_readiness_trace(record, False)

    def test_retained_trace_cannot_relax_quiet_time(self):
        record = self.readiness_record()
        record['dialogue_checks'][0]['quiet_seconds'] = 1
        with self.assertRaises(ValueError):
            validate_readiness_trace(record, False)


class PursuitTest(unittest.TestCase):
    def test_approach_uses_ground_projection_instead_of_obstacle_limited_walking(self):
        # Retained failed approach: the path-derived basis repeatedly targeted
        # (570,336), while the wizard remained at (22,47). The existing
        # isometric projection directs the order toward the enemy's elevation.
        actor=dict(x=22,y=47,z=1)
        enemy=dict(x=31,y=52,z=4)
        self.assertEqual(ground_waypoint(actor,enemy),(480,410))
        self.assertEqual(ground_waypoint(actor,actor),(400,300))
        with self.assertRaises(ValueError):
            ground_waypoint(actor,dict(enemy,x=float('nan')))

    def test_recycled_or_retired_melee_recipient_does_not_become_a_pursuit_target(self):
        actor=dict(slot=2,type=10,owner=2,active=1,health=69,x=25,y=62,z=4)
        hit=[0]*16;hit[8:11]=[2,10,2]
        self.assertEqual(living_engaged_targets({2:actor},[hit,hit]),[actor])
        self.assertEqual(living_engaged_targets({},[hit]),[])
        for changes in [dict(type=14),dict(owner=0),dict(active=0),dict(health=0)]:
            self.assertEqual(living_engaged_targets({2:dict(actor,**changes)},[hit]),[])

    def test_fleeing_engaged_enemy_gets_a_bounded_physical_waypoint(self):
        wizard=dict(x=32,y=50,z=4,health=156)
        enemy=dict(x=25,y=62,z=4,health=69,active=1)
        self.assertEqual(pursuit_waypoint(wizard,enemy),(230,322))
        enemy.update(x=500,y=-500,z=-100)
        x,y=pursuit_waypoint(wizard,enemy)
        self.assertTrue(230<=x<=570 and 190<=y<=410)

    def test_nearby_dead_or_inactive_targets_do_not_receive_pursuit(self):
        wizard=dict(x=32,y=50,z=4,health=156)
        enemy=dict(x=32,y=52,z=4,health=69,active=1)
        self.assertIsNone(pursuit_waypoint(wizard,enemy))
        for changes in [dict(health=0),dict(active=0)]:
            enemy.update(x=25,y=62,health=69,active=1);enemy.update(changes)
            self.assertIsNone(pursuit_waypoint(wizard,enemy))
        wizard['health']=0
        self.assertIsNone(pursuit_waypoint(wizard,dict(enemy,health=69,active=1)))


class HoverTest(unittest.TestCase):
    def rows(self):
        def sample(sequence, kind, counter, cursor, point=(440, 309)):
            return dict(sequence=sequence,kind=kind,world_counter=counter,flags=127,
                        mouse=list(point),mouse_flags=0,input_active=1,x=point[0],y=point[1],
                        mode=2,cursor_type=cursor,selected_slot=1,spell_count=3,receiver_slot_plus_one=1,callback_mask=32 if kind==3 else 0,
                        ground=[12,72,4],ground_bounds=[80,80,24])
        return [sample(1,1,40,20,(523,574)), sample(2,3,41,20),
                sample(3,1,41,20), sample(4,1,42,5)]

    def test_published_cursor_and_motion_callback_do_not_admit_stale_world(self):
        rows = self.rows()
        self.assertIsNone(fresh_hover(rows[:3],(440,310),1,require_cast=True))
        self.assertEqual(fresh_hover(rows,(440,310),1,require_cast=True)['sequence'],4)

    def test_button_release_is_not_an_original_motion_return(self):
        for change in [dict(callback_mask=8),dict(mouse_flags=8),dict(mouse_flags=4)]:
            rows=self.rows();rows[1].update(change)
            self.assertIsNone(fresh_hover(rows,(440,310),1,require_cast=True))

    def test_later_original_callback_invalidates_an_obsolete_motion_proof(self):
        rows=self.rows();later=dict(rows[1],sequence=3,world_counter=42,mouse=[500,400])
        final=dict(rows[-1],sequence=4,world_counter=43)
        self.assertIsNone(fresh_hover([rows[0],rows[1],later,final],(440,310),1))
        # A new matching return and a strictly later World restore proof.
        restored=dict(rows[1],sequence=5,world_counter=44)
        world=dict(final,sequence=6,world_counter=45)
        self.assertEqual(fresh_hover([rows[0],rows[1],later,final,restored,world],(440,310),1)['sequence'],6)

    def test_qt_only_already_there_cannot_replace_original_position_proof(self):
        first=dict(self.rows()[-1],sequence=1,world_counter=40)
        second=dict(first,sequence=2,world_counter=41)
        self.assertIsNone(fresh_hover([first,second],(440,310),1,already_there=True))

    def test_missing_original_motion_cannot_be_replaced_by_qt_snapshot(self):
        rows = self.rows()
        self.assertIsNone(fresh_hover([rows[0],rows[-1]],(440,310),1,require_cast=True))

    def test_invalid_original_cast_target_does_not_send_positive_order(self):
        for key, value in [('cursor_type',20),('mode',1),('selected_slot',0xffffffff),
                           ('receiver_slot_plus_one',0),('selected_slot',3),('spell_count',0),('ground',[-1,72,4])]:
            rows = self.rows()
            rows[-1][key] = value
            self.assertIsNone(fresh_hover(rows,(440,310),1,require_cast=True))

    def test_already_hovered_point_still_needs_a_new_world_and_cannot_be_forged(self):
        rows = self.rows()
        self.assertIsNone(fresh_hover(rows,(440,310),1,already_there=True))
        self.assertEqual(fresh_hover(rows,(440,310),3,already_there=True,require_cast=True)['sequence'],4)
        self.assertIsNone(fresh_hover(rows[:3],(440,310),3,already_there=True))

    def test_unrelated_or_inactive_input_and_old_prefix_do_not_admit(self):
        for key, value in [('x',500),('input_active',0),('flags',119)]:
            rows = self.rows()
            rows[-1][key] = value
            self.assertIsNone(fresh_hover(rows,(440,310),1,require_cast=True))
        self.assertIsNone(fresh_hover(self.rows(),(440,310),4))

    def test_fractional_projection_is_compared_to_the_injected_integer_pixel(self):
        self.assertEqual(logical_input_point(570.,348.16037735849056),(570,348))
        rows = self.rows()
        for r in rows[1:]:
            r.update(x=569,y=347,mouse=[569,347])
        self.assertIsNone(fresh_hover(rows,(570.,348.16037735849056),1))
        self.assertIsNotNone(fresh_hover(rows,logical_input_point(570.,348.16037735849056),1))
        for x,y in [(800,300),(-1,300),(400,float('nan')),(400,600)]:
            with self.assertRaises(ValueError):
                logical_input_point(x,y)



class DelayedSummonTest(unittest.TestCase):
    def fixture(self):
        def actor(sequence,tick,slot,kind,owner,active,health,mana=0):
            row=[0]*16
            row[:9]=[sequence,1,316,tick,slot,kind,owner,active,health]
            row[15]=mana
            return tuple(row)
        gp=[actor(1,1000,0,0,0,1,200,6912),actor(2,1000,1,10,2,0,0),
            actor(3,1000,3,14,0,1,90),actor(4,2000,0,0,0,1,200,2560),
            actor(5,14178,1,14,0,1,90),actor(6,14200,0,0,0,1,200,2560)]
        cast=[1,1,316,1878,14,0,0,0,6912,2560,0,0,0,0,6,1]
        return gp,[tuple(cast)],[dict(before_sequence=3)]

    def test_accepted_effect_survives_a_short_poll_until_late_birth(self):
        gp,spells,attempts=self.fixture()
        confirmed,pending=resolve_additional_summons(gp[:4],spells,attempts,0,0)
        self.assertEqual(confirmed,[])
        self.assertEqual(pending,[dict(before_sequence=3,cast_return_sequence=1)])
        confirmed,pending=resolve_additional_summons(gp,spells,attempts,0,0)
        self.assertEqual(pending,[])
        self.assertEqual(confirmed,[dict(before_sequence=3,after_sequence=5,slot=1,cast_return_sequence=1)])

    def test_unrelated_birth_cannot_replace_a_valid_original_mana_debit(self):
        gp,spells,attempts=self.fixture()
        for field,value in [(2,999),(5,1),(6,14),(7,2),(9,6912),(9,0xffffffff),(15,0)]:
            changed=list(spells[0]);changed[field]=value
            self.assertEqual(resolve_additional_summons(gp,[tuple(changed)],attempts,0,0),([],[]))

    def test_existing_owned_zombie_cannot_be_adopted_as_new_creation(self):
        gp,spells,attempts=self.fixture();old=list(gp[1]);old[5:9]=[14,0,1,90];gp[1]=tuple(old)
        confirmed,pending=resolve_additional_summons(gp,spells,attempts,0,0)
        self.assertFalse(confirmed);self.assertTrue(pending)

    def test_duplicate_casts_or_ambiguous_born_slots_refuse(self):
        gp,spells,attempts=self.fixture()
        with self.assertRaises(ValueError):resolve_additional_summons(gp,spells+spells,attempts,0,0)
        born=list(gp[-1]);born[4:9]=[2,14,0,1,90];gp[-1]=tuple(born)
        with self.assertRaises(ValueError):resolve_additional_summons(gp,spells,attempts,0,0)

    def test_overlapping_accepted_effects_cannot_steal_a_late_birth(self):
        gp,spells,attempts=self.fixture()
        # A second accepted cast precedes the first birth: attribution is
        # unknown. The driver waits for pending accepted effects instead.
        second=list(spells[0]);second[0]=2;second[3]=2500;second[8:10]=[2560,0]
        attempts.append(dict(before_sequence=4))
        with self.assertRaises(ValueError):resolve_additional_summons(gp,spells+[tuple(second)],attempts,0,0)

    def test_attempt_order_and_thread_change_refuse(self):
        gp,spells,attempts=self.fixture()
        for bad in [[dict(before_sequence=0)],attempts*2,[dict(before_sequence=7)]]:
            with self.assertRaises(ValueError):resolve_additional_summons(gp,spells,bad,0,0)
        changed=list(gp[-1]);changed[2]=999;gp[-1]=tuple(changed)
        with self.assertRaises(ValueError):resolve_additional_summons(gp,spells,attempts,0,0)

    def test_monotonic_tick_wrap_keeps_accepted_effect_proof(self):
        gp,spells,attempts=self.fixture()
        offset=2**32-1500
        gp=[tuple((v+offset)%2**32 if k==3 else v for k,v in enumerate(row)) for row in gp]
        spells=[tuple((v+offset)%2**32 if k==3 else v for k,v in enumerate(row)) for row in spells]
        confirmed,pending=resolve_additional_summons(gp,spells,attempts,0,0)
        self.assertEqual(confirmed[0]['slot'],1);self.assertFalse(pending)


if __name__ == '__main__':
    unittest.main()
