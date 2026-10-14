#!/usr/bin/env python3
"""Require native menus and native command presentation in one public-shell campaign.

A rejected mode, menu retirement, original-window fallback, missing native frames,
or incomplete click-through is a failed test. --check-only builds the public shell
and probes the combined flags without launching a game or reading game media.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import re
import struct
import subprocess
import time
from campaign_gameplay import read_rows, creatures, damage_rows, player_lethal_rows
from campaign_movement import validate_movement_probes
from campaign_crowded import validate_crowded
from campaign_spell import player_engaged_fireball_lethal, read_spell_rows, validate_spell_observation, validate_spell_cases, player_cure_healing
from native_audio_launch import validate_activity as validate_audio_activity, validate_retirement as validate_audio_retirement

ROOT = Path(__file__).resolve().parents[1]
STEPS = ['main', 'region', 'campaign-handoff', 'gameplay', 'campaign-menu', 'gameplay-resumed']
SOURCES = ['tools/test-native-campaign.py', 'tools/run-qt-shell.sh',
           'apps/qt-shell/campaign_smoke_test.cpp', 'apps/qt-shell/campaign_smoke_test.hpp',
           'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt',
           'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/menu_bridge.cpp',
           'tools/test-live-campaign-mini.py', 'tools/campaign-gameplay-input.py',
           'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_command_session.cpp',
           'apps/qt-shell/live_command_session.hpp', 'tools/prepare-menu-observer.py',
           'tools/run-menu-observer.py', 'tools/menu-game-runner.py', 'tools/profile-render-stream.py',
           'runtime/menu/campaign_gameplay_observe.h', 'tools/campaign_gameplay.py', 'tools/campaign-combat-input.py']
SOURCES += ['tools/campaign_crowded.py', 'tests/campaign-crowded-test.py','runtime/render/precise_clock.h', 'tests/campaign-clock-reference.c', 'tools/test-native-clock.py', 'runtime/render/pacer_yield.h', 'tests/campaign-pacer-reference.c', 'tools/test-native-pacer.py', 'apps/qt-shell/healing_smoke_test.cpp', 'tools/campaign-healing-input.py', 'apps/qt-shell/live_battle_menu_controller.cpp', 'apps/qt-shell/live_spell_menu_controller.cpp', 'apps/qt-shell/spellbox_widget.cpp', 'apps/qt-shell/spellbox_widget.hpp', 'tools/campaign-quit-input.py','tools/campaign_spell.py', 'tests/campaign-spell-test.py', 'tools/campaign_movement.py', 'tests/campaign-movement-test.py', 'runtime/menu/campaign_spell_observe.h', 'renderer/blit.cpp', 'renderer/blit.hpp', 'tools/native_render_config.py']


def linked_compiler_sources(build, root):
    """Follow the built shell's object/archive links, refusing missing receipts."""
    import shlex
    build,root=build.resolve(),root.resolve()
    pending=[build/'CMakeFiles/mnm-qt-shell.dir/link.txt'];visited=set();closure=set()
    while pending:
        link=pending.pop().resolve()
        if link in visited:continue
        visited.add(link)
        base=link.parents[2];objects=0
        for token in shlex.split(link.read_text()):
            path=Path(token)
            if path.suffix not in ('.o','.a'):continue
            path=(base/path).resolve()
            if not path.is_relative_to(build):continue
            if path.suffix=='.a':
                target=path.stem.removeprefix('lib')
                pending.append(path.parent/'CMakeFiles'/f'{target}.dir/link.txt')
                continue
            objects+=1
            for dependency in shlex.split(Path(str(path)+'.d').read_text().replace('\\\n',' ')):
                source=(base/Path(dependency)).resolve()
                if source.is_relative_to(root) and not source.is_relative_to(root/'working'):
                    closure.add(source.relative_to(root).as_posix())
        if not objects:raise ValueError('Linked target has no compiler dependency receipts: '+str(link))
    return closure


def validate_portrait_stress(inputs, flow, capture_root, seconds, min_fps=20):
    stress=inputs.get('portrait_stress',{})
    cycles=[a for a in inputs['actions'] if a['action']=='portrait-recenter']
    starts=[a['seconds'] for a in inputs['actions'] if a['action']=='portrait-stress-start']
    ends=[a['seconds'] for a in inputs['actions'] if a['action']=='portrait-stress-complete']
    if len(starts)!=1 or len(ends)!=1 or stress.get('seconds',0)<seconds or len(cycles)<3 or stress.get('cycles')!=len(cycles):
        raise ValueError('Incomplete repeated portrait recentering')
    elapsed=0;samples=[]
    for row in flow['gameplay_rates']:
        if row['cycle']!=0:continue
        begin=elapsed;elapsed+=row['seconds']
        # The helper starts just after the Qt sampling clock. Keep complete
        # interior windows; the whole-phase gate still includes both boundaries.
        if begin>=starts[0]+2 and elapsed<=ends[0]-2:samples.append(row)
    duration=sum(r['seconds'] for r in samples)
    if duration<seconds-7:raise ValueError('Missing portrait performance windows')
    native=sum(r['native_frames'] for r in samples)/duration
    paints=sum(r['painted_frames'] for r in samples)/duration
    worst=min(min(r['native_fps'],r['paint_fps']) for r in samples)
    if min(native,paints)<min_fps or worst<min_fps/2 or any(r['seconds']>2 for r in samples):raise ValueError('Portrait rendering too slow')
    capture=next(c for c in inputs['captures'] if c['name'].endswith('-portrait-stress'))
    if not re.fullmatch(r'combat-\d{2}-[a-z-]+',capture['name']):raise ValueError('Unsafe portrait capture name')
    if capture['metadata']['fallback'] or not all(capture['metadata'][r+'_saved'] for r in ('native','original')):raise ValueError('Missing independent portrait capture')
    from PIL import Image,ImageChops
    images=[]
    for route in ('native','original'):
        path=capture_root/(capture['name']+'-'+route+'.png')
        if sha(path)!=capture['image_sha256'][route]:raise ValueError('Portrait image changed after capture')
        images.append(Image.open(path).convert('RGB').crop((716,507,759,538)))
    maximum=max(hi for lo,hi in ImageChops.difference(*images).getextrema())
    if maximum>1:raise ValueError('Native portrait face differs from independent original pixels')
    return dict(cycles=len(cycles),seconds=duration,native_fps=native,paint_fps=paints,worst_window_fps=worst,
                portrait_region=[716,507,759,538],maximum_channel_error=maximum,
                scope='Unsynchronized stable wizard face region only; full World and animation equivalence remain pending')



def validate_dialogue_readiness(inputs, capture_root, resident_world=False):
    oracle="resident-native" if resident_world else "dual-draw"
    if inputs.get("dialogue_oracle","dual-draw")!=oracle:
        raise ValueError("Dialogue oracle differs from requested rendering mode")
    routes=("native",) if resident_world else ("native","original")
    checks=inputs.get('dialogue_checks',[])
    if not 1<=len(checks)<=12 or set(checks[-1]['visible'])!={'native','original'} or any(checks[-1]['visible'][r] for r in routes):
        raise ValueError('Missing cleared introductory dialogue from requested World oracle')
    captures={c['name']:c for c in inputs['captures']}
    for item in checks:
        if set(item['texts'])!={'native','original'} or item['visible']!={r:'hermes' in item['texts'][r].lower() for r in ('native','original')}:
            raise ValueError('Dialogue classification differs from retained OCR')
        if item['capture'] not in captures:raise ValueError('Missing dialogue capture')
    for cap in inputs['captures']:
        if not re.fullmatch(r'combat-\d{2}-[a-z0-9-]+',cap['name']) or cap['metadata']['fallback'] or not all(cap['metadata'][r+'_saved'] for r in ('native','original')):
            raise ValueError('Invalid independent gameplay diagnostic capture')
        for route in ('native','original'):
            if sha(capture_root/(cap['name']+'-'+route+'.png'))!=cap['image_sha256'][route]:raise ValueError('Gameplay diagnostic image changed after capture')
    return dict(verified=True,checks=len(checks),dismissed=sum(all(i['visible'][r] for r in routes) for i in checks),oracle=oracle,
                scope=('Bounded resident Hermes heading OCR with retained CPU diagnostic images; CPU World pixels are stale and cannot classify dismissal. Original summon/cast/combat observation separately required; full UI equivalence excluded.' if resident_world else 'Bounded introductory Hermes heading OCR from independent native/original images; other dialogue speakers/layouts and internal tutorial admission excluded'))


