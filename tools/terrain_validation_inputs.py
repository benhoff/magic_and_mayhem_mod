"""Select an isolated PE input without replacing the installed launcher baseline.

Every caller must retain its executable SHA-256 check and original-manifest
bookends. This changes input selection only, never executable bytes or admission.
"""
import os
from pathlib import Path


def terrain_executable(root):
    selected = os.environ.get('MNM_TERRAIN_EXECUTABLE')
    return Path(selected).resolve() if selected else root / 'working/game-nocd/Chaos.exe'
