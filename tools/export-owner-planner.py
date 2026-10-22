#!/usr/bin/env python3
"""Read-only discovery of the pinned owner's strategic task pipeline."""
from coverage_evidence import read_text as evidence_text
from importlib.machinery import SourceFileLoader
from pathlib import Path

module = SourceFileLoader('planner_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
    'brain-driver': (0x415c40, 0x416523),
    'source-controller': (0x416590, 0x416b7e),
    'task-helper-a': (0x416b80, 0x416c28),
    'task-helper-b': (0x416c30, 0x416c95),
    'task-helper-c': (0x416ca0, 0x416d72),
    'sighting-value': (0x416d80, 0x416ea0),
    'sighting-driver': (0x4181d0, 0x4189fe),
    'task-pool-lookup': (0x522770, 0x52279a),
    'notification-begin': (0x407920, 0x407982),
    'notification-next': (0x4079f0, 0x407a73),
}
# Task construction, ordering and dispatch; preserve unknown indirect families.
import json
_inventory = json.loads(evidence_text(Path(__file__).resolve().parents[1] / 'research/runtime/coverage/binary-nocd-owner-planner-20261016.json'))
for _f in _inventory['functions']:
    _a = _f['entry']
    if 0x416ea0 <= _a < 0x4181d0 or 0x407920 <= _a <= 0x407b10 or _a in (0x4284e0,0x421f70, 0x423590, 0x428b40, 0x428e40, 0x428f80):
        module.RANGES[f'task-{_a:x}'] = (_a, max(_r[1] for _r in _f['ranges']))

for _f in _inventory['functions']:
    if _f['entry'] in (0x419e40, 0x41b100, 0x426850, 0x426c80, 0x4274f0, 0x427990, 0x427e00, 0x428210, 0x4343b0, 0x435a90, 0x436900, 0x437650, 0x438b90, 0x439b40, 0x43b260, 0x43b3f0, 0x43c390, 0x43d410, 0x4422a0):
        module.RANGES[f'evaluator-{_f["entry"]:x}'] = (_f['entry'], max(_r[1] for _r in _f['ranges']))

module.RANGES.update({'kind13-next': (0x43b4c0,0x43c390), 'kind13-execute': (0x43c3c0,0x43c600), 'kind12-next': (0x43a000,0x43b260), 'kind12-prepare': (0x439e80,0x43a000), 'kind12-execute': (0x43b290,0x43b3e0), 'fallback-prepare': (0x415640,0x415670), 'fallback-next': (0x415670,0x4156a0), 'fallback-execute': (0x419900,0x419910), 'kind13-prepare': (0x43b3f0,0x43b4c0), 'kind13-lifetime': (0x43c600,0x43c610)})

module.RANGES.update({'kind6-prepare':(0x431be0,0x431ce0),'kind6-next':(0x431ce0,0x4328d0),'kind6-save':(0x4261b0,0x426310),'kind6-execute':(0x432900,0x432fb0),'kind7-prepare':(0x432fc0,0x4330c0),'kind7-next':(0x4330c0,0x433ca0),'kind7-execute':(0x433cd0,0x434370),'kind10-prepare':(0x4378d0,0x4379d0),'kind10-next':(0x4379d0,0x4389d0),'kind10-execute':(0x438a00,0x438b80)})

for _f in _inventory['functions']:
    if _f['entry'] in (0x413330, 0x414ff0, 0x40b3f0, 0x40b420, 0x4766b0, 0x40edb0, 0x40eff0, 0x40eed0, 0x40aa40, 0x40aa90, 0x40aae0, 0x40ab60, 0x40abd0, 0x407c10):
        module.RANGES[f'planner-service-{_f["entry"]:x}'] = (_f['entry'], max(_r[1] for _r in _f['ranges']))

for _f in _inventory['functions']:
    if _f['entry'] in (0x47cdc0, 0x47d000, 0x40e680, 0x520620, 0x414fe0, 0x407c10, 0x40f7f0, 0x4078f0, 0x4078a0, 0x40fc40, 0x50af10, 0x5075f0, 0x51e500, 0x41b010):
        module.RANGES[f'planner-input-{_f["entry"]:x}'] = (_f['entry'], max(_r[1] for _r in _f['ranges']))
module.RANGES['memory-target-order'] = (0x50ad10,0x50af02)
if __name__ == '__main__':
    import sys
    if '--focus' in sys.argv:
        _i=sys.argv.index('--focus')
        _entries={int(x,16) for x in sys.argv[_i+1].split(',')}
        del sys.argv[_i:_i+2]
        for _f in _inventory['functions']:
            if _f['entry'] in _entries and _f['entry'] not in {v[0] for v in module.RANGES.values()}:
                module.RANGES[f'focus-{_f["entry"]:x}']=(_f['entry'],max(_r[1] for _r in _f['ranges']))
        module.RANGES={k:v for k,v in module.RANGES.items() if v[0] in _entries}
        if {v[0] for v in module.RANGES.values()} != _entries:
            raise ValueError('Unknown focus entry')
    module.main()
