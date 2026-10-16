"""Independent evidence checks for the resident campaign Save/Load journey."""
import hashlib
import json
import re
import struct
from pathlib import Path


def validate_mini_dispatch(flow, capture_delay_ms=0):
    requests=[s for s in flow.get('states',[]) if s.get('event')=='mini-request']
    expected=[29,28,15]+([15] if flow.get('recreate_world') else [])
    if ([r.get('action') for r in requests]!=expected
            or any(r.get('accepted') is not True or r.get('reason') or r.get('screen')!=17
                   or r.get('generation',0)<=0 for r in requests)):
        raise ValueError('Missing ordered accepted native Mini requests')
    clicks=[s for s in flow.get('states',[]) if s.get('event')=='click' and s.get('action','').startswith('miniMenuButton')]
    controls=['miniMenuButton2','miniMenuButton1','miniMenuButton4']+(['miniMenuButton4'] if flow.get('recreate_world') else [])
    if [s['action'] for s in clicks]!=controls:
        raise ValueError('Missing Mini diagnostic capture ordering')
    for request,capture in zip(requests,clicks):
        if (not capture['time_ms']<=request['time_ms']<=capture['capture_begin_ms']<=capture['capture_end_ms']
                or capture.get('injected_capture_delay_ms')!=capture_delay_ms
                or capture.get('capture_duration_ms',0)<capture_delay_ms):
            raise ValueError('Mini request was not submitted before delayed capture')
    return dict(verified=True,requests=len(requests),capture_delay_ms=capture_delay_ms,
                scope='Native host acceptance before diagnostic capture; original callback ACKs checked independently')


def validate_mini_inputs(directory, flow=None):
    records=[]
    prescribed={'select-spell':('select zombie spell',[523,575],1),'summon':('summon zombie',[400,280],3),
                'select-creature':('select zombie',[400,280],1),'move-creature':('move zombie',[400,280],3),'dismiss':('hermes',[400,300],1)}
    for path in sorted(directory.glob('storage-input-*.json')):
        record=json.loads(path.read_text())
        if record.get('mode')!='mini':continue
        if record.get('success') is not True or record.get('skip_click') is not True:
            raise ValueError('Mini ingress contains an unclassified focus click')
        buttons=[e for e in record.get('events',[]) if e.get('event')=='button']
        titles={' '.join(o.get('text','').lower().split()) for o in record.get('tutorial_observations',[]) if o.get('ocr_returncode')==0}
        for observation in record.get('tutorial_observations',[]):
            for kind in ('image','crop'):
                name=observation[kind]
                if Path(name).name!=name or not name.endswith('.png') or hashlib.sha256((directory/name).read_bytes()).hexdigest()!=observation[kind+'_sha256']:
                    raise ValueError('Mini title image changed after observation')
        for action in record.get('tutorial_actions',[]):
            rule=prescribed.get(action.get('action'))
            if rule is None or rule[0] not in titles or (action['logical_point'],action['button'])!=(rule[1],rule[2]):
                raise ValueError('Mini mouse action lacks its recognized visible title')
        expected=[(a['button'],down,a['logical_point']) for a in record.get('tutorial_actions',[]) for down in (True,False)]
        if [(e['button'],e['down'],e['point']) for e in buttons]!=expected:
            raise ValueError('Mini ingress mouse events differ from prescribed tutorial actions')
        escape=[e.get('down') for e in record.get('events',[]) if e.get('event')=='escape']
        if buttons and escape:raise ValueError('Mini ingress combined a mouse action and Escape')
        if escape!=([True,False] if record.get('escape_sent') else []):
            raise ValueError('Missing ordered physical Escape edges')
        records.append(dict(path=path.name,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),escape=bool(escape),tutorial_actions=len(expected)//2))
    if not records or not any(r['escape'] for r in records):raise ValueError('Missing physical Mini ingress')
    if flow is not None:
        previous=0;observed=[]
        for event in flow.get('states',[]):
            if event.get('event')!='mini-input':continue
            source=int(event['source_generation'])
            if source<=previous:raise ValueError('Mini input reused a World source generation')
            previous=source;observed.append(event['record'])
        if len(observed)!=len(records) or set(observed)!={r['path'] for r in records}:
            raise ValueError('Missing World-bound Mini input observation')
    return dict(verified=True,records=records,scope='Only recognized Hermes dismissal or prescribed tutorial mouse actions and Escape; input channel observations do not acknowledge original consumption')


def validate_storage_flow(flow):
    if (flow.get('success') is not True or flow.get('save_load') is not True
            or flow.get('stage') != 12 or flow.get('normal_quit') is not True
            or flow.get('resident_world_requested') is not True
            or flow.get('native_command_fallback') is not False
            or flow.get('native_recoveries') != 0 or flow.get('native_error')
            or flow.get('native_command_frames', 0) < 30):
        raise ValueError('Incomplete resident Save/Load flow')
    checkpoints = flow.get('checkpoints', [])
    phases=['before-save', 'after-save', 'after-load']
    if flow.get('recreate_world'):
        phases.append('after-recreate-load')
        if flow.get('display_rebuilds')!=2 or flow.get('reloaded_from_main') is not True:
            raise ValueError('Missing original display rebuild and Main Load')
    if [c.get('phase') for c in checkpoints] != phases:
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


def validate_opening_inputs(directory):
    order=['select-spell','summon','select-creature','move-creature']
    progress=[];first_escape=False;records=[]
    for path in sorted(directory.glob('storage-input-*.json'),key=lambda p:int(p.stem.rsplit('-',1)[1])):
        record=json.loads(path.read_text())
        if record.get('success') is not True:raise ValueError('Failed tutorial input helper')
        if record.get('mode')!='mini' or first_escape:continue
        for action in record.get('tutorial_actions',[]):
            name=action['action']
            if len(progress)<4 and name==order[len(progress)]:progress.append(name)
        if record.get('escape_sent'):
            if progress!=order or record.get('opening_progress')!=order:
                raise ValueError('Initiate entered Mini before completing the opening commands')
            first_escape=True
        records.append(dict(path=path.name,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),actions=record.get('tutorial_actions',[])))
    if not first_escape:raise ValueError('Missing Initiate opening tutorial input proof')
    return dict(verified=True,commands=progress,records=records)


