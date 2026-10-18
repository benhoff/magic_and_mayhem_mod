#!/usr/bin/env python3
"""Compare wizard ANI footstep-compatible ground bank production."""
from owner_planner_comparison import SUITES, main
SUITES['wizard-ground-profile'] = dict(
    ranges=[(0x505220, 0x50559f), (0x505160, 0x505216)], endpoints=[],
    scope='Whole505220/505160 selected ground-only type0 wizard ANI two-bank production and maximum. Installed version5 first-eight programs contain12 sprites, footstep events5, event2, repeated first sprite and stop. Endpoint-preserving translations; complete48 words/max compared, no text redirects. Original full CFG/asset selection, flight, gameplay motion and native World separate.',
    suite='wizard-ground-profile', include_dirs=['reconstruction/pathfinding', 'reconstruction/motion'], sources=['tests/wizard-ground-profile-test.cpp',
        'reconstruction/motion/creature_profile.cpp', 'assets/animation_decode.cpp',
        'reconstruction/pathfinding/route_scalar.cpp', 'reconstruction/pathfinding/route_search.cpp'],
    wrapper='tools/test-wizard-ground-profile.py')
if __name__ == '__main__':
    main('wizard-ground-profile')
