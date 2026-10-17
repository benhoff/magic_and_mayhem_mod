#!/usr/bin/env python3
"""Compare bounded automatic original423590 search and selection."""
from owner_planner_comparison import SUITES, main
SUITES['source-decision'] = dict(
 ranges=[(0x423590,0x425c9e),(0x4284e0,0x4289d7)],
 endpoints=[0x597850,0x597870,0x42a340,0x4289e0,0x416fe0,0x42b7c0,0x418050,0x429270,0x523240],
 scope='Whole423590 reset through automatic decision/commit for exactly two ordinary friendly members, pinned proposal caps1, no current task and absent/completed other tasks with varied value/bias, supplied maximum/weighted mode, normal and split-first reset admission. Actual4284e0 and RNG; evaluator prepare/next/save supplies finite membership-dependent scores, allocation, task install/register/map teardown and one-candidate distance and zero net-danger523240 services supplied. Compare sliced budgets/stages/winners/saved members/full RNG. Native invalid domain refusal separate. Multi-root ordering beyond sole candidate, old healthy-task retention/progress gates, full task6 composition, World/live excluded.',
 suite='owner-source-decision',
 sources=['tests/owner-source-decision-test.cpp','reconstruction/creature-ai/source_decision.cpp','reconstruction/creature-ai/source_lifecycle.cpp','reconstruction/creature-ai/source_graph.cpp','reconstruction/creature-ai/source_pending.cpp','reconstruction/creature-ai/ownership_helpers.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/combat/melee.cpp'],
 wrapper='tools/test-owner-source-decision.py')
if __name__ == '__main__': main('source-decision')
