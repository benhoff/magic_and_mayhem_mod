#!/usr/bin/env python3
"""Hash-checked staged game adapter for run-game.sh --runner."""
import hashlib
import json
import os
from pathlib import Path
import sys
root=Path(os.environ['MNM_SHADOW_EXPERIMENT']).resolve()
metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
for file,key in [('Chaos.exe','staged_sha256'),('MnmShadow.dll','dll_sha256')]:
    if hashlib.sha256((game/file).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'Staged {file} hash mismatch')
os.chdir(game)
os.execvp('wine',['wine',str(game/'Chaos.exe'),*sys.argv[2:]])
