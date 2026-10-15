#!/usr/bin/env python3
"""Read-only discovery of the pinned original navigation graph construction."""
from importlib.machinery import SourceFileLoader
from pathlib import Path
import json
import sys
module = SourceFileLoader('regional_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
 'constructor': (0x53a660, 0x53a834), 'destructor': (0x53a840, 0x53aa29),
 'reset': (0x53aa30, 0x53aac2), 'build': (0x53ab20, 0x53b600),
 'connections': (0x53b600, 0x53baf4), 'clearance': (0x53bb40, 0x53c070),
 'helper-c070': (0x53c070, 0x53c2e7), 'helper-c320': (0x53c320, 0x53cd28),
 'helper-cd30': (0x53cd30, 0x53ce4d), 'helper-ce50': (0x53ce50, 0x53d6bb),
 'sort': (0x53d710, 0x53d7a9),
 'world-construction': (0x470950, 0x470d00),
 'terrain-admission': (0x4f3440, 0x4f3500),
 'direct-route': (0x542220, 0x542600),
 'sort-compare': (0x53d6f0, 0x53d710)}
if __name__ == '__main__':
 module.main()
 # The shared exporter records its implementation; retain this range selection too.
 output = Path(sys.argv[sys.argv.index('--output')+1]) / 'report.json'
 report = json.loads(output.read_text())
 report['sources']['tools/export-regional-graph.py'] = module.sha(Path(__file__))
 output.write_text(json.dumps(report,indent=2)+'\n')
