#!/usr/bin/env python3
"""Driver readiness and retained wrong-direction navigation regressions."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('combat_input',ROOT/'tools/campaign-combat-input.py')
driver=importlib.util.module_from_spec(spec)
spec.loader.exec_module(driver)


class ReadinessTest(unittest.TestCase):
    def prepare(self,spell_cases=False,modals=()):
        events=[];modal=iter(modals)
        def settle(label,*seconds):
            events.append(('settle',label))
            return next(modal,0) if label.startswith('summon-selection') else 0
        driver.prepare_summon(lambda *p:events.append(('click',*p)),
            lambda:events.append(('select-wizard',)),settle,
            lambda *p:events.append(('blocked-cast',*p)),owner=2,wizard_slot=7,
            spell_cases=spell_cases,wait=lambda _:None)
        return events

    def test_repeated_selection_visits_keep_distinct_complete_readiness_stages(self):
        from campaign_input_admission import validate_readiness_trace
        record=dict(input_readiness_policy='continuous-quiet-distinct-spell-v2',
                    dialogue_checks=[],additional_dialogue_checks=[],captures=[])
        now=0
        for label in ['dialogue-check','zombie-selection-0','after-invalid-target','zombie-selection-0']:
            rows=record['dialogue_checks' if label=='dialogue-check' else 'additional_dialogue_checks']
            stage=driver.dialogue_stage(label,rows)
            quiet=12 if label=='dialogue-check' or label.startswith('after-') else 2
            for elapsed,decision in [(0,'wait'),(quiet,'ready')]:
                name=str(len(record['captures']));record['captures'].append(dict(name=name))
                rows.append(dict(stage=stage,capture=name,quiet_seconds=quiet,
                                 visible=dict(native=False,original=False),texts=dict(native='',original=''),
                                 observed_monotonic=now+elapsed,decision=decision))
            now+=quiet+1
        result=validate_readiness_trace(record,True)
        self.assertEqual(len(result['stages']),len(set(result['stages'])))
        self.assertEqual(len(result['stages']),4)
        # Old repeated labels still refuse: every visit must retain its own gate.
        for row in record['additional_dialogue_checks']:
            if row['stage'].startswith('zombie-selection-0'):row['stage']='zombie-selection-0'
        with self.assertRaisesRegex(ValueError,'Duplicate or invalid'):
            validate_readiness_trace(record,True)

    def test_default_route_selects_caster_before_spell_hud(self):
        events=self.prepare()
        self.assertEqual(events[:2],[('select-wizard',),('settle','after-center')])
        self.assertEqual([e for e in events if e[0]=='click'],[('click',523,575)])
        self.assertFalse(any(e[0]=='blocked-cast' for e in events))

    def test_optional_refusal_uses_the_selected_caster(self):
        events=self.prepare(True)
        self.assertLess(events.index(('select-wizard',)),events.index(('blocked-cast','invalid-target',2,7)))
        self.assertLess(events.index(('blocked-cast','invalid-target',2,7)),events.index(('click',523,575)))

    def test_delayed_modal_retries_only_spell_selection(self):
        events=self.prepare(modals=(1,0))
        self.assertEqual([e for e in events if e[0]=='click'],[('click',523,575)]*2)
        self.assertEqual(events.count(('select-wizard',)),1)

    def test_repeated_modals_refuse_positive_cast_admission(self):
        with self.assertRaisesRegex(RuntimeError,'repeatedly consumed'):
            self.prepare(modals=(1,1,1))


class GroundProjectionTest(unittest.TestCase):
    def test_retained_obstructed_path_regression_points_toward_enemy(self):
        # The old fitted basis sent this retained pair toward [150,-110].
        sx,sy=driver.ground_step(dict(x=12,y=73,z=4),dict(x=31,y=52,z=4))
        self.assertAlmostEqual(sx,170)
        self.assertAlmostEqual(sy,-4.25)

    def test_height_and_direction_are_preserved_when_bounded(self):
        for target,expected in [(dict(x=10,y=10,z=0),(0,110)),
                                (dict(x=-10,y=10,z=0),(-170,0)),
                                (dict(x=0,y=0,z=2),(0,-32)),
                                (dict(x=0,y=0,z=0),(0,0))]:
            with self.subTest(target=target):
                self.assertEqual(driver.ground_step(dict(x=0,y=0,z=0),target),expected)


class AdmissionIntegrationTest(unittest.TestCase):
    def test_admitted_spell_selection_runs_after_caster_readiness(self):
        events=[]
        def unexpected_click(*args):
            self.fail('The supplied spell selector owns HUD admission')
        driver.prepare_summon(unexpected_click,lambda:events.append('center'),
            lambda *args:events.append('ready'),lambda *args:events.append('blocked'),
            owner=2,wizard_slot=7,spell_cases=False,wait=lambda _:None,
            spell_selector=lambda spell:events.append(('select',spell)))
        self.assertEqual(events,['center','ready',('select','zombie')])

    def test_refusal_precedes_admitted_positive_selection(self):
        events=[]
        driver.prepare_summon(lambda *args:None,lambda:events.append('center'),
            lambda *args:events.append('ready'),lambda *args:events.append('blocked'),
            owner=2,wizard_slot=7,spell_cases=True,wait=lambda _:None,
            spell_selector=lambda spell:events.append(('select',spell)))
        self.assertEqual(events,['center','ready','blocked',('select','zombie')])

    def actors(self):
        return {7:dict(slot=7,type=0,owner=2,active=1,health=86,x=0,y=0,z=0),
                9:dict(slot=9,type=10,owner=3,active=1,health=2,x=0,y=0,z=0)}

    def test_navigation_reads_positions_after_centering(self):
        actors=self.actors();events=[]
        def center():
            events.append('center');actors[9].update(x=10,y=-10)
        def state():
            events.append('read');return actors
        actor,enemy,point=driver.combat_approach(center,state,wizard_slot=7,owner=2)
        self.assertEqual(events,['center','read'])
        self.assertEqual((actor['slot'],enemy['slot'],point),(7,9,(570,300)))

    def test_close_combat_uses_fresh_tiles_instead_of_screen_calibration(self):
        actors=self.actors()
        actors[9].update(x=30,y=40)
        self.assertFalse(driver.combat_in_range(actors[7],actors[9]))
        def center():
            actors[7].update(x=100,y=100)
            actors[9].update(x=103,y=104)
        actor,enemy,point=driver.combat_approach(center,lambda:actors,wizard_slot=7,owner=2)
        # The returned screen point is hundreds of pixels away from (0,0),
        # while the observed actors are only five tiles apart.
        self.assertGreater(point[0],100)
        self.assertTrue(driver.combat_in_range(actor,enemy))
        actors[9].update(x=107,y=100)
        self.assertFalse(driver.combat_in_range(actor,actors[9]))

    def test_navigation_refuses_dead_or_replaced_wizard(self):
        for changes,message in [({'health':0},'died'),({'owner':3},'Expected active'),
                                ({'type':14},'Expected active'),({'active':0},'Expected active')]:
            actors=self.actors();actors[7].update(changes)
            with self.subTest(changes=changes),self.assertRaisesRegex(RuntimeError,message):
                driver.combat_approach(lambda:None,lambda:actors,wizard_slot=7,owner=2)

    def test_navigation_refuses_missing_living_enemy(self):
        actors=self.actors();actors[9]['health']=0
        with self.assertRaisesRegex(RuntimeError,'No living enemy'):
            driver.combat_approach(lambda:None,lambda:actors,wizard_slot=7,owner=2)


if __name__=='__main__':unittest.main()
