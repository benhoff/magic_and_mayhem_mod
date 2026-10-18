#!/usr/bin/env python3
"""Compare whole task6 two-member probe generation and candidate assembly."""
from owner_planner_comparison import SUITES, main
SUITES['three-member-probes'] = dict(
 ranges=[(0x431ce0,0x4328cc)],
 endpoints=[0x597850,0x597870,0x4766b0],
 scope='Whole431ce0 from a prepared one-enemy group with two ordinary friendly members in independently assigned regions, actual ordered source traversal/regional searches/feature assembly/scoring/lifetime RNG. Supplied finite regional graph, empty inventories/resources, physical actors, region parameter1, constant inference outputs and allocator. Compare per-slice budgets/results/full RNG and successful72 features, max travel, auxiliary/destination-danger sums, all-path risk and lifetime. Preparation/admission/memory group production/learned inference/full423590 composition/World/live excluded.',
 suite='owner-three-member-probes',
 sources=['tests/owner-three-member-probes-test.cpp','reconstruction/creature-ai/owner_group_features.cpp','reconstruction/creature-ai/owner_planner_features.cpp','reconstruction/creature-ai/owner_task_probe.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/creature-ai/regional_navigation.cpp','reconstruction/pathfinding/route_neighbors.cpp','reconstruction/pathfinding/route_search.cpp','reconstruction/combat/melee.cpp'],
 wrapper='tools/test-owner-three-member-probes.py')
if __name__ == '__main__':main('three-member-probes')
