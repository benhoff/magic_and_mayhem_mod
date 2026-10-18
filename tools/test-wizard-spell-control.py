#!/usr/bin/env python3
"""Compare original wizard mana readiness with native and sanitizer execution."""
from owner_planner_comparison import SUITES, main

SUITES['wizard-spell-control'] = dict(
    ranges=[(0x51e530, 0x51e6c0)], endpoints=[0x412aa0],
    scope='Whole51e530 mana readiness: unsigned maximum admission, fixed-point/count DWORD arithmetic, signed deficit, all42 ordered inventory slots and same-owner eligible active production/ceil wait. Producer412aa0 supplied as a predicate, finite owned pool and configured rate/period; actor bytes unchanged. Native observed-peer availability projection tested separately, not whole40a950 or wholewizard/World/regen/inventory consumption equivalence.',
    suite='wizard-spell-control', sources=['tests/wizard-spell-control-test.cpp',
        'reconstruction/creature-ai/wizard_spell_control.cpp',
        'reconstruction/creature-ai/ownership_helpers.cpp'],
    wrapper='tools/test-wizard-spell-control.py')
if __name__ == '__main__':
    main('wizard-spell-control')
