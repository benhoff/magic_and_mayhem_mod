#!/usr/bin/env python3
"""Compare original source detachment and task-protected source maintenance."""
from owner_planner_comparison import SUITES, main
SUITES['source-lifecycle'] = dict(
 ranges=[(0x423590,0x425c9e),(0x407780,0x407819),(0x407820,0x407872),(0x407650,0x407707),(0x417580,0x417e51),(0x417e60,0x417e7e)],
 endpoints=[0x597850,0x597870,0x42a340,0x4289e0,0x416fe0,0x42b7c0,0x418050],
 scope='Whole407780/407820 with occupied source cursors; whole417580 pruning with task/current/other ancestor protection, actual407650/417e60 destruction and original deque/vector erase. Authored finite forest, three actors, one supplied block per source queue; allocation/append/task installation/registration/map teardown are explicit services. Selected423590 split/merge commit projection starts at completed stage5 with authored winning proposal, two ordinary friendly members, no task or invalidated old tasks, supplied task-map erase/install/register/destruction and preallocated append storage; topology, displaced retirement order and winning/remainder source identities compared. full proposal generation, World cadence, training and live replacement excluded.',
 suite='owner-source-lifecycle',
 sources=['tests/owner-source-lifecycle-test.cpp','reconstruction/creature-ai/source_lifecycle.cpp','reconstruction/creature-ai/source_graph.cpp','reconstruction/creature-ai/source_pending.cpp','reconstruction/creature-ai/ownership_helpers.cpp'],
 wrapper='tools/test-owner-source-lifecycle.py')
if __name__ == '__main__': main('source-lifecycle')
