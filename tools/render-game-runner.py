#!/usr/bin/env python3
"""Run a hash-checked render experiment inside the launcher's Wine desktop."""
import hashlib
import json
import os
from pathlib import Path
import sys
import subprocess
root=Path(os.environ['MNM_RENDER_EXPERIMENT']);metadata=json.loads((root/'manifest.json').read_text());game=root/'game'
binaries=[('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]
if metadata.get('audio_dll_sha256'):binaries.append(('MnmAudio.dll','audio_dll_sha256'))
for name,key in binaries:
    if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
os.chdir(game)
if metadata.get('capture_locks'):
    timing=root/'presentation-rate.jsonl'
    try:
        subprocess.Popen([sys.executable,str(Path(__file__).resolve().with_name('profile-render-stream.py')),
                          metadata['stream'],'--output',str(timing),'--parent-pid',str(os.getpid())],
                         stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,close_fds=True)
        print(f'Render timing: {timing}',flush=True)
    except OSError as error:
        print(f'Render timing monitor could not start: {error}',flush=True)
os.execvp('wine',['wine','explorer','/desktop=MagicMayhem,800x600',str(game/'Chaos.exe'),*sys.argv[2:]])
