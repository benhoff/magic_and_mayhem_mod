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
from campaign_spell import read_spell_rows, validate_spell_observation, validate_spell_cases, player_cure_healing

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
SOURCES += ['apps/qt-shell/healing_smoke_test.cpp', 'tools/campaign-healing-input.py', 'apps/qt-shell/live_battle_menu_controller.cpp', 'apps/qt-shell/live_spell_menu_controller.cpp', 'apps/qt-shell/spellbox_widget.cpp', 'apps/qt-shell/spellbox_widget.hpp', 'tools/campaign-quit-input.py','tools/campaign_spell.py', 'tests/campaign-spell-test.py', 'tools/campaign_movement.py', 'tests/campaign-movement-test.py', 'runtime/menu/campaign_spell_observe.h', 'renderer/blit.cpp', 'renderer/blit.hpp', 'tools/native_render_config.py']


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



def validate_dialogue_readiness(inputs, capture_root):
    checks=inputs.get('dialogue_checks',[])
    if not 1<=len(checks)<=12 or set(checks[-1]['visible'])!={'native','original'} or any(checks[-1]['visible'].values()):
        raise ValueError('Missing independently cleared introductory dialogue')
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
    return dict(verified=True,checks=len(checks),dismissed=sum(all(i['visible'].values()) for i in checks),
                scope='Bounded introductory Hermes heading OCR from independent native/original images; other dialogue speakers/layouts and internal tutorial admission excluded')


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
    if not lethal:raise ValueError('No observed player party lethal enemy health depletion')
    return dict(casting_verified=True,combat_verified=True,summoned_slot=slot,player_owner=owner,
                movement=validate_movement_probes(rows,inputs),
                spells=validate_spell_observation(read_spell_rows(experiment/'spell-events.bin',complete=True),rows,inputs),
                spell_trace_sha256=sha(experiment/'spell-events.bin'),
                enemy_health_depleted=True,lethal_melee_events=len(lethal),first_lethal_melee=list(lethal[0]),
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
    return hashlib.sha256(path.read_bytes()).hexdigest()


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
    if any(s.get('screenshot_saved') is not True for s in steps if s['step'] != 'campaign-handoff'):
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
    if channel[:16] != b'MNMMCM12' + struct.pack('<II', 12, 106496) or len(channel) != 106496:
        raise ValueError('Campaign test did not use the ordinary V12 menu protocol')
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
            'protocol_version': 12, 'events_sha256': sha(experiment / 'events.bin')}


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stress-seconds', type=int, default=0, help='Exercise physical gameplay input per phase (0 or 10..120)')
    parser.add_argument('--menu-cycles', type=int, choices=range(1,6), default=1, help='Number of native Escape/Mini/Cancel returns')
    parser.add_argument('--min-fps', type=float, default=20, help='Minimum average native publications and visible Qt paints during stress')
    parser.add_argument('--difficulty', type=int, choices=range(4), default=0, help='Choose the normal Region Entry difficulty via its native radio (0 Initiate; 1 Apprentice for camera/combat stress)')
    parser.add_argument('--require-casting-combat', action='store_true', help='Require successful native-viewport summon and player-versus-enemy melee damage observation')
    parser.add_argument('--portrait-stress-seconds', type=int, default=0, help='Repeat portrait recentering after verified combat (0 or 10..120)')
    parser.add_argument('--healing-case',action='store_true',help='Separate native Quick Battle Cure loadout and injured human-wizard healing case')
    parser.add_argument('--normal-quit',action='store_true',help='Require native Mini Quit No/Yes, native report/Main and original normal process exit')
    parser.add_argument('--spell-cases', action='store_true', help='Require invalid-target and insufficient-mana refusal plus player Fireball damage through native gameplay input')
    parser.add_argument('--claims', type=Path, help='Prospective coverage scope/source declaration prepared before this run')
    parser.add_argument('--check-only', action='store_true', help='Build and check combined-mode admission; no game launch')
    parser.add_argument('--display', help='Use this X11/XWayland display instead of a private Xvfb (moves focus and sends Escape)')
    parser.add_argument('--software-threads', type=int, choices=range(1,65), default=4, help='Explicit Mesa worker count for the private software-rendered display')
    parser.add_argument('--timeout', type=int, default=330, help='Bound live automation, in seconds (30..600)')
    args = parser.parse_args()
    if args.healing_case:
        if args.require_casting_combat or args.spell_cases or args.normal_quit or args.portrait_stress_seconds:parser.error('Healing is a separate Quick Battle journey')
        if not args.stress_seconds:args.stress_seconds=20
    if args.require_casting_combat:
        if args.difficulty not in (1,2,3):parser.error('Casting/combat requires --difficulty 1, 2 or 3 (other campaign journeys remain separate)')
        if not args.stress_seconds:args.stress_seconds=60
    if args.spell_cases and not args.require_casting_combat:parser.error('--spell-cases requires casting/combat')
    if not 30 <= args.timeout <= 600:
        parser.error('--timeout must be between 30 and 600')
    if args.portrait_stress_seconds and (not args.require_casting_combat or not 10 <= args.portrait_stress_seconds <= 120):parser.error('Portrait stress requires casting/combat and 10..120 seconds')
    if args.stress_seconds and not 10 <= args.stress_seconds <= 120:parser.error('--stress-seconds must be 0 or 10..120')
    if not 1 <= args.min_fps <= 120:parser.error('--min-fps must be 1..120')
    parent = ROOT / 'working/tests/native-campaign'
    parent.mkdir(parents=True, exist_ok=True)
    import tempfile
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print('Campaign test evidence: ' + str(out), flush=True)
    report = {'schema': 1, 'success': False, 'status': 'running', 'stage': 'build',
              'required_mode': ['native-menus', 'native-command-presentation'],
              'launch_requested': False, 'game_entry_verified': False, 'flow_completed': False, 'movement_verified': False,
              'complete_drawing_replacement': False, 'cleanup': 'bounded private-session termination',
              'casting_combat_required':args.require_casting_combat,'casting_verified':False,'combat_verified':False,
              'portrait_stress_seconds':args.portrait_stress_seconds,
              'healing_required':args.healing_case,'spell_cases_required':args.spell_cases,'normal_quit_required':args.normal_quit,
              'difficulty': args.difficulty, 'stress_seconds_per_phase': args.stress_seconds, 'menu_cycles': args.menu_cycles, 'minimum_fps': args.min_fps,
              'sources': {p: sha(ROOT / p) for p in SOURCES}}
    if args.claims:
        declaration=json.loads(args.claims.read_text())
        for path,expected in declaration['sources'].items():
            if Path(path).is_absolute() or not (ROOT/path).resolve().is_relative_to(ROOT):raise ValueError('Unsafe declared source')
            if sha(ROOT/path)!=expected:raise ValueError('Declared source changed before execution: '+path)
        report['claims']=declaration['claims'];report['sources'].update(declaration['sources'])
    env = clean_environment(out)
    shell = server = monitor = None
    verified = False
    exit_code = 1
    try:
        with (out / 'build.log').open('w') as log:
            subprocess.run([str(ROOT / 'tools/run-qt-shell.sh'), '--help'], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        binary = ROOT / 'working/build/qt-shell/mnm-qt-shell'
        report['shell_sha256'] = sha(binary)
        cache=(ROOT/'working/build/qt-shell/CMakeCache.txt').read_text().splitlines()
        report['native_build_type']=next((v.split('=',1)[1] for v in cache if v.startswith('CMAKE_BUILD_TYPE:STRING=')),None)
        report['stage'] = 'combined-mode-admission'
        # A smoke probe opens/closes the shell without starting the game. The
        # invalid combination currently exits 2; never silently retry one mode.
        probe_env = dict(env, QT_QPA_PLATFORM='offscreen')
        command = [str(binary), '--repo', str(ROOT), '--live-menus', '--native-commands', '--smoke-test']
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
            for tool in ['wine', 'wineserver'] + (['tesseract'] if args.require_casting_combat else []) + ([] if args.display else ['Xvfb']):
                if not shutil.which(tool):
                    report['status'] = 'blocked'
                    raise RuntimeError('Missing runtime prerequisite: ' + tool)
            if args.display:
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
            env['MNM_CAMPAIGN_CASTING_COMBAT'] = '1' if args.require_casting_combat or args.healing_case else '0'
            env['MNM_NATIVE_HEALING']='1' if args.healing_case else '0'
            env['MNM_CAMPAIGN_PORTRAIT_SECONDS'] = str(args.portrait_stress_seconds)
            env['MNM_CAMPAIGN_NORMAL_QUIT']='1' if args.normal_quit else '0'
            env['MNM_CAMPAIGN_SPELL_CASES'] = '1' if args.spell_cases else '0'
            env['MNM_CAMPAIGN_STRESS_SECONDS'] = str(args.stress_seconds)
            env['MNM_CAMPAIGN_MENU_CYCLES'] = str(args.menu_cycles)
            env['MNM_CAMPAIGN_TIMEOUT_MS'] = str((args.timeout-5)*1000)
            command = [str(ROOT / 'tools/run-qt-shell.sh'), '--live-menus', '--native-commands',
                       '--live-menu-test', str(out / 'flow.json')]
            report['launch_command'] = command
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
            if not args.healing_case:validate_flow(flow, args.menu_cycles,normal_quit=args.normal_quit)
            if args.stress_seconds and not args.healing_case:
                for cycle in range(args.menu_cycles+1):
                    inputs=json.loads((out/f'input-{cycle}.json').read_text())
                    if inputs.get('success') is not True or inputs['seconds']<args.stress_seconds or len(inputs['actions'])<20:raise RuntimeError('Incomplete gameplay input sequence')
                report['gameplay_input_exercised']=True
            for step in flow['steps']:
                if step['step'] not in ('campaign-handoff','healing-loadout'):
                    if step['step'] in ('gameplay','gameplay-resumed') and (step.get('native_image_saved') is not True or step.get('original_image_saved') is not True):raise RuntimeError('Missing independent native/original gameplay image')
                    shot = Path(step['screenshot']).resolve()
                    if shot.parent != out or not shot.is_file() or shot.stat().st_size == 0:
                        raise RuntimeError('Missing or redirected screenshot: ' + str(shot))
            lines = (out / 'shell.log').read_text(errors='replace').splitlines()
            roots = [Path(line[20:]).resolve() for line in lines if line.startswith('Evidence directory: ')]
            if len(roots) != 1 or not roots[0].is_relative_to(ROOT / 'working/experiments/menu-observer'):
                raise RuntimeError('Missing unique menu experiment identity')
            report['experiment'] = str(roots[0])
            report['native_draw_cadence']=json.loads((roots[0]/'manifest.json').read_text())['native_draw_cadence']
            if report['native_draw_cadence']['settings']!={'SkipFrameEvery':0,'SkipXFrames':0,'MaxSkipXFrames':0}:raise RuntimeError('Native draw skipping remained enabled')
            if args.healing_case:
                report['healing']=validate_healing(roots[0],json.loads((out/'healing-input.json').read_text()),flow,out,args.stress_seconds,args.min_fps)
                report['healing_verified']=True
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
                report['dialogue']=validate_dialogue_readiness(json.loads((out/'input-0.json').read_text()),out)
                report['gameplay']=validate_casting_combat(roots[0],json.loads((out/'input-0.json').read_text()),out)
                report.update(casting_verified=True,combat_verified=True,movement_verified=report['gameplay']['movement']['verified'])
                if args.spell_cases:
                    report['spell_cases']=validate_spell_cases(read_spell_rows(roots[0]/'spell-events.bin',complete=True),read_rows(roots[0]/'gameplay-events.bin',complete=True),json.loads((out/'input-0.json').read_text()))
            if args.portrait_stress_seconds:
                report['portrait']=validate_portrait_stress(json.loads((out/'input-0.json').read_text()),flow,out,args.portrait_stress_seconds,args.min_fps)
            if not args.healing_case:validate_flow(flow,args.menu_cycles,args.stress_seconds,args.min_fps,args.normal_quit)
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
            if profile.exists():report["original_owned_publication_profile"]=[json.loads(line) for line in profile.read_text().splitlines()]
            report["sources_unchanged"]=all(sha(ROOT/path)==expected for path,expected in report["sources"].items())
            stop_group(shell)
        except (OSError, subprocess.SubprocessError) as error:
            report.update(success=False, status='failed', cleanup_error=str(error))
            exit_code = 1
        if (out / 'wineprefix').exists():
            try:
                subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, timeout=10, check=True)
            except (OSError, subprocess.SubprocessError) as error:
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
