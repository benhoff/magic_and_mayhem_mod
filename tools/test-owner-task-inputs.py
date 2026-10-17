#!/usr/bin/env python3
"""Compare original commander distress, special occupancy and task damage credit."""
from owner_planner_comparison import SUITES, main
SUITES['inputs'] = dict(
    ranges=[(0x52a3a0, 0x52a40d), (0x522690, 0x522717), (0x514990, 0x514c56)],
    endpoints=[0x54de10,0x51fd60,0x513e40,0x51fba0,0x56c290,0x570bd0,
               0x411e50,0x5137d0,0x4d9ac0,0x50aa30,0x522340,0x416d80,
               0x416c30,0x4656c0,0x466870,0x58f830,0x58f890,0x4d9890,0x4f3840,0x5075f0],
    scope='Whole52a3a0 commander distress with supplied maximum and pinned strict ratio>.5, whole522690 special occupancy with supplied point enumeration, and whole514990 nonlethal/zero/dead task-credit paths. Account416d80 and source task-ID resolution416c30 are explicit supplied endpoints; victim ancestry traversal is original. Wizard doubling, absent source recipients, absent/duplicate victim roots and parent traversal. No full health/credit numerical composition, notifications, resource economy, source topology changes or World scheduling.',
    suite='owner-task-inputs',
    sources=['tests/owner-task-inputs-test.cpp','reconstruction/creature-ai/owner_task_inputs.cpp',
             'reconstruction/combat/health.cpp'], wrapper='tools/test-owner-task-inputs.py')
if __name__ == '__main__':
    main('inputs')
