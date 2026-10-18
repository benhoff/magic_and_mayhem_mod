#!/usr/bin/env python3
"""Execute the original normal World AI allocation call-site window."""
from owner_planner_comparison import SUITES,main
SUITES['world-allocation']=dict(ranges=[(0x46b5be,0x46b5e8)],endpoints=[0x415c40,0x46b5e8],scope='Original46b5be..46b5e8 normal World count allocation call-site window with authored nonnegative0..1000000 global allocation. Actual signed divide-by-two instruction sequences and ordered brain0/brain1 calls; caller415c40 and continuation at window end supplied as recorders. World admission/tick/timing/startup and whole46afc0 excluded.',suite='owner-world-allocation',sources=['tests/owner-world-allocation-test.cpp','reconstruction/creature-ai/owner_brain_driver.cpp'],wrapper='tools/test-owner-world-allocation.py')
if __name__=='__main__':main('world-allocation')
