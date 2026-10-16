#!/usr/bin/env python3
"""Read-only discovery of original owner observation and memory production."""
from importlib.machinery import SourceFileLoader
from pathlib import Path

module = SourceFileLoader('knowledge_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
    'observation-driver': (0x4181d0, 0x4189fe),
    'knowledge-construction': (0x4284e0, 0x4289d7),
    'knowledge-event': (0x4c02a0, 0x4c0541),
    'knowledge-lifecycle': (0x4c0920, 0x4c0b62),
    'observation-mode': (0x418a90, 0x418b1c),
    'snapshot-construction': (0x418b60, 0x418cc8),
    'snapshot-copy': (0x418d50, 0x4198fb),
    'live-predicate': (0x412aa0, 0x412aae),
    'live-iterator': (0x52a670, 0x52a6b8),
    'filtered-iterator': (0x52a710, 0x52a773),
    'distance': (0x40a710, 0x40a758),
    'brain-reset': (0x4156b0, 0x415abb),
    'snapshot-destructor': (0x505f70, 0x50607c),
    'memory-insert': (0x42a5d0, 0x42a6d8),
    'memory-erase': (0x42a6e0, 0x42ab4e),
    'owner-construction': (0x4c0d50, 0x4c0e24),
}
if __name__ == '__main__':
    module.main()
