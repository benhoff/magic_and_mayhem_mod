#!/usr/bin/env python3
"""Compare original40a760 source center and429270 merge distance."""
from owner_planner_comparison import SUITES,main
SUITES['source-center']=dict(ranges=[(0x40a760,0x40a854),(0x429270,0x429306)],endpoints=[],scope='Whole40a760 ordered ordinary source centers with two/three actors and whole429270 between one/two-member sources. Raw-coordinate incremental truncated means, normalized positions and wrapped horizontal/full-height squared merge distance; bounded32x32x8 points, reversed traversal and seam cases. No redirects, full original image unchanged. Larger/special sources and World/live excluded.',suite='owner-source-center',sources=['tests/owner-source-center-test.cpp','reconstruction/creature-ai/source_decision.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/combat/melee.cpp'],wrapper='tools/test-owner-source-center.py')
if __name__=='__main__':main('source-center')