def validate_storage(experiment, flow, difficulty=1, input_directory=None):
    if type(difficulty) is not int or difficulty not in (0, 1):
        raise ValueError('Unsupported storage journey difficulty')
    tutorial=validate_opening_inputs(input_directory) if difficulty==0 and input_directory else None
    if difficulty==0 and tutorial is None:raise ValueError('Initiate requires opening tutorial evidence')
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
    expected = [(3, 0)] + ([(18, 65536+difficulty)] if difficulty else []) + [(18, 0), (17, 1), (13, 0), (17, 0), (12, 0), (17, 3), (6, 21), (3, 4)]
    if flow.get('recreate_world'):
        expected=expected[:-1]+[(3,3),(10,0),(3,3),(10,0),(3,1),(12,0),(17,3),(6,21),(3,4)]
        display=[r for r in rows if r[1] in (6,7,8)]
        if [r[1] for r in display]!=[6,7,8]*2 or [(r[9],r[10]) for r in display if r[1]==7]!=[(640,480),(800,600)]:
            raise ValueError('Original display rebuild dimensions were not observed')
    if actions != expected or len({r[2] for r in rows if r[1] in (3, 12, 20)}) != 1:
        raise ValueError('Unexpected original Save/Load callback/thread trace: ' + repr(actions))
    # A returned storage callback and new original World event must precede
    # the next Mini action. Merely seeing either event somewhere cannot pass.
    offset = int(bool(difficulty))
    storage_actions=[(3+offset, 13), (5+offset, 12)]
    if flow.get('recreate_world'):storage_actions.append((len(callbacks)-4,12))
    for index, screen in storage_actions:
        begin, end = callbacks[index][0], callbacks[index+1][0]
        if not any(r[1] == 4 and r[3] == screen and r[8] == 1 and begin < r[0] < end for r in rows):
            raise ValueError('Original storage callback did not return successfully')
        if flow.get('recreate_world') and index==len(callbacks)-4:
            # Main Load initializes World; it does not call the Mini resume
            # method. Require its Main parent and a subsequent original active
            # campaign Mini in addition to the fresh fourth World checkpoint.
            if callbacks[index][12]!=3 or not any(s.get('stage')==8 and s.get('screen')==12 and s.get('reloaded') is True and s.get('parent')==3 and s.get('depth')==3 for s in flow.get('states',[])):
                raise ValueError('Main Load did not have its original Main parent')
            if not any(r[1] in (13,16) and r[3]==17 and r[4]==0x6a5088 and r[5]==0x5c6644 and r[6]==1 and r[9]==2 and r[10]==5 and r[11]==r[4] and r[12]==5 and begin<r[0]<end for r in rows):
                raise ValueError('Original campaign Mini did not reopen after Main Load')
        elif not any(r[1] in (12, 20) and r[3] == 2 and r[6] == 1 and begin < r[0] < end for r in rows):
            raise ValueError('Original World did not resume after storage action')
    answers = [r for r in rows if r[1] == 22]
    quits=[6+offset]+([len(callbacks)-3] if flow.get('recreate_world') else [])
    if len(answers) != len(quits) or any(answer[9]!=0 or not callbacks[index][0]<answer[0]<callbacks[index+1][0] for answer,index in zip(answers,quits)):
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
    return dict(verified=True, difficulty=difficulty, tutorial=tutorial, original_callbacks=actions, checkpoints=checkpoints,
                saved_sha256=digest, save_bytes=saved.stat().st_size,
                owned_save_persisted=True,
                events_sha256=hashlib.sha256(data).hexdigest(),
                scope='Same-session original Save/Load, fresh native World generations and normal Quit; full restored simulation and whole-session pixel equivalence are separate')
