#!/usr/bin/env python3
"""Compare original previous-task selection gates and lifetime draws."""
from owner_planner_comparison import SUITES, main
SUITES['replanning']=dict(ranges=[(0x4284e0,0x4289d7),(0x40a3b0,0x40a413),(0x40b3f0,0x40b41e)],endpoints=[],scope='Whole4284e0 with authored evaluator services and absent/present previous task, completion/weighted/value gates, relative improvement and age/progress rejection, count budgets, bias/reject-one/ties and RNG. Whole40a3b0 progress and40b3f0 mean100 lifetime draw. No text redirects. Full423590 driver, task creation and World cadence excluded from this component.',suite='owner-task-replanning',sources=['tests/owner-task-replanning-test.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/combat/melee.cpp'],wrapper='tools/test-owner-replanning.py')
if __name__=='__main__':main('replanning')
