#!/usr/bin/env python3
"""Run a hash-checked render experiment inside the launcher's Wine desktop."""
import hashlib
import json
import os
from pathlib import Path
import sys
root=Path(os.environ['MNM_RENDER_EXPERIMENT']);metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
binaries=[('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]
if metadata.get('audio_dll_sha256'):binaries.append(('MnmAudio.dll','audio_dll_sha256'))
for name,key in binaries:
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
os.chdir(game)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