def validate_casting_combat(experiment, inputs, capture_root):
    rows=read_rows(experiment/'gameplay-events.bin', complete=True)
    summon=inputs['summon']; before=summon['before_sequence']; after=summon['after_sequence']; slot=summon['slot']; owner=inputs['player_owner']
    if len({r[2] for r in rows})!=1:raise ValueError('Gameplay observation thread changed')
    initial=creatures([r for r in rows if r[0]<=before])
    wizards=[a for a in initial.values() if a['type']==0 and a['active'] and a['health']>0]
    if len(wizards)!=1 or wizards[0]['owner']!=owner:raise ValueError('Missing original player owner identity')
    created={}
    for cast in [summon]+inputs.get('additional_summons',[]):
        begin,end,source=cast['before_sequence'],cast['after_sequence'],cast['slot']
        if not before<=begin<end<=rows[-1][0]:raise ValueError('Invalid summon observation interval')
        old=creatures([r for r in rows if r[0]<=begin]).get(source)
        zombie=creatures([r for r in rows if begin<r[0]<=end]).get(source)
        if old and old['active'] and old['type']==14 and old['owner']==owner:
            raise ValueError('Player Zombie was already active before the cast')
        if not zombie or zombie['type']!=14 or zombie['owner']!=owner or not zombie['active'] or zombie['health']<=0:
            raise ValueError('No newly living player Zombie after native summon')
        created[source]=end
    captures={c['name']:c for c in inputs['captures']}
    for key,count in [('before_capture','0/15'),('after_capture','1/15')]:
        capture=captures[summon[key]]
        if not re.fullmatch(r'combat-\d{2}-[a-z-]+',capture['name']):raise ValueError('Unsafe capture name')
        for route in ('native','original'):
            if sha(capture_root/(capture['name']+'-'+route+'.png'))!=capture['image_sha256'][route]:raise ValueError('Capture image changed after OCR')
        if capture['ocr']!={'native':count,'original':count} or not capture['metadata']['native_saved'] or not capture['metadata']['original_saved'] or capture['metadata']['fallback']:
            raise ValueError('Missing independent native/original summon count')
    hits=[r for r in damage_rows(rows,owner,list(created),after) if r[0]>created[r[4]]]
    if not hits:
        raise ValueError('No observed original player Zombie versus enemy melee damage')
    lethal=player_lethal_rows(rows,owner,wizards[0]['slot'],hits)
    spell_rows=read_spell_rows(experiment/'spell-events.bin',complete=True)
    spell_lethal=player_engaged_fireball_lethal(spell_rows,rows,owner,wizards[0]['slot'],hits)
    if not lethal and not spell_lethal:raise ValueError('No observed player party lethal enemy health depletion')
    return dict(casting_verified=True,combat_verified=True,summoned_slot=slot,player_owner=owner,
                movement=validate_movement_probes(rows,inputs),
                spells=validate_spell_observation(spell_rows,rows,inputs),
                spell_trace_sha256=sha(experiment/'spell-events.bin'),
                enemy_health_depleted=True,lethal_melee_events=len(lethal),first_lethal_melee=list(lethal[0]) if lethal else None,
                lethal_fireball_events=len(spell_lethal),first_lethal_fireball=list(spell_lethal[0]) if spell_lethal else None,
                combat_source_slots=sorted({r[4] for r in hits}),melee_damage_events=len(hits),first_melee_damage=list(hits[0]),trace_sha256=sha(experiment/'gameplay-events.bin'))



def validate_healing(experiment, inputs, flow, capture_root, seconds, min_fps=20):
    if not flow.get('success') or not flow.get('healing_mode') or flow.get('native_command_fallback') is not False or flow.get('native_command_frames',0)<6:
        raise ValueError('Incomplete native healing presentation')
    if flow.get('native_recoveries') or flow.get('native_error'):
        raise ValueError('Healing presentation recovered or failed')
    steps=flow.get('steps',[])
    if [s.get('step') for s in steps]!=['main','quick','setup','map','setup-ready','spells','healing-loadout'] or any(not s.get('screenshot_saved') for s in steps[:-1]):
        raise ValueError('Incomplete native healing menus')
    loadout=steps[-1]
    if sorted(a['spell'] for a in loadout['assignments'] if a['spell'])!=['14','41'] or loadout['owner']!=inputs['owner'] or flow['owner']!=inputs['owner']:
        raise ValueError('Native loadout did not assign exactly Cure and Zombie')
    if not inputs.get('success') or inputs.get('seconds',0)<seconds:
        raise ValueError('Incomplete physical native healing input')
    samples=flow.get('gameplay_rates',[]);elapsed=sum(r['seconds'] for r in samples)
    if elapsed<seconds-2 or any(r['cycle']!=0 for r in samples):raise ValueError('Missing healing rate samples')
    native=sum(r['native_frames'] for r in samples)/elapsed;paints=sum(r['painted_frames'] for r in samples)/elapsed
    if min(native,paints)<min_fps or any(min(r['native_fps'],r['paint_fps'])<min_fps/2 or r['seconds']>2 for r in samples):raise ValueError('Healing rendering too slow')
    gp=read_rows(experiment/'gameplay-events.bin',complete=True);spells=read_spell_rows(experiment/'spell-events.bin',complete=True)
    if len({r[2] for r in gp+spells})!=1 or len({r[3] for r in gp if r[1]==1})<3:raise ValueError('Healing observation thread/World baseline missing')
    proof=inputs['healing'];begin,end=proof['before_sequence'],proof['after_sequence']
    if not 0<begin<end<=gp[-1][0]:raise ValueError('Invalid healing interval')
    before=creatures([r for r in gp if r[0]<=begin]).get(inputs['wizard_slot'])
    if not before or not before['active'] or before['type']!=0 or before['owner']!=inputs['owner'] or not 0<before['health']<inputs['initial_wizard']['health']:
        raise ValueError('Original human wizard was not injured before Cure')
    t0,t1=gp[begin-1][3],gp[end-1][3]
    heals=[r for r in player_cure_healing(spells,inputs['owner'],inputs['wizard_slot']) if ((r[3]-t0)&0xffffffff)<=((t1-t0)&0xffffffff)]
    if not heals or proof['first_health_change']!=list(heals[0]):raise ValueError('Missing matching original Cure health recovery and mana debit')
    labels=[]
    for cap in inputs['captures']:
        if not re.fullmatch(r'healing-\d{2}-[a-z-]+',cap['name']) or cap['metadata']['fallback'] or not all(cap['metadata'][r+'_saved'] for r in ('native','original')):raise ValueError('Invalid healing image capture')
        labels.append(cap['name'].split('-',2)[2])
        for route in ('native','original'):
            if sha(capture_root/(cap['name']+'-'+route+'.png'))!=cap['image_sha256'][route]:raise ValueError('Healing image changed after input')
    if not {'before-cure','after-cure'}<=set(labels):raise ValueError('Missing independent before/after Cure images')
    events=(experiment/'events.bin').read_bytes()
    if events[:16]!=b'MNMMENU1'+struct.pack('<II',1,64) or (len(events)-16)%64:raise ValueError('Invalid healing menu trace')
    rows=list(struct.iter_unpack('<16I',events[16:]));actions=[(r[3],r[9]) for r in rows if r[1]==3]
    if [r[0] for r in rows]!=list(range(1,len(rows)+1)) or len(rows)>256 or len({r[2] for r in rows if r[1]==3})!=1:
        raise ValueError('Invalid healing menu sequence/thread')
    if actions[:5]!=[(3,2),(22,2),(14,2),(25,0),(14,1)] or actions[-1]!=(7,12) or any(menu!=7 for menu,arg in actions[5:]):
        raise ValueError('Unexpected original Quick Battle/loadout callbacks')
    channel=(experiment/'channel.bin').read_bytes()
    if len(channel)!=106496 or channel[:16]!=b'MNMMCM12'+struct.pack('<II',12,106496) or struct.unpack_from('<I',channel,36*4)[0]!=6:
        raise ValueError('Missing native healing menu acknowledgements')
    return dict(verified=True,first_health_change=list(heals[0]),healing_events=len(heals),native_fps=native,paint_fps=paints,
                worst_window_fps=min(min(r['native_fps'],r['paint_fps']) for r in samples),actions=actions,
                spell_trace_sha256=sha(experiment/'spell-events.bin'),trace_sha256=sha(experiment/'gameplay-events.bin'),
                scope='Native offered Cure/Zombie Quick Battle map2 loadout, injured living human wizard and original Cure positive HP plus mana debit; no cleanse/clamp/refund formula or full battle completion claim')

def sha(path):
    digest=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):digest.update(block)
    return digest.hexdigest()


def validate_flow(flow, cycles=1, stress_seconds=0, min_fps=20, normal_quit=False):
    if flow.get('success') is not True:
        raise ValueError(flow.get('error') or 'Campaign driver did not complete')
    if flow.get('native_command_fallback') is not False:
        raise ValueError('Native rendering fallback or missing fallback evidence')
    if type(flow.get('native_command_frames')) is not int or flow['native_command_frames'] < 6:
        raise ValueError('Insufficient native command presentations')
    steps = flow.get('steps', [])
    expected = STEPS + ['campaign-menu', 'gameplay-resumed']*(cycles-1)
    if normal_quit:
        expected += ['campaign-quit-menu','quit-no-resumed','campaign-quit-menu','campaign-defeat','quit-main']
        if flow.get('normal_quit') is not True:raise ValueError('Missing normal original game Quit')
    if [s.get('step') for s in steps] != expected:
        raise ValueError('Incomplete campaign click-through')
    if flow.get("route_screenshots",True) and any(s.get('screenshot_saved') is not True for s in steps if s['step'] != 'campaign-handoff'):
        raise ValueError('Missing campaign screenshots')
    if stress_seconds:
        rates = flow.get('gameplay_rates', [])
        for cycle in range(cycles+1):
            samples = [r for r in rates if r.get('cycle') == cycle]
            seconds = sum(r.get('seconds', 0) for r in samples)
            if seconds < stress_seconds-2:raise ValueError('Incomplete gameplay performance sample')
            native_fps = sum(r['native_frames'] for r in samples)/seconds
            paint_fps = sum(r['painted_frames'] for r in samples)/seconds
            if min(native_fps, paint_fps) < min_fps or any(min(r['native_fps'], r['paint_fps']) < min_fps/2 or r['seconds'] > 2 for r in samples):
                raise ValueError(f'Gameplay too slow in cycle {cycle}: native {native_fps:.1f}, paints {paint_fps:.1f} FPS; minimum {min_fps}')


