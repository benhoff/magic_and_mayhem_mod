#!/usr/bin/env python3
"""Compare original actor-target wizard spell task bodies."""
from owner_planner_comparison import SUITES, main

SUITES['wizard-target-task'] = dict(
    ranges=[(0x4466f0, 0x446816), (0x45d2b0, 0x45d3e0),
            (0x44f9d0, 0x44faf0), (0x450780, 0x4508b0),
            (0x44c560, 0x44c6a0), (0x4500c0, 0x4501f0),
            (0x450ee0, 0x451010), (0x45bb60, 0x45bc90)],
    endpoints=[0x5075f0, 0x51fef0, 0x51e530, 0x51d570, 0x40ad00, 0x57c6e0],
    scope='Whole actor-target wizard task execution for spells41/42/74/78/79/83/84/85: alive/max-health/status gates, issued latch, wait/inventory shortage, reread health, cast positioning and semantic descriptor words0/7..11. Supplied pure maximum/status projection and wait/inventory/positioning/cast services; directed lower mutations. Full preparation/candidate scoring/lifetime/geometry/effects/World remain separate.',
    suite='wizard-target-task', sources=['tests/wizard-target-task-test.cpp',
        'reconstruction/creature-ai/wizard_target_task.cpp'],
    wrapper='tools/test-wizard-target-task.py')
if __name__ == '__main__':
    main('wizard-target-task')
