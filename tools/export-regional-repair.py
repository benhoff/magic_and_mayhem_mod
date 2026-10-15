#!/usr/bin/env python3
"""Read-only export of the pinned dynamic navigation update boundary."""
from importlib.machinery import SourceFileLoader
from pathlib import Path
import json
import sys

module = SourceFileLoader('regional_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
    'update': (0x53f710, 0x53f980),
    'component-reset': (0x53ee10, 0x53ef80),
    'component-lookup': (0x53f980, 0x53f9fa),
    'boundary-lookup': (0x53fa30, 0x53fad2),
    'component-build': (0x53fbc0, 0x540130),
    'terrain-edit-a': (0x4eb040, 0x4eb480),
    'terrain-edit-b': (0x4eb490, 0x4eb890),
    'terrain-edit-c': (0x4eb8a0, 0x4ebad2),
    'terrain-replace': (0x4f35d0, 0x4f37b0),
    'terrain-signature': (0x4f49e0, 0x4f4b30),
    'queue-priority': (0x4f4b30, 0x4f4b9a),
    'enqueue': (0x540430, 0x54049a),
    'terrain-notify': (0x53faf0, 0x53fbb9),
    'nearby-enqueue': (0x540140, 0x540429),
    'connection-step': (0x540820, 0x541662),
    'clearance-step': (0x5416d0, 0x5421e2),
    'world-update': (0x46b700, 0x46b800),
    'update-step': (0x5404a0, 0x540814),
    'local-connections': (0x53c070, 0x53c2e7),
    'connections': (0x53bb40, 0x53c070),
}
if __name__ == '__main__':
    if '--output' in sys.argv:
        at = sys.argv.index('--output') + 1
        sys.argv[at] = str(Path(sys.argv[at]).resolve())
    module.main()
    output = Path(sys.argv[sys.argv.index('--output') + 1]) / 'report.json'
    report = json.loads(output.read_text())
    report['sources']['tools/export-regional-repair.py'] = module.sha(Path(__file__))
    output.write_text(json.dumps(report, indent=2) + '\n')
