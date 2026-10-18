#!/usr/bin/env python3
"""Read-only discovery of pinned owner visibility production and consumers."""
from importlib.machinery import SourceFileLoader
from pathlib import Path

module = SourceFileLoader('visibility_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
    'sighting': (0x508ca0, 0x508e6c),
    'decay': (0x508e70, 0x508ed5),
    'pool-perception': (0x52b540, 0x52b87d),
    'vision-cast': (0x56a8d0, 0x56b1dc),
    'world-driver': (0x494ab0, 0x496f01),
    'creature-driver': (0x4f8960, 0x4fbcb1),
    'geometric-sight': (0x515270, 0x515344),
    'share-query': (0x52b880, 0x52b89c),
    'observation-points': (0x52b8a0, 0x52baef),
    'sight-radius': (0x5150a0, 0x5151ab),
    'fine-facing': (0x5084f0, 0x50880e),
    'point-sight': (0x5151b0, 0x515261),
    'profile-center': (0x50343b, 0x503467),
    'sight-ray': (0x4e14a0, 0x4e1738),
}
if __name__ == '__main__':
    module.main()
