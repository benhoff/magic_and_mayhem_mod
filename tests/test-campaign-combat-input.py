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


if __name__=='__main__':unittest.main()
