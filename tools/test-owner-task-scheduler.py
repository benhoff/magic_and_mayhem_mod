#!/usr/bin/env python3
"""Ordinary415c40 count-mode caller with real task6 planning and execution."""
from owner_planner_comparison import SUITES, main
SUITES['task-scheduler'] = dict(
    ranges=[(0x432fb0,0x432fb1),(0x415c40,0x416523),(0x428b40,0x428e32),(0x4075b0,0x407638),(0x423590,0x425c9e),(0x4284e0,0x4289d7),(0x431be0,0x4328cc),(0x4261b0,0x426310),(0x4289e0,0x428b30),(0x4766b0,0x476b33),(0x432900,0x432fb0),(0x514990,0x514c20)],
    endpoints=[0x4181d0,0x417580,0x428e40,0x428f80,0x416fe0,0x416d80,0x597850,0x597870,0x418050,0x476e30,0x50af10,0x50fd50,0x50fef0],
    scope='Whole415c40 ordinary single direct-root source, count-budget mode with authored nonnegative budgets; actual423590/task6 generation/inference/save/install,428b40 retirement, real task-map queue copying/erasure and432900 execution. Source queue insertion, source maintenance, world snapshot swapping and perception are explicit services; clone/teardown/training/accounting endpoints retain NS82 bounds. No split/merge, simultaneous displaced tasks, specialized queues, dual brains, rdtsc or World allocation producer, wizard/campaign factories or live replacement. Distinct execution+14 and replacement+15 flags, partial planning, independent execution budget, expiry and completion cleanup, stable supplied observations and full RNG per call.',
    suite='owner-task-scheduler',
    sources=['tests/owner-task-scheduler-test.cpp','reconstruction/creature-ai/owner_task_scheduler.cpp','reconstruction/creature-ai/owner_group_task.cpp','reconstruction/creature-ai/owner_group_features.cpp','reconstruction/creature-ai/owner_planner_features.cpp','reconstruction/creature-ai/owner_memory_groups.cpp','reconstruction/creature-ai/owner_task_probe.cpp','reconstruction/creature-ai/regional_navigation.cpp','reconstruction/creature-ai/regional_danger.cpp','reconstruction/pathfinding/route_neighbors.cpp','reconstruction/pathfinding/route_search.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_task_inputs.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/combat/melee.cpp','reconstruction/creature-ai/brain_inference.cpp','assets/dat.cpp'],
    wrapper='tools/test-owner-task-scheduler.py')
if __name__ == '__main__':
    main('task-scheduler')
