#!/usr/bin/env python3
"""Compare the original Magic Sphere wizard task execution body."""
from owner_planner_comparison import SUITES, main
SUITES['wizard-projectile-task'] = dict(
    ranges=[(0x44ae70, 0x44b07e)],
    endpoints=[0x51e530, 0x51d570, 0x40ad00, 0x57c6e0, 0x520b30],
    scope='Whole44ae70 MagicSphere73 execution: dead-target saved-command restoration, issued/readiness/inventory gates, exact stand/action admission, positioning, semantic cast descriptor words0/7..11 and ordered normalized saved command. Supplied wait/inventory/positioning/cast/saved-command services, directed health/mana mutations; candidate prediction/scoring, lifetime and full lower effects/World separate.',
    suite='wizard-projectile-task', sources=['tests/wizard-projectile-task-test.cpp',
        'reconstruction/creature-ai/wizard_projectile_task.cpp',
        'reconstruction/creature-ai/wizard_target_task.cpp'],
    wrapper='tools/test-wizard-projectile-task.py')
if __name__ == '__main__':
    main('wizard-projectile-task')
