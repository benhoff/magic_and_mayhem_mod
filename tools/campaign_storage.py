"""Independent evidence checks for the resident campaign Save/Load journey."""
import hashlib
import json
import re
import struct


def validate_storage_flow(flow):
    if (flow.get('success') is not True or flow.get('save_load') is not True
            or flow.get('stage') != 12 or flow.get('normal_quit') is not True
            or flow.get('resident_world_requested') is not True
            or flow.get('native_command_fallback') is not False
            or flow.get('native_recoveries') != 0 or flow.get('native_error')
            or flow.get('native_command_frames', 0) < 30):
        raise ValueError('Incomplete resident Save/Load flow')
    checkpoints = flow.get('checkpoints', [])
    if [c.get('phase') for c in checkpoints] != ['before-save', 'after-save', 'after-load']:
        raise ValueError('Missing ordered Save/Load World checkpoints')
    previous = 0
    for c in checkpoints:
        generation, baseline = int(c['source_generation']), int(c['source_base'])
        if (not c['native_active'] or c['fallback'] or c['error']
                or generation < baseline + 3 or generation <= previous
                or c['frames'] <= c['frame_base'] + 10):
            raise ValueError('Save/Load checkpoint reused stale World presentation')
        previous = generation
    return checkpoints


def validate_storage(experiment, flow):
    checkpoints = validate_storage_flow(flow)
    lines=(experiment/'lock-capture/lifecycle.log').read_text().splitlines()
    synchronization=[line.split() for line in lines if line.startswith('resident_storage_materialized ')]
    if [int(row[4],16) for row in synchronization] != [1,0] or any(int(row[5],16)<1 for row in synchronization):
        raise ValueError('Missing ordered native Save/Load synchronization boundaries')
    data = (experiment / 'events.bin').read_bytes()
    if data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64) or (len(data)-16) % 64:
        raise ValueError('Invalid original storage callback stream')
    rows = list(struct.iter_unpack('<16I', data[16:]))
    if not rows or len(rows) > 256 or [r[0] for r in rows] != list(range(1, len(rows)+1)):
        raise ValueError('Invalid original storage callback sequence')
    callbacks = [r for r in rows if r[1] == 3]
    actions = [(r[3], r[9]) for r in callbacks]
    expected = [(3, 0), (18, 65537), (18, 0), (17, 1), (13, 0), (17, 0), (12, 0), (17, 3), (6, 21), (3, 4)]
    if actions != expected or len({r[2] for r in rows if r[1] in (3, 12, 20)}) != 1:
        raise ValueError('Unexpected original Save/Load callback/thread trace: ' + repr(actions))
    # A returned storage callback and new original World event must precede
    # the next Mini action. Merely seeing either event somewhere cannot pass.
    for index, screen in ((4, 13), (6, 12)):
        begin, end = callbacks[index][0], callbacks[index+1][0]
        if not any(r[1] == 4 and r[3] == screen and r[8] == 1 and begin < r[0] < end for r in rows):
            raise ValueError('Original storage callback did not return successfully')
        if not any(r[1] in (12, 20) and r[3] == 2 and r[6] == 1 and begin < r[0] < end for r in rows):
            raise ValueError('Original World did not resume after storage action')
    answers = [r for r in rows if r[1] == 22]
    if len(answers) != 1 or answers[0][9] != 0 or not callbacks[7][0] < answers[0][0] < callbacks[8][0]:
        raise ValueError('Missing original Quit Yes confirmation')
    name = flow['saved_name']
    if not re.fullmatch(r'native-render-[a-f0-9]{12}', name):
        raise ValueError('Unexpected isolated save name')
    saved = experiment / 'game/Save' / (name + '.sav')
    digest = hashlib.sha256(saved.read_bytes()).hexdigest()
    if not saved.stat().st_size or digest != flow['saved_sha256']:
        raise ValueError('Save changed after Load')
    metadata = json.loads((experiment / 'manifest.json').read_text())
    merged = json.loads((experiment / 'save-store-result.json').read_text())
    owned = type(experiment)(metadata['save_store']['store']) / (name + '.sav')
    if not merged.get('merged') or hashlib.sha256(owned.read_bytes()).hexdigest() != digest:
        raise ValueError('Normal Quit did not persist the owned save')
    return dict(verified=True, original_callbacks=actions, checkpoints=checkpoints,
                saved_sha256=digest, save_bytes=saved.stat().st_size,
                owned_save_persisted=True,
                events_sha256=hashlib.sha256(data).hexdigest(),
                scope='Same-session original Save/Load, fresh native World generations and normal Quit; full restored simulation and whole-session pixel equivalence are separate')
