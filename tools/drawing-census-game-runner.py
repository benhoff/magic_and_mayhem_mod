#!/usr/bin/env python3
"""Authorize a fresh DRAW-01 experiment through the repository game launcher."""
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
from drawing_api_cohorts import cohort

root = Path(os.environ['MNM_DRAW_CENSUS_EXPERIMENT']).resolve()
metadata = json.loads((root / 'manifest.json').read_text()); game = root / 'game'
if metadata['origin'] != 'drawing_census_observation': raise ValueError('Unsupported experiment')
selected=cohort(metadata)
expected = {'Chaos.exe', 'MnmDrawingCensus.dll', 'MnmMenu.dll'}
if metadata.get('api_observation_enabled', False):
    expected.add('MnmDrawingApiCensus.dll')
    if json.loads((root/'api-build.json').read_text()).get('cohort','core_api')!=selected:
        raise ValueError('Staged API plan/observation cohort mismatch')
if set(metadata['binaries']) != expected: raise ValueError('Missing/unreviewed staged binary identities')
for name, digest in metadata['binaries'].items():
    if hashlib.sha256((game / name).read_bytes()).hexdigest() != digest: raise ValueError('Staged binary changed: ' + name)
capture = root / 'capture'
if capture.is_symlink() or sorted(p.name for p in capture.iterdir()) != ['census-control.bin']:
    raise ValueError('Census directory must be fresh and owned')
control = (capture / 'census-control.bin').read_bytes()
channel = (root / 'channel.bin').read_bytes()
if len(control) != 64 or control[:16] != b'MNMDCTL1' + struct.pack('<II', 1, 64): raise ValueError('Invalid census control')
if len(channel) != 106496 or channel[:16] != b'MNMMCM12' + struct.pack('<II', 12, 106496): raise ValueError('Invalid menu control')
for key in list(os.environ):
    if key.startswith('MNM_'): os.environ.pop(key)
def windows(path): return 'Z:' + str(path).replace('/', '\\')
os.environ.update(MNM_DRAW_CENSUS_DIR=windows(capture), MNM_MENU_OBSERVE=windows(root / 'events.bin'),
                  MNM_MENU_CHANNEL=windows(root / 'channel.bin'))
if metadata.get('api_observation_enabled', False): os.environ['MNM_DRAW_API_DIR'] = windows(capture)
if selected=='api_only':os.environ.pop('MNM_DRAW_CENSUS_DIR')
if metadata.get('gameplay_observation_enabled', False):
    os.environ.update(MNM_MENU_GAMEPLAY_OBSERVE=windows(root / 'gameplay-events.bin'),
                      MNM_MENU_SPELL_OBSERVE=windows(root / 'spell-events.bin'))
os.chdir(game)
os.execvp('wine', ['wine', 'explorer', '/desktop=DrawingCensus,800x600', str(game / 'Chaos.exe'), *sys.argv[2:]])