def world_ready(experiment):
    path = experiment / 'events.bin'
    if not path.exists():return False
    data = path.read_bytes()
    if len(data) < 16 or data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64):return False
    # A writer may be appending its final record; only inspect complete rows.
    end = 16 + ((len(data)-16)//64)*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    ticks = [r for r in rows if r[1] == 12 and r[3] == 2 and r[6] == 1 and r[11] == 0x6cbb78]
    return len(ticks) >= 3 and len({r[2] for r in ticks}) == 1


def validate_events(experiment, cycles=1, difficulty=0, normal_quit=False):
    """Independent engine-side checks; window appearance alone cannot pass."""
    data = (experiment / 'events.bin').read_bytes()
    if data[:16] != b'MNMMENU1' + struct.pack('<II', 1, 64) or (len(data)-16) % 64:
        raise ValueError('Invalid original callback observation')
    rows = list(struct.iter_unpack('<16I', data[16:]))
    if len(rows) > 256 or [r[0] for r in rows] != list(range(1, len(rows)+1)):
        raise ValueError('Invalid original callback sequence')
    actions = [(r[3], r[9]) for r in rows if r[1] == 3]
    expected=[(3, 0)] + ([(18,65536+difficulty)] if difficulty else []) + [(18,0)] + [(17,4)]*cycles
    if normal_quit:expected += [(17,3),(17,3),(6,21),(3,4)]
    if actions != expected:
        raise ValueError('Unexpected New Game/Enter/Cancel callback trace: ' + str(actions))
    ticks = [r for r in rows if r[1] == 12 and r[3] == 2 and r[6] == 1 and r[11] == 0x6cbb78]
    cancels = [r for r in rows if r[1] == 3 and r[3] == 17 and r[9] == 4]
    resumes = [r for r in rows if r[1] == 20 and r[11] == 0x6cbb78 and r[0] > cancels[0][0]]
    if len(ticks) < 3 or len(resumes)<cycles or len({r[2] for r in rows if r[1] in (3, 12, 20)}) != 1:
        raise ValueError('Missing same-thread original World updates/Cancel resume')
    channel = (experiment / 'channel.bin').read_bytes()
    if channel[:16] != b'MNMMCM17' + struct.pack('<II', 17, 196608) or len(channel) != 196608:
        raise ValueError('Campaign test did not use the ordinary V17 menu protocol')
    if struct.unpack_from('<I', channel, 36*4)[0] != 2+cycles+bool(difficulty)+(4 if normal_quit else 0):
        raise ValueError('Missing third menu acknowledgement')
    if normal_quit:
        answers=[r for r in rows if r[1]==22]
        quits=[r for r in rows if r[1]==3 and r[3]==17 and r[9]==3]
        if len(answers)!=2 or any(q[4]!=0x6a5088 for q in quits) or [r[9] for r in answers]!=[1,0] or any(r[4]!=0x6a5088 or r[2]!=ticks[0][2] or r[0]<=q[0] for r,q in zip(answers,quits)):
            raise ValueError('Missing original No/Yes campaign Quit answers')
        if not any(r[1]==20 and answers[0][0]<r[0]<quits[1][0] and r[11]==0x6cbb78 and r[2]==answers[0][2] for r in rows):
            raise ValueError('Original No did not resume World before repeated Quit')
    return {'actions': actions, 'world_ticks': len(ticks), 'world_resumes': len(resumes),
            'protocol_version': 17, 'events_sha256': sha(experiment / 'events.bin')}


def stop_group(process):
    if process is None or process.poll() is not None:
        return
    try:
        os.killpg(process.pid, signal.SIGTERM)
        process.wait(timeout=5)
    except ProcessLookupError:
        return
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait(timeout=5)


def clean_environment(root):
    # Diagnostic overrides can silently select another rendering route.
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(XDG_CONFIG_HOME=str(root / 'config'), WINEPREFIX=str(root / 'wineprefix'),
               QT_QPA_PLATFORM='xcb', PYTHONDONTWRITEBYTECODE='1')
    env.pop('WAYLAND_DISPLAY', None)
    return env


def validate_native_draw(experiment):
    metadata=json.loads((experiment/'manifest.json').read_text())
    if metadata.get('native_draw') is not True or metadata.get('native_render') is not True:raise ValueError('Owned native graphics were not staged')
    if sha(experiment/'game/MnmDraw.dll')!=metadata['native_draw_sha256']:raise ValueError('Native graphics DLL changed')
    if (experiment/'game/native-draw-refusal.bin').exists():raise ValueError('Unadmitted native graphics method')
    raw=(experiment/'native-draw-status.bin').read_bytes()
    if len(raw)!=96 or raw[:8]!=b'MNMDRW01':raise ValueError('Native graphics status missing or malformed')
    q=struct.unpack('<24I',raw)
    if q[2:5]!=(2,96,0) or q[19] or q[23] or not q[9] or not q[10] or q[20]<q[9] or q[22]:raise ValueError('Native graphics startup or module absence failed')
    if q[5]>64 or q[6]>512 or q[7]>128 or q[8]>64 or q[15]>512 or q[16]>256*1024*1024 or q[17]>128 or q[18]>64:raise ValueError('Native graphics resources exceeded bounds')
    return dict(ddraw_disabled=True,ddraw_module_absent=True,native_dll_sha256=metadata['native_draw_sha256'],status_sha256=sha(experiment/'native-draw-status.bin'),factory_creations=q[9],surface_creations=q[10],peak_surfaces=q[15],peak_bytes=q[16],scope='Public native Qt menus and command presentation use owned graphics services with real ddraw disabled/absent; World-body suppression requires the separate resident proof. No V4 World-body suppression, independent original pixels or semantic COM teardown assertion.')


def start_native_hardware(out,env):
    gpu=subprocess.check_output(['nvidia-smi','--query-gpu=pci.bus_id','--format=csv,noheader'],text=True).strip().splitlines()[0]
    match=re.fullmatch(r'[0-9A-Fa-f]{8}:([0-9A-Fa-f]{2}):([0-9A-Fa-f]{2})\.([0-7])',gpu)
    if not match:raise ValueError('Unsupported NVIDIA bus identity')
    bus='PCI:'+':'.join(str(int(x,16)) for x in match.groups())
    display=next(n for n in range(170,190) if not Path(f'/tmp/.X11-unix/X{n}').exists() and not Path(f'/tmp/.X{n}-lock').exists())
    configuration=out/'private-gpu.conf'
    configuration.write_text('Section "ServerFlags"\n Option "AutoAddDevices" "False"\n Option "AutoEnableDevices" "False"\n Option "DontVTSwitch" "True"\nEndSection\nSection "Device"\n Identifier "native-test-gpu"\n Driver "nvidia"\n BusID "'+bus+'"\n Option "UseDisplayDevice" "none"\n Option "AllowEmptyInitialConfiguration" "True"\n Option "ProbeAllGpus" "False"\nEndSection\nSection "Screen"\n Identifier "native-test-screen"\n Device "native-test-gpu"\n DefaultDepth 24\n SubSection "Display"\n  Depth 24\n  Virtual 1280 1024\n EndSubSection\nEndSection\nSection "ServerLayout"\n Identifier "native-test-layout"\n Screen "native-test-screen"\nEndSection\n')
    with (out/'private-gpu-start.log').open('w') as log:
        server=subprocess.Popen(['/usr/lib/Xorg',f':{display}','-seat','native-campaign-test','-config',str(configuration),'-logfile',str(out/'private-gpu-xorg.log'),'-nolisten','tcp','-noreset','-novtswitch','-sharevts'],stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
    try:
        deadline=time.monotonic()+30
        while not Path(f'/tmp/.X11-unix/X{display}').exists():
            if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Private NVIDIA display failed')
            time.sleep(.1)
        for key in ['LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','WAYLAND_DISPLAY','QT_XCB_GL_INTEGRATION']:env.pop(key,None)
        env.update(DISPLAY=f':{display}',MESA_LOADER_DRIVER_OVERRIDE='zink',GALLIUM_DRIVER='zink')
        with (out/'hardware-driver.log').open('w') as log:subprocess.run(['glxinfo','-B'],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=20)
        driver=(out/'hardware-driver.log').read_text()
        if 'NVIDIA' not in driver or 'Accelerated: yes' not in driver or 'llvmpipe' in driver or 'lavapipe' in driver:raise ValueError('Hardware renderer fallback refused')
        return server
    except BaseException:
        stop_group(server)
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stress-seconds', type=int, default=0, help='Exercise physical gameplay input per phase (0 or 10..120)')
    parser.add_argument('--menu-cycles', type=int, choices=range(1,6), default=1, help='Number of native Escape/Mini/Cancel returns')
    parser.add_argument('--min-fps', type=float, default=20, help='Minimum average native publications and visible Qt paints during stress')
    parser.add_argument('--difficulty', type=int, choices=range(4), default=0, help='Choose the normal Region Entry difficulty via its native radio (0 Initiate; 1 Apprentice for camera/combat stress)')
    parser.add_argument('--require-casting-combat', action='store_true', help='Require successful native-viewport summon and player-versus-enemy melee damage observation')
    parser.add_argument('--portrait-stress-seconds', type=int, default=0, help='Repeat portrait recentering after verified combat (0 or 10..120)')
    parser.add_argument('--disable-precise-clock',action='store_true',help='Diagnostic control: retain original coarse GetTickCount uptime')
    parser.add_argument('--trace-engine-cadence',action='store_true',help='Memory-only original simulation/animation/World cadence trace; flush at normal process exit')
    parser.add_argument('--scene-production-probe',choices=('observe','full','slice','frozen-history','prepared-scene','budgeted-scene'),help='Bounded original scene production audit; prepared-scene repeats after original queue cleanup before creature/camera post-work')
    pacer=parser.add_mutually_exclusive_group()
    pacer.add_argument('--enable-pacer-yield',dest='pacer_yield',action='store_true',help='Experimental opt-in cooperative Sleep0 wait; has no demonstrated portrait performance benefit')
    pacer.add_argument('--disable-pacer-yield',dest='pacer_yield',action='store_false')
    parser.set_defaults(pacer_yield=False)
    parser.add_argument('--crowded-case',action='store_true',help='Four-player native Quick Battle with ordinary 200 mana/800 health/30 control settings, eight Zombie summon attempts and 180..240 seconds of observed crowded combat')
    parser.add_argument('--healing-case',action='store_true',help='Separate native Quick Battle Cure loadout and injured human-wizard healing case')
    parser.add_argument('--normal-quit',action='store_true',help='Require native Mini Quit No/Yes, native report/Main and original normal process exit')
    parser.add_argument('--spell-cases', action='store_true', help='Require invalid-target and insufficient-mana refusal plus player Fireball damage through native gameplay input')
    parser.add_argument('--claims', type=Path, help='Prospective coverage scope/source declaration prepared before this run')
    parser.add_argument('--independent-world',action='store_true',help='Use explicit owned V5 delivery and independent native World/HUD drawing')
    parser.add_argument('--resident-world',action='store_true',help='Require GPU-resident World delivery and ordered public UI composition with no pixel return')
    parser.add_argument('--profile-resident',action='store_true',help='Opt-in bounded CPU/GPU stage and original pacing diagnostics; cadence is instrumented')
    parser.add_argument('--capture-resident',action='store_true',help='Retain at most8192 ordered source-only World blocks/16GiB for offline resident replay')
    parser.add_argument('--native-draw',action='store_true',help='Require owned graphics services with real DirectDraw disabled')
    parser.add_argument('--native-voices',action='store_true',help='Require native voice selection, successful upload/play and output retirement')
    parser.add_argument('--default-audio',action='store_true',help='Verify default native audio without an explicit shell audio flag; requires normal Quit')
    parser.add_argument('--native-loading',action='store_true',help='Require guarded native WAV loading in the default audio route')
    parser.add_argument('--native-factory',action='store_true',help='Require native factory, disabled Wine dsound and zero unsupported local COM calls')
    parser.add_argument('--native-media',action='store_true',help='Enable native movies/file sounds alongside live menus')
    parser.add_argument('--native-music',action='store_true',help='Require native CD hooks, music PCM and normal Quit retirement')
    parser.add_argument('--default-media',action='store_true',help='Verify native movie routing without an explicit shell media flag')
    parser.add_argument('--default-music',action='store_true',help='Verify native CD routing without an explicit shell music flag')
    parser.add_argument('--legacy-media',action='store_true',help='Opt out of native movie/file-sound routing')
    parser.add_argument('--no-native-music',action='store_true',help='Opt out of the native CD route')
    parser.add_argument('--skip-movies',action='store_true',help='Suppress intro playback in this bounded campaign run')
    parser.add_argument('--disable-wine-dsound',action='store_true',help='Force Wine DirectSound disable; marked native factory/media staging disables it by default')
    parser.add_argument('--wine-import-census',action='store_true',help='Count original import calls after native adapters; excludes dynamic/COM/forwarded calls')
    parser.add_argument('--initial-silent-start',action='store_true',help='Require initially absent physical outputs and progressing native discard/decoder playback')
    parser.add_argument('--legacy-audio',action='store_true',help='Select original DirectSound voices for this diagnostic run')
    parser.add_argument('--source-game',type=Path,help='Pinned source installation under working/; preserve the default working installation')
    parser.add_argument('--legacy-directdraw',action='store_true',help='Explicit diagnostic control using real DirectDraw; native default retirement proof is disabled')
    parser.add_argument('--check-only', action='store_true', help='Build and check combined-mode admission; no game launch')
    parser.add_argument('--worker-status-cadence',action='store_true',help='Experimental16ms owned-worker status construction')
    parser.add_argument('--worker-status-every-turn',action='store_true',help='Diagnostic baseline: construct owned-worker status on every turn')
    parser.add_argument('--synchronous-route-capture',action='store_true',help='Diagnostic comparison: encode route screenshots on GUI instead of bounded background job')
    parser.add_argument('--no-route-screenshots',action='store_true',help='Diagnostic comparison: skip route screenshots, retain casting/combat diagnostic captures and all performance gates')
    parser.add_argument('--display-while-pending',action='store_true',help='Keep last imported native World displayable while new async generation waits for command import')
    parser.add_argument('--hitch-capture',action='store_true',help='Bounded warm active paint-hitch windows with queue/UI/lease state; requires frame trace')
    parser.add_argument('--frame-trace',action='store_true',help='Bounded public worker/source/GUI timeline; owned independent World only')
    parser.add_argument('--reuse-world-image',action='store_true',help='Experimental reuse of completed native GPU World between source updates')
    parser.add_argument('--async-world',action='store_true',help='Experimental separate native GPU World producer and immutable bounded handoff')
    parser.add_argument('--hardware',action='store_true',help='Use a private no-monitor TITAN/NVIDIA display; refuse software fallback')
    parser.add_argument('--display', help='Use this X11/XWayland display instead of a private Xvfb (moves focus and sends Escape)')
    parser.add_argument('--software-threads', type=int, choices=range(1,65), default=4, help='Explicit Mesa worker count for the private software-rendered display')
    parser.add_argument('--timeout', type=int, default=330, help='Bound live automation, in seconds (30..600)')
    args = parser.parse_args()
    explicit_native_audio=args.native_voices
    explicit_native_media=args.native_media;explicit_native_music=args.native_music
    args.native_media |= args.default_media;args.native_music |= args.default_music
    if args.legacy_media and args.native_media:parser.error('Legacy and native media routes conflict')
    if args.no_native_music and args.native_music:parser.error('Native music admission and opt-out conflict')
    if args.legacy_audio and (args.native_voices or args.default_audio or args.native_loading or args.native_factory):parser.error('Legacy and native audio are separate routes')
    args.native_voices |= args.default_audio or args.native_loading or args.native_factory or args.native_music
    if args.native_music and (args.legacy_audio or not args.normal_quit):parser.error('Native music requires native audio and normal Quit')
    if args.disable_wine_dsound and not args.native_factory:parser.error('Module-disable diagnostic requires native factory verification')
    if args.worker_status_cadence and not args.independent_world:parser.error('Worker status cadence requires independent World')
    if args.worker_status_every_turn and not args.independent_world:parser.error('Worker status baseline requires independent World')
    if args.synchronous_route_capture and (not args.frame_trace or args.no_route_screenshots):parser.error('Synchronous capture comparison requires frame trace and retained route screenshots')
    if args.no_route_screenshots and (not args.frame_trace or args.healing_case or args.crowded_case):parser.error('Route screenshot comparison requires frame trace and ordinary campaign')
    if args.display_while_pending and not args.async_world:parser.error('Display while pending requires --async-world')
    if args.hitch_capture and not args.frame_trace:parser.error('Hitch capture requires --frame-trace')
    if args.frame_trace and not args.independent_world:parser.error('Frame trace requires independent resident World')
    if args.reuse_world_image and not args.independent_world:parser.error('World image reuse requires independent resident World')
    if args.async_world and (not args.reuse_world_image or not args.independent_world or args.profile_resident or args.capture_resident):parser.error('Async World requires independent image reuse and excludes resident capture profiling')
    if args.independent_world and (not args.resident_world or args.scene_production_probe):parser.error('Independent World requires resident mode and excludes original extra-draw diagnostics')
    if (args.profile_resident or args.capture_resident) and not args.resident_world:parser.error('Resident diagnostics require --resident-world')
    if args.capture_resident and not args.profile_resident:parser.error('Resident capture requires --profile-resident')
    if args.native_draw and args.legacy_directdraw:parser.error('Owned graphics and legacy DirectDraw are separate routes')
    explicit_native_draw=args.native_draw
    args.native_draw=not args.legacy_directdraw
    if args.hardware and args.display:parser.error('--hardware and --display are separate display routes')
    if args.crowded_case:
        if args.healing_case:parser.error('Select only one Quick Battle case')
        if not args.stress_seconds:args.stress_seconds=180
        if not 180 <= args.stress_seconds <= 240:parser.error('Crowded stress requires 180..240 seconds')
    if args.healing_case or args.crowded_case:
        if args.require_casting_combat or args.spell_cases or args.normal_quit or args.portrait_stress_seconds:parser.error('Healing is a separate Quick Battle journey')
        if not args.stress_seconds:args.stress_seconds=20
    if args.require_casting_combat:
        if args.difficulty not in (1,2,3):parser.error('Casting/combat requires --difficulty 1, 2 or 3 (other campaign journeys remain separate)')
        if not args.stress_seconds:args.stress_seconds=60
    if args.spell_cases and not args.require_casting_combat:parser.error('--spell-cases requires casting/combat')
    if args.native_voices and not (args.check_only or args.normal_quit):parser.error('Native voice retirement proof requires --normal-quit')
    if args.initial_silent_start and not (args.native_voices and args.native_loading and args.native_factory and args.native_media and args.native_music and args.normal_quit):
        parser.error('Silent-start proof requires all native audio/media/music paths and normal Quit')
    if not 30 <= args.timeout <= 600:
        parser.error('--timeout must be between 30 and 600')
    if args.portrait_stress_seconds and (not args.require_casting_combat or not 10 <= args.portrait_stress_seconds <= 120):parser.error('Portrait stress requires casting/combat and 10..120 seconds')
    if args.stress_seconds and not args.crowded_case and not 10 <= args.stress_seconds <= 120:parser.error('--stress-seconds must be 0 or 10..120')
    if not 1 <= args.min_fps <= 120:parser.error('--min-fps must be 1..120')
    parent = ROOT / 'working/tests/native-campaign'
    parent.mkdir(parents=True, exist_ok=True)
    import tempfile
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('Campaign test evidence: ' + str(out), flush=True)
    report = {'schema': 1, 'success': False, 'status': 'running', 'stage': 'build',
              'required_mode': ['native-menus', 'native-command-presentation'],
              'launch_requested': False, 'game_entry_verified': False, 'flow_completed': False, 'movement_verified': False,
              'native_draw_required':args.native_draw,'native_draw_default_required':args.native_draw and not explicit_native_draw, 'native_voices_required':args.native_voices, 'native_audio_default_required':args.default_audio, 'legacy_audio_requested':args.legacy_audio, 'complete_drawing_replacement': False, 'cleanup': 'bounded private-session termination',
              'native_wave_loading_required':args.native_loading,
              'native_audio_factory_required':args.native_factory,
              'native_media_required':args.native_media,
              'native_music_required':args.native_music,
              'native_media_default_required':args.default_media,
              'native_music_default_required':args.default_music,
              'wine_dsound_disable_requested':args.disable_wine_dsound,
              'casting_combat_required':args.require_casting_combat,'casting_verified':False,'combat_verified':False,
              'portrait_stress_seconds':args.portrait_stress_seconds,
              'route_screenshots_requested':not args.no_route_screenshots,
              'crowded_required':args.crowded_case,'healing_required':args.healing_case,'spell_cases_required':args.spell_cases,'normal_quit_required':args.normal_quit,
              'difficulty': args.difficulty, 'stress_seconds_per_phase': args.stress_seconds, 'menu_cycles': args.menu_cycles, 'minimum_fps': args.min_fps,
              'sources': {p: sha(ROOT / p) for p in SOURCES}}
    if args.native_draw and not args.claims:
        args.claims=out/'prospective-claims.json'
        declaration_command=['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.public-resident-world' if args.resident_world else 'NR.directdraw-public-native-menu','--scenario','NR.public-resident-world.campaign' if args.resident_world else 'native-draw-public-menu-campaign-20261011','--output',str(args.claims)]
        if args.independent_world:declaration_command.extend(['--behavior','HOST.native-worker-status-cadence'])
        if args.frame_trace:declaration_command.extend(['--behavior','HOST.native-frame-trace','--scenario','HOST.native-frame-trace.public','--behavior','HOST.native-gui-delivery-diagnostics','--scenario','HOST.native-gui-delivery-diagnostics.public'])
        if args.reuse_world_image:declaration_command.extend(['--behavior','NR.native-retained-world-image','--scenario','NR.native-retained-world-image.public'])
        if args.display_while_pending:declaration_command.extend(['--behavior','HOST.pending-import-display','--scenario','HOST.pending-import-display.public'])
        if args.frame_trace and not (args.no_route_screenshots or args.synchronous_route_capture):declaration_command.extend(['--behavior','HOST.native-route-capture','--scenario','HOST.native-route-capture.public','--behavior','HOST.native-capture-request-intake','--scenario','HOST.native-capture-request-intake.public'])
        if args.hitch_capture:declaration_command.extend(['--behavior','HOST.native-hitch-capture','--scenario','HOST.native-hitch-capture.public'])
        if args.async_world:declaration_command.extend(['--behavior','HOST.native-world-gpu-production','--scenario','HOST.native-world-gpu-production.public'])
        if args.native_voices:declaration_command.extend(['--behavior','HOST.native-menu-audio','--scenario','native-menu-audio-campaign-20261011'])
        if args.native_loading:declaration_command.extend(['--behavior','HOST.native-wave-loading','--scenario','native-wave-loading-campaign-20261012'])
        if args.native_factory:declaration_command.extend(['--behavior','HOST.native-audio-factory','--scenario','native-audio-factory-campaign-20261012'])
        if args.native_media:declaration_command.extend(['--behavior','HOST.native-menu-media','--scenario','native-menu-media-campaign-20261012','--behavior','HOST.native-media-startup-probe','--scenario','native-media-startup-probe-campaign-20261012'])
        if args.native_media:declaration_command.extend(['--behavior','HOST.native-media-output-recovery','--scenario','native-media-output-recovery-campaign-20261012'])
        if args.native_factory and args.native_media:declaration_command.extend(['--behavior','HOST.native-dsound-module-policy'])
        if args.native_voices and args.native_media:declaration_command.extend(['--behavior','HOST.native-audio-initial-silence','--scenario','native-audio-initial-silence-campaign-20261012' if args.initial_silent_start else 'native-audio-initial-silence-healthy-campaign-20261012'])
        if args.native_voices:declaration_command.extend(['--behavior','HOST.native-audio-profile-reads','--scenario','native-audio-profile-campaign-20261012'])
        if args.native_voices:declaration_command.extend(['--behavior','HOST.native-audio-file-sizes','--scenario','native-audio-file-sizes-campaign-20261012'])
        if args.wine_import_census:declaration_command.extend(['--behavior','HOST.wine-import-census','--scenario','wine-import-census-campaign-20261012'])
        if args.native_music:declaration_command.extend(['--behavior','HOST.native-menu-music','--scenario','native-menu-music-campaign-20261012'])
        if args.default_music or args.default_media:declaration_command.extend(['--behavior','HOST.native-audio-defaults','--scenario','native-audio-defaults-campaign-20261012'])
        if args.resident_world:declaration_command.extend(['--behavior','NR.world-source-region-epoch','--behavior','RS.displacement-linear-stride','--behavior','RS.shadow-linear-stride','--behavior','HOST.native-campaign-dialogue-readiness','--behavior','HOST.native-campaign-spell-cases','--behavior','NR.minimap-write-candidates','--behavior','NR.resident-normal-shutdown'])
        if args.profile_resident:declaration_command.extend(['--behavior','NR.resident-stage-profiling','--scenario','NR.resident-stage-profiling.live'])
        if args.trace_engine_cadence:declaration_command.extend(['--behavior','NR.engine-cadence-observation','--scenario','NR.engine-cadence-observation.live'])
        if args.scene_production_probe:declaration_command.extend(['--behavior','NR.scene-production-repetition','--scenario','NR.scene-production-repetition.live'])
        if args.scene_production_probe=='budgeted-scene':declaration_command.extend(['--behavior','NR.scene-draw-budget','--scenario','NR.scene-draw-budget.live'])
        if args.scene_production_probe=='budgeted-scene':declaration_command.extend(['--behavior','HOST.menu-render-progress-heartbeat','--scenario','HOST.menu-render-progress-heartbeat.live'])
        if args.scene_production_probe=='budgeted-scene':declaration_command.extend(['--behavior','HOST.resident-report-publication','--scenario','HOST.resident-report-publication.live'])
        subprocess.run(declaration_command,cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
    if args.claims:
        declaration=json.loads(args.claims.read_text())
        for path,expected in declaration['sources'].items():
            if Path(path).is_absolute() or not (ROOT/path).resolve().is_relative_to(ROOT):raise ValueError('Unsafe declared source')
            if sha(ROOT/path)!=expected:raise ValueError('Declared source changed before execution: '+path)
        report['claims']=declaration['claims'];report['sources'].update(declaration['sources'])
        if args.native_voices and not any(c['behavior']=='HOST.native-menu-audio' for c in declaration['claims']):raise ValueError('Native audio requires its prospective behavior declaration')
        if args.native_loading and not any(c['behavior']=='HOST.native-wave-loading' for c in declaration['claims']):raise ValueError('Native loading requires its prospective behavior declaration')
        if args.native_factory and not any(c['behavior']=='HOST.native-audio-factory' for c in declaration['claims']):raise ValueError('Native factory requires its prospective behavior declaration')
        if args.native_music and not any(c['behavior']=='HOST.native-menu-music' for c in declaration['claims']):raise ValueError('Native music requires its prospective behavior declaration')
    if not args.legacy_audio:
        audio_sources=['tools/native_audio_launch.py','tools/prepare-native-audio.py','tools/prepare-native-draw.py','tools/prepare-shadow-experiment.py','tools/build-audio-bridge.py','tools/build-shadow-bridge.py','runtime/shadow/win32_min.h']
        audio_sources += [p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/audio').iterdir() if p.is_file()]
        report['sources'].update({p:sha(ROOT/p) for p in audio_sources})
    music_sources=['audio/music_bridge.cpp','audio/music_bridge.hpp','audio/music_catalog.cpp','audio/music_catalog.hpp','audio/music_service.cpp','audio/music_service.hpp','audio/music_output.cpp','protocols/include/mnm/music_v1.h','protocols/python/mnm_protocols/music_v1.py','protocols/schemas/music-v1.json','tools/native_music_launch.py','reconstruction/audio/cd_track_controls.cpp','reconstruction/audio/cd_track_controls.hpp']
    report['sources'].update({p:sha(ROOT/p) for p in music_sources})
    if args.native_draw:
        native_sources=['tools/build-native-draw.py','tools/prepare-native-draw.py']+[str(p.relative_to(ROOT)) for p in (ROOT/'runtime/native-draw').iterdir() if p.suffix in ('.c','.h')]
        report['sources'].update({p:sha(ROOT/p) for p in native_sources})
        # The render bridge also executes the imported shadow build recipe and
        # checks generated wire headers against their JSON schemas. Declare
        # these inputs before building or launching, then keep the existing
        # exact compiler/protocol manifest comparison at completion.
        render_sources=['tools/build-render-bridge.py','tools/build-shadow-bridge.py']
        render_sources += [p.relative_to(ROOT).as_posix() for directory in ('runtime/render','protocols')
                           for p in (ROOT/directory).rglob('*')
                           if p.is_file() and p.suffix in ('.c','.h','.py','.json')]
        report['sources'].update({p:sha(ROOT/p) for p in render_sources})
    if args.resident_world:
        if not args.native_draw:raise ValueError('Resident World requires native drawing')
        # Declare the complete repository compiler-input superset before launch;
        # retain exact current bytes rather than claiming old renderer evidence.
        for directory in ('apps','assets','audio','compat','reconstruction','renderer','runtime','protocols'):
            for source in (ROOT/directory).rglob('*'):
                if source.is_file() and (source.suffix in ('.cpp','.hpp','.h','.c','.S','.py') or source.name=='CMakeLists.txt'):
                    report['sources'][source.relative_to(ROOT).as_posix()]=sha(source)
        report['sources']['tools/build-scene-observer.py']=sha(ROOT/'tools/build-scene-observer.py')
    env = clean_environment(out)
    # Regression proof requires fingerprints even when the normal launcher uses fast asset staging.
    if args.native_draw:env['MNM_VERIFY_ASSETS']='1'
    env['MNM_SYNC_ROUTE_CAPTURE']='1' if args.synchronous_route_capture else '0'
    env['MNM_NO_ROUTE_SCREENSHOTS']='1' if args.no_route_screenshots else '0'
    env['MNM_DISPLAY_WHILE_PENDING']='1' if args.display_while_pending else '0'
    env['MNM_NATIVE_HITCH_CAPTURE']='1' if args.hitch_capture else '0'
    if args.frame_trace:env['MNM_NATIVE_FRAME_TRACE']='1'
    # Make the comparison independent of an inherited experiment setting.
    env['MNM_WORLD_IMAGE_REUSE']='1' if args.reuse_world_image else '0'
    env['MNM_ASYNC_WORLD_GPU']='1' if args.async_world else '0'
    if args.worker_status_cadence:env['MNM_WORKER_STATUS_CADENCE']='1'
    if args.worker_status_every_turn:env['MNM_WORKER_STATUS_EVERY_TURN']='1'
    if args.trace_engine_cadence:
        if not args.resident_world or not args.normal_quit:raise ValueError('Engine cadence requires resident World and normal Quit')
        env['MNM_ENGINE_CADENCE_TRACE']='1'
    if args.scene_production_probe:
        if not args.resident_world or not args.normal_quit or args.trace_engine_cadence:raise ValueError('Scene production requires isolated resident World and normal Quit')
        env['MNM_SCENE_PRODUCTION_PROBE']=str(('observe','full','slice','frozen-history','prepared-scene','budgeted-scene').index(args.scene_production_probe))
    if args.source_game:
        source_game=args.source_game.resolve()
        if not source_game.is_relative_to(ROOT/'working') or not source_game.is_dir():raise ValueError('Campaign source game must be under working/')
        env['MNM_MENU_GAME_SOURCE']=str(source_game)
        report['source_game']=str(source_game)
    if args.resident_world:
        env['MNM_PUBLIC_RESIDENT_WORLD']='1'
        env['MNM_RESIDENT_POLL_TIMING']='1'
    env.pop('MNM_NATIVE_DISABLE_DSOUND',None)
    if args.disable_wine_dsound:env['MNM_NATIVE_DISABLE_DSOUND']='1'
    if args.wine_import_census:env['MNM_WINE_IMPORT_CENSUS']='1'
    if args.profile_resident:env.update(MNM_RESIDENT_STAGE_PROFILE='1',MNM_RENDER_PACER_OBSERVE='1')
    if args.capture_resident:env['MNM_RESIDENT_PROFILE_CAPTURE']=str(out/'resident-source.bin')
    if args.legacy_directdraw:env['MNM_NATIVE_DRAW']='0'
    elif explicit_native_draw:env['MNM_NATIVE_DRAW']='1'
    shell = server = monitor = None
    verified = False
    exit_code = 1
    try:
        with (out / 'build.log').open('w') as log:
            subprocess.run([str(ROOT / 'tools/run-qt-shell.sh'), '--help'], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        binary = ROOT / 'working/build/qt-shell/mnm-qt-shell'
        report['shell_sha256'] = sha(binary)
        if args.default_music or args.default_media:
            with (out/'audio-policy.log').open('x') as policy_log:
                subprocess.run(['python3',str(ROOT/'tests/test-native-audio-policy.py'),'--binary',str(binary)],cwd=ROOT,env=env,stdout=policy_log,stderr=subprocess.STDOUT,check=True,timeout=120)
            report['audio_policy_checked']=True
        if args.native_draw:
            closure=linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT)
            report['compiled_sources']={p:sha(ROOT/p) for p in sorted(closure)}
            missing=closure-set(report['sources'])
            if missing:raise ValueError('Undeclared native shell compiler dependencies: '+repr(sorted(missing)))

        cache=(ROOT/'working/build/qt-shell/CMakeCache.txt').read_text().splitlines()
        report['native_build_type']=next((v.split('=',1)[1] for v in cache if v.startswith('CMAKE_BUILD_TYPE:STRING=')),None)
        report['stage'] = 'combined-mode-admission'
        # A smoke probe opens/closes the shell without starting the game. The
        # invalid combination currently exits 2; never silently retry one mode.
        probe_env = dict(env, QT_QPA_PLATFORM='offscreen')
        command = [str(binary), '--repo', str(ROOT), '--live-menus', '--native-commands', '--smoke-test']
        if explicit_native_audio:command.append('--native-voices')
        if explicit_native_media:command.append('--native-media')
        if explicit_native_music:command.append('--native-music')
        if args.legacy_media:command.append('--legacy-media')
        if args.no_native_music:command.append('--no-native-music')
        if args.skip_movies:command.append('--skip-movies')
        if args.legacy_audio:command.append('--legacy-audio')
        if args.resident_world:command.append('--resident-world')
        if args.independent_world:command.append('--independent-world')
        with (out / 'mode-probe.log').open('w') as log:
            probe = subprocess.run(command, cwd=ROOT, env=probe_env, stdout=log,
                                   stderr=subprocess.STDOUT, timeout=20)
        report['mode_probe'] = {'command': command, 'exit_code': probe.returncode}
        if probe.returncode:
            report['status'] = 'blocked'
            raise RuntimeError('Public shell rejects native menus + native commands (exit 2).' if probe.returncode == 2
                               else 'Combined-mode smoke probe failed; inspect mode-probe.log.')
        if args.check_only:
            report.update(status='admitted-not-exercised', stage='admission-complete')
            exit_code = 0
        else:
            report['stage'] = 'runtime-prerequisites'
            for tool in ['wine', 'wineserver'] + (['tesseract'] if args.require_casting_combat else []) + ([] if args.display or args.hardware else ['Xvfb']) + (['nvidia-smi','glxinfo'] if args.hardware else []):
                if not shutil.which(tool):
                    report['status'] = 'blocked'
                    raise RuntimeError('Missing runtime prerequisite: ' + tool)
            if args.hardware:
                server=start_native_hardware(out,env)
                report['hardware_driver']=dict(driver='zink',accelerated=True,nvidia=True,software_fallback=False,log_sha256=sha(out/'hardware-driver.log'))
            elif args.display:
                env['DISPLAY'] = args.display
            else:
                with (out / 'display').open('w+') as display, (out / 'xvfb.log').open('w') as log:
                    server = subprocess.Popen(['Xvfb', '-displayfd', str(display.fileno()), '-screen', '0',
                                               '1280x1024x24', '-nolisten', 'tcp'], pass_fds=(display.fileno(),),
                                              stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                    deadline = time.monotonic() + 10
                    while time.monotonic() < deadline:
                        display.seek(0)
                        number = display.read().strip()
                        if number:
                            env['DISPLAY'] = ':' + number
                            env['LIBGL_ALWAYS_SOFTWARE'] = '1'
                            env['LP_NUM_THREADS'] = str(args.software_threads)
                            report['software_renderer_threads'] = args.software_threads
                            break
                        if server.poll() is not None:
                            raise RuntimeError('Private X display exited')
                        time.sleep(.1)
                    else:
                        raise RuntimeError('Private X display did not start')
            report['stage'] = 'original-manifest-before'
            with (out / 'original-before.log').open('w') as log:
                subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], cwd=ROOT,
                               stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
            verified = True
            report['stage'] = 'campaign-flow'
            env['MNM_CAMPAIGN_SMOKE_TEST'] = '1'
            env['MNM_CAMPAIGN_DIFFICULTY'] = str(args.difficulty)
            env['MNM_CAMPAIGN_CASTING_COMBAT'] = '1' if args.require_casting_combat or args.healing_case or args.crowded_case else '0'
            env['MNM_CAMPAIGN_PRECISE_CLOCK']='0' if args.disable_precise_clock else '1'
            env['MNM_CAMPAIGN_PACER_YIELD']='1' if args.pacer_yield else '0'
            env['MNM_NATIVE_HEALING']='1' if args.healing_case or args.crowded_case else '0'
            env['MNM_NATIVE_CROWDED']='1' if args.crowded_case else '0'
            env['MNM_CAMPAIGN_PORTRAIT_SECONDS'] = str(args.portrait_stress_seconds)
            env['MNM_CAMPAIGN_NORMAL_QUIT']='1' if args.normal_quit else '0'
            env['MNM_CAMPAIGN_SPELL_CASES'] = '1' if args.spell_cases else '0'
            env['MNM_CAMPAIGN_STRESS_SECONDS'] = str(args.stress_seconds)
            env['MNM_CAMPAIGN_MENU_CYCLES'] = str(args.menu_cycles)
            env['MNM_CAMPAIGN_TIMEOUT_MS'] = str((args.timeout-5)*1000)
            command = [str(ROOT / 'tools/run-qt-shell.sh'), '--live-menus', '--native-commands',
                       '--live-menu-test', str(out / 'flow.json')]
            if explicit_native_audio:command.append('--native-voices')
            if explicit_native_media:command.append('--native-media')
            if explicit_native_music:command.append('--native-music')
            if args.legacy_media:command.append('--legacy-media')
            if args.no_native_music:command.append('--no-native-music')
            if args.skip_movies:command.append('--skip-movies')
            if args.legacy_audio:command.append('--legacy-audio')
            report['launch_command'] = command
            if args.resident_world:command.append('--resident-world')
            if args.independent_world:command.append('--independent-world')
            with (out / 'shell.log').open('w') as log:
                shell = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log,
                                         stderr=subprocess.STDOUT, start_new_session=True)
                report['launch_requested'] = True
                deadline = time.monotonic() + args.timeout
                while time.monotonic() < deadline:
                    lines = (out / 'shell.log').read_text(errors='replace').splitlines()
                    roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
                    if monitor is None and len(roots)==1 and roots[0].is_relative_to(ROOT/'working/experiments/menu-observer') and (roots[0]/'render-frame.bin').exists():
                        monitor=subprocess.Popen(['python3',str(ROOT/'tools/profile-render-stream.py'),str(roots[0]/'render-frame.bin'),'--output',str(out/'publication-rate.jsonl'),'--seconds',str(args.timeout)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,start_new_session=True)
                    if len(roots) == 1 and roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer') and world_ready(roots[0]):
                        (out / 'flow.json.world-ready').touch(exist_ok=True)
                    if (out / 'flow.json').exists():
                        break
                    if shell.poll() is not None:
                        raise RuntimeError('Public shell exited before writing campaign evidence')
                    time.sleep(.2)
                else:
                    raise RuntimeError('Public campaign flow exceeded its deadline')
            flow = json.loads((out / 'flow.json').read_text())
            report['flow'] = flow
            if args.resident_world and len(roots)==1 and roots[0].is_relative_to(ROOT/'working/experiments/menu-observer'):
                resident_path=roots[0]/'resident-world-report.json'
                if resident_path.exists():
                    report['resident_world']=json.loads(resident_path.read_text());report['experiment']=str(roots[0])
                    if report['resident_world']['error']:raise ValueError('Resident World rendering stopped: '+report['resident_world']['error'])
            expected_capture_policy='omitted' if args.no_route_screenshots else 'synchronous' if args.synchronous_route_capture else 'bounded-background-encoding'
            if not (args.healing_case or args.crowded_case) and flow.get('route_capture_policy')!=expected_capture_policy:raise ValueError('Route capture policy differs from requested mode')
            if flow.get('route_screenshots',True)!= (not args.no_route_screenshots):raise ValueError('Route screenshot mode differs from requested comparison')
            if args.no_route_screenshots and any(not step.get('route_capture_skipped') or any(step.get(k) for k in ('screenshot_saved','native_image_saved','original_image_saved')) for step in flow['steps'] if step['step']!='campaign-handoff'):raise ValueError('Route screenshot omission not explicitly recorded')
            if flow.get('success') is not True:raise ValueError(flow.get('error') or 'Campaign flow failed')
            if args.hardware and ('NVIDIA' not in flow.get('renderer','') or any(x in flow['renderer'].lower() for x in ('llvmpipe','lavapipe','softpipe'))):raise ValueError('Actual Qt renderer did not retain NVIDIA hardware identity')
            if not (args.healing_case or args.crowded_case):validate_flow(flow, args.menu_cycles,normal_quit=args.normal_quit)
            if args.stress_seconds and not (args.healing_case or args.crowded_case):
                for cycle in range(args.menu_cycles+1):
                    inputs=json.loads((out/f'input-{cycle}.json').read_text())
                    if inputs.get('success') is not True or inputs['seconds']<args.stress_seconds or len(inputs['actions'])<20:raise RuntimeError('Incomplete gameplay input sequence')
                report['gameplay_input_exercised']=True
            for step in flow['steps']:
                if not args.no_route_screenshots and step['step'] not in ('campaign-handoff','healing-loadout','crowded-setup'):
                    if step['step'] in ('gameplay','gameplay-resumed') and (step.get('native_image_saved') is not True or step.get('original_image_saved') is not True):raise RuntimeError('Missing independent native/original gameplay image')
                    shot = Path(step['screenshot']).resolve()
                    if shot.parent != out or not shot.is_file() or shot.stat().st_size == 0:
                        raise RuntimeError('Missing or redirected screenshot: ' + str(shot))
            lines = (out / 'shell.log').read_text(errors='replace').splitlines()
            roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
            if len(roots) != 1 or not roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer'):
                raise RuntimeError('Missing unique menu experiment identity')
            report['experiment'] = str(roots[0])
            if args.resident_world:
                resident=json.loads((roots[0]/'resident-world-report.json').read_text())
                if resident['error'] or min(int(resident[k]) for k in ('completed','imports','verified_returns'))<64 or any(resident[k] for k in ('world_readbacks','native_readbacks','rgba_readbacks')):
                    raise ValueError('Resident World completion/import/no-readback proof failed: '+repr(resident))
                if args.normal_quit and (not resident.get('normal_quit_retired') or resident['pending'] or len({int(resident[k]) for k in ('completed','imports','verified_returns')})!=1):raise ValueError('Resident World normal Quit was not acknowledged and balanced')
                if args.async_world:
                    production=resident.get('world_gpu_production',{})
                    if production.get('policy')!='dedicated-native-owner' or production.get('error') or production.get('high_water',3)>2 or int(production.get('immutable_allocations',3))>2:
                        raise ValueError('Native GPU production ownership/bound proof failed')
                    if args.normal_quit and (production.get('outstanding',1) or any(int(production.get(k,-1))!=int(resident['completed']) for k in ('submitted','produced','taken'))):
                        raise ValueError('Native GPU production normal Quit did not balance')
                    if int(resident.get('retained_world_image_captures',-1))!=int(resident['completed']) or resident.get('source_high_water',3)>2:
                        raise ValueError('Native GPU production image/source credit proof failed')
                if resident['engine_fps']<args.min_fps:raise ValueError('Resident World engine FPS below target: '+str(resident['engine_fps']))
                if args.hardware and ('NVIDIA' not in resident['renderer'] or any(x in resident['renderer'].lower() for x in ('llvmpipe','lavapipe','softpipe'))):raise ValueError('Resident World did not use NVIDIA hardware')
                report['resident_world']=resident
                scene=json.loads((roots[0]/'scene-bridge-build.json').read_text())
                report['scene_bridge_build']=scene
            if args.native_voices:
                report['native_audio']=validate_audio_activity(json.loads((roots[0]/'native-audio-report.json').read_text()),native_wave=args.native_loading,native_factory=args.native_factory,allow_initial_silence=args.initial_silent_start)
                if args.initial_silent_start and (report['native_audio'].get('initial_silence') is not True or report['native_audio'].get('output_silent') is not True or report['native_audio'].get('output_available') is not False):
                    raise ValueError('Silent-start proof did not retain the initially absent physical output')
                report['native_audio_channel']=validate_audio_retirement(roots[0]/'native-voices.bin',report['native_audio'])
                audio_build=json.loads((roots[0]/'audio-bridge-build.json').read_text())
                manifest=json.loads((roots[0]/'manifest.json').read_text())
                if not manifest.get('native_voices') or sha(roots[0]/'game/MnmAudio.dll')!=manifest['audio_dll_sha256'] or audio_build['sha256']!=manifest['audio_dll_sha256']:raise ValueError('Native audio staging/build fingerprint mismatch')
                if manifest.get('native_audio_profiles') is not True or report['native_audio'].get('profile_hook_installed') is not True or any(report['native_audio'].get(key,0)<1 for key in ['profile_strings','profile_sections','profile_integers']):
                    raise ValueError('Native audio profile hooks/queries were not confirmed')
                if manifest.get('native_audio_file_sizes') is not True or report['native_audio'].get('file_size_hook_installed') is not True:
                    raise ValueError('Native audio file-size hook was not confirmed')
                if args.native_factory and not manifest.get('native_audio_factory'):raise ValueError('Native factory was not staged')
                for name,digest in audio_build['compiled_sources'].items():
                    if report['sources'].get(name)!=digest or sha(ROOT/name)!=digest:raise ValueError('Undeclared or changed native audio compiler source: '+name)
                report['native_audio_compiled_sources']=audio_build['compiled_sources']
            if args.native_media:
                media=json.loads((roots[0]/'native-media-report.json').read_text())
                availability=[entry for entry in media if entry.get('operation')==4]
                if not availability or any(entry.get('status')!=3 or entry.get('native_pipeline_available') is not True for entry in availability):
                    raise ValueError('Native startup media availability acknowledgement missing')
                policy=json.loads((roots[0]/'native-audio-launch-policy.json').read_text())
                if policy.get('native_media_startup_probe') is not True or (args.native_factory and policy.get('wine_directsound_disabled') is not True):
                    raise ValueError('Native media startup/module policy was not confirmed')
                report['native_audio_launch_policy']=policy
                movies=[entry for entry in media if entry.get('operation')==1]
                if not movies or any(entry.get('status') not in (3,4) or not entry.get('accepted') or entry.get('error') or
                                     entry.get('video_frames',0)<1 or entry.get('audio_frames',0)<1 or
                                     entry.get('audible_output_requested') is not True or
                                     (entry.get('audio_device_available') is not True and not (args.initial_silent_start and entry.get('initial_silent_output') is True)) for entry in movies):
                    raise ValueError('Native movie decoding/output/completion history missing or failed')
                if any(entry.get('status') in (5,6) for entry in media):raise ValueError('Native media rejected or failed a submitted request')
                report['native_media']=media
            if args.native_music:
                from native_music_launch import validate_activity as validate_music,catalog_inputs
                report['native_music']=validate_music(json.loads((roots[0]/'native-music-report.json').read_text()),allow_initial_silence=args.initial_silent_start)
                manifest=json.loads((roots[0]/'manifest.json').read_text());inputs,toc=catalog_inputs(roots[0]/'native-music')
                if inputs!=manifest['music_inputs'] or [list(x) for x in toc]!=manifest['music_toc']:raise ValueError('Native music staged inputs changed')
                report['native_music_inputs']=inputs;report['native_music_inputs_stable']=True
            if args.native_draw:
                report['native_draw']=validate_native_draw(roots[0])
                native_build=json.loads((roots[0]/'native-draw-build.json').read_text())
                if native_build.get('directdraw_imports') is not False:raise ValueError('Native graphics DLL imports DirectDraw')
                for name,digest in native_build['sources'].items():
                    if report['sources'].get(name)!=digest or sha(ROOT/name)!=digest:raise ValueError('Undeclared or changed native graphics compiler source: '+name)
                report['native_draw_compiled_sources']=native_build['sources']
                render_build=json.loads((roots[0]/'render-bridge-build.json').read_text())
                if not render_build.get('compiled_sources'):raise ValueError('Missing rendering bridge compiler provenance')
                for field in ('compiled_sources','protocol_sources'):
                    for name,digest in render_build[field].items():
                        if report['sources'].get(name)!=digest or sha(ROOT/name)!=digest:raise ValueError('Undeclared or changed rendering bridge input: '+name)
                report['render_bridge_compiled_sources']=render_build['compiled_sources']
                report['render_bridge_protocol_sources']=render_build['protocol_sources']
                game_inputs=json.loads((roots[0]/'input-fingerprints.json').read_text())
                report['inputs']={str(roots[0]/'game'/name):digest for name,digest in game_inputs.items()}
                report['inputs_stable']=all(sha(Path(name))==digest for name,digest in report['inputs'].items())
                if not report['inputs_stable']:raise ValueError('Native graphics game inputs changed during smoke')
            report['native_precise_clock']={'enabled':not args.disable_precise_clock,'scope':'Real loaded WinMM timeGetTime uptime in guarded original clock import; game speed and wait target unchanged; full timing/physics equivalence excluded'}
            if not args.disable_precise_clock:
                log=roots[0]/'lock-capture/lifecycle.log'
                if not log.exists() or 'native_precise_clock ' not in log.read_text():raise ValueError('Native precise uptime clock was not admitted')
            report['native_pacer_yield']={'enabled':args.pacer_yield,'scope':'Guarded original wait call0x4e3f8b Sleep0 then original clock forwarding; original game speed/frame target retained; timing equivalence excluded'}
            if args.pacer_yield:
                log=roots[0]/'lock-capture/lifecycle.log'
                if not log.exists() or 'native_pacer_yield ' not in log.read_text():raise ValueError('Native cooperative wait was not observed')
            report['native_draw_cadence']=json.loads((roots[0]/'manifest.json').read_text())['native_draw_cadence']
            if report['native_draw_cadence']['settings']!={'SkipFrameEvery':0,'SkipXFrames':0,'MaxSkipXFrames':0}:raise RuntimeError('Native draw skipping remained enabled')
            exports = sorted(p.name for p in (roots[0]/'lock-capture').glob('*.bin')
                             if p.name.startswith(('lock-', 'blit-', 'update-')))
            report['ordinary_diagnostic_binary_exports'] = exports
            if exports:raise RuntimeError('Ordinary native presentation wrote diagnostic binary archives')
            if args.healing_case:
                report['healing']=validate_healing(roots[0],json.loads((out/'healing-input.json').read_text()),flow,out,args.stress_seconds,args.min_fps)
                report['healing_verified']=True
            elif args.crowded_case:
                crowded_flow=dict(flow,healing_mode=True,steps=[s for s in flow['steps'] if s['step']!='crowded-setup'])
                report['healing']=validate_healing(roots[0],json.loads((out/'healing-input.json').read_text()),crowded_flow,out,args.stress_seconds,args.min_fps)
                report['crowded']=validate_crowded(roots[0],json.loads((out/'healing-input.json').read_text()),flow,out,args.stress_seconds,args.min_fps)
                report['crowded_verified']=True
            else:report['engine'] = validate_events(roots[0], args.menu_cycles, args.difficulty,args.normal_quit)
            if args.normal_quit:
                for n in (0,1):
                    q=json.loads((out/f'quit-input-{n}.json').read_text())
                    if not q['success'] or q['answer']!=('no' if n==0 else 'yes'):raise ValueError('Incomplete native Quit confirmation input')
                    cap=q['capture']
                    if cap['metadata']['fallback'] or not all(cap['metadata'][r+'_saved'] for r in ('native','original')):raise ValueError('Missing independent Quit image')
                    for route in ('native','original'):
                        if sha(out/(cap['name']+'-'+route+'.png'))!=cap['image_sha256'][route]:raise ValueError('Quit capture changed after input')
                if 'Original game Quit accepted;' not in (out/'shell.log').read_text() or 'Menu launcher exited with status 0.' not in (out/'shell.log').read_text():raise ValueError('Original normal Quit launcher did not exit successfully')
                shell.wait(timeout=15)
                if shell.returncode!=0:raise ValueError('Native shell did not exit normally after original Quit')
                report['normal_quit_verified']=True
            if args.require_casting_combat:
                report['dialogue']=validate_dialogue_readiness(json.loads((out/'input-0.json').read_text()),out,resident_world=args.resident_world)
                report['gameplay']=validate_casting_combat(roots[0],json.loads((out/'input-0.json').read_text()),out)
                report.update(casting_verified=True,combat_verified=True,movement_verified=report['gameplay']['movement']['verified'])
                if args.spell_cases:
                    report['spell_cases']=validate_spell_cases(read_spell_rows(roots[0]/'spell-events.bin',complete=True),read_rows(roots[0]/'gameplay-events.bin',complete=True),json.loads((out/'input-0.json').read_text()))
            if args.portrait_stress_seconds:
                report['portrait']=validate_portrait_stress(json.loads((out/'input-0.json').read_text()),flow,out,args.portrait_stress_seconds,args.min_fps)
            if not (args.healing_case or args.crowded_case):validate_flow(flow,args.menu_cycles,args.stress_seconds,args.min_fps,args.normal_quit)
            if any(sha(ROOT / p) != h for p,h in report['sources'].items()):raise RuntimeError('Source changed during execution')
            report.update(success=True, status='passed', stage='campaign-complete', flow_completed=True, game_entry_verified=True)
            exit_code = 0
    except (KeyboardInterrupt, OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error) or 'Campaign test interrupted'
        if report['status'] == 'running':
            report['status'] = 'failed'
    finally:
        try:
            stop_group(monitor)
            profile=out/"publication-rate.jsonl"
            if profile.exists():
                try:report["original_owned_publication_profile"]=[json.loads(line) for line in profile.read_text().splitlines()]
                except (OSError,ValueError) as error:
                    report.update(success=False,status='failed',profile_error=str(error))
                    exit_code=1
            report["sources_unchanged"]=all(sha(ROOT/path)==expected for path,expected in report["sources"].items())
            corpus=out/'resident-source.bin'
            if corpus.exists():report['resident_capture']={'path':str(corpus),'bytes':corpus.stat().st_size,'sha256':sha(corpus)}
            if not report['sources_unchanged']:
                report.update(success=False,status='failed',error='Declared sources changed during the run')
                exit_code=1
            stop_group(shell)
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, status='failed', cleanup_error=str(error))
            exit_code = 1
        if (out / 'wineprefix').exists():
            try:
                stopped=subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, timeout=10, check=False)
                # Normal Quit may have already stopped this private server.
                # Verify absence with a bounded wait even when -k reports that
                # no server remains; other failures retain the failed result.
                if stopped.returncode not in (0,1):raise RuntimeError('Private wineserver stop failed')
                subprocess.run(['wineserver', '-w'], env=env, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, timeout=15, check=True)
                report['private_wine_stopped']=True
                if args.wine_import_census and report.get('success'):
                    from wine_import_census import decode as decode_census
                    census_file=roots[0]/'wine-import-census.bin'
                    report['wine_import_census']=decode_census(census_file,True)
                    census_build=json.loads((roots[0]/'api-census-build.json').read_text())
                    if any(report['sources'].get(path)!=digest for path,digest in census_build['compiled_sources'].items()):
                        raise ValueError('Undeclared or changed census compiler source')
                    report['wine_import_census_compiled_sources']=census_build['compiled_sources']
                    report['wine_import_census_file']=str(census_file)
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                report.update(success=False, status='failed', cleanup_error=str(error))
                exit_code = 1
        try:
            stop_group(server)
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, status='failed', display_cleanup_error=str(error))
            exit_code = 1
        if verified:
            try:
                with (out / 'original-after.log').open('w') as log:
                    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], cwd=ROOT,
                                   stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
                report['original_manifest_verified_after'] = True
            except (OSError, subprocess.SubprocessError) as error:
                report.update(success=False, status='failed', original_manifest_error=str(error))
                exit_code = 1
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(report['status'] + ': ' + report.get('error', report['stage']), flush=True)
    print('Report: ' + str(out / 'report.json'), flush=True)
    return exit_code


if __name__ == '__main__':
    raise SystemExit(main())
