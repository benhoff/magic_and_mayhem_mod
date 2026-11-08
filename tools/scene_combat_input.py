"""Bounded physical first-contact observation; original AI owns actual combat."""
import ctypes as c
import json
import math
import os
import struct
import time

from campaign_dialogue import ActorQuietWindow
from campaign_input_admission import spell_selection_points
from campaign_gameplay import creatures, read_rows
from campaign_route import aim_point, approach_point, frame_size, hud_point
from scene_effects_input import exercise_views
from scene_actor_observation import decode


def first_frame_extent(path):
    """Read one complete live producer frame, using the production wire reader."""
    with path.open('rb') as stream:
        raw=stream.read(192)
        if len(raw)<192:return None
        count=struct.unpack_from('<I',raw,64)[0]
        if not 128<=count<=128+256*800+4096*80:
            raise ValueError('Scene input frame extent')
        tail=stream.read(count-128)
    if len(tail)!=count-128:return None
    observed=decode(raw+tail,complete=False)
    if observed['version']!=2:
        raise ValueError('Unsupported scene input observation version')
    frame=observed['frames'][0]
    if not any(a[0]==0 and a[1]==0 and a[2]==0 and a[12]&1 for a in frame['actors']):
        raise ValueError('No observed player body before scene input')
    return frame['extent']


def run(args):
    size = frame_size(tuple(map(int, os.environ['MNM_CAMPAIGN_PROGRESSION_SIZE'].split('x'))))
    width, height = size
    x0, y0, w, h = args.rect
    x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
    x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
    x.XKeysymToKeycode.argtypes = [c.c_void_p, c.c_ulong]; x.XKeysymToKeycode.restype = c.c_uint
    xt.XTestFakeKeyEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_int, c.c_ulong]
    display = x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not display:
        raise RuntimeError('No isolated scene input display')
    started = time.monotonic(); actions = []; held = set()
    q = dict(schema=1, policy='physical-wizard-first-contact-v1', success=False,
             simulation_writes=False, requested_extent=list(size), actions=actions,
             original_party_lethal_proved=False, original_spell_cast_proved=False)
    trace = args.experiment/'gameplay-events.bin'

    def record(action, **values):
        actions.append(dict(action=action, seconds=time.monotonic()-started, **values))
        args.output.write_text(json.dumps(q, indent=2)+'\n')

    def click(point, button=1):
        px, py = point
        if not 0 <= px < width or not 0 <= py < height:
            raise ValueError('Scene physical input outside actual frame')
        if not xt.XTestFakeMotionEvent(display, -1, x0+round(w*px/width), y0+round(h*py/height), 0):
            raise RuntimeError('Scene motion refused')
        x.XFlush(display); time.sleep(.12)
        try:
            if not xt.XTestFakeButtonEvent(display, button, 1, 0):
                raise RuntimeError('Scene button refused')
            held.add(button); x.XFlush(display); time.sleep(.15)
        finally:
            xt.XTestFakeButtonEvent(display, button, 0, 0); held.discard(button); x.XFlush(display)
        record('click', frame_point=list(point), button=button)
        time.sleep(.15)

    def dismiss():
        key = x.XKeysymToKeycode(display, 0xff0d)
        try:
            if not key or not xt.XTestFakeKeyEvent(display, key, 1, 0):
                raise RuntimeError('Dialogue Return refused')
            x.XFlush(display); time.sleep(.15)
        finally:
            xt.XTestFakeKeyEvent(display, key, 0, 0); x.XFlush(display)
        record('quiet-state-return-hint')

    try:
        deadline = started+180
        while not trace.exists() or not creatures(read_rows(trace)):
            if time.monotonic()>started+12:
                raise RuntimeError('No original actor observations')
            time.sleep(.2)
        while True:
            captures = list(args.experiment.rglob('scene-actors-0001.bin'))
            if len(captures)>1:
                raise ValueError('Ambiguous actual scene actor stream')
            actual=first_frame_extent(captures[0]) if captures else None
            if actual is not None:
                if tuple(actual)!=size:
                    raise ValueError('Actual World extent differs from physical input policy')
                q['actual_extent']=actual;break
            if time.monotonic()>started+12:
                raise RuntimeError('No exact scene generation before physical input')
            time.sleep(.2)
        actors = creatures(read_rows(trace))
        heroes = [v for v in actors.values() if v['type']==0 and v['owner']==0 and v['active'] and v['health']>0]
        if len(heroes)!=1:
            raise ValueError('Expected one living original player wizard')
        wizard = heroes[0]; q['wizard'] = wizard; begin = read_rows(trace)[-1][0]
        q['before_sequence'] = begin
        quiet = ActorQuietWindow(seconds=4, dismissals=16)
        click((width//2,height//2))
        for order in range(32):
            rows = read_rows(trace); actors = creatures(rows)
            hero = actors.get(wizard['slot'])
            if not hero or hero['type']!=0 or hero['owner']!=0 or not hero['active'] or hero['health']<=0:
                raise ValueError('Wizard identity/liveness lost before scene observation')
            hits = [r for r in rows if r[0]>begin and r[1]==2 and r[12]<r[11]
                    and (r[4]==wizard['slot'] or r[8]==wizard['slot'])]
            if hits:
                q['melee'] = list(hits[0]); q['after'] = hero
                record('actual-original-melee', event_sequence=hits[0][0])
                targets=[v for v in actors.values() if v['type']==10 and v['owner']!=0 and v['active'] and v['health']>0]
                if targets and hero['health']>80:
                    for _ in range(2):click(hud_point((744,522),size))
                    time.sleep(.6)
                    fresh=creatures(read_rows(trace));hero=fresh[wizard['slot']]
                    target=min((fresh[v['slot']]for v in targets),key=lambda v:math.hypot(v['x']-hero['x'],v['y']-hero['y']))
                    point=aim_point(hero,target,size,body=True)
                    if point is not None and target['health']>0:
                        for icon in spell_selection_points('fireball'):click(hud_point(icon,size))
                        record('ordinary-fireball-request',wizard=hero,target=target,frame_point=list(point))
                        click(point,3)
                time.sleep(1)
                exercise_views(display,x,xt,record)
                time.sleep(1)
                q['success'] = True; record('scene-combat-observation-complete')
                return
            if time.monotonic()>deadline:
                raise RuntimeError('No actual wizard combat before bounded deadline')
            if quiet.observe(actors,time.monotonic()):
                dismiss(); time.sleep(.5)
            enemies = [v for v in actors.values() if v['type']==10 and v['owner']!=0 and v['active'] and v['health']>0]
            if not enemies:
                raise RuntimeError('No original living Redcap for finite scene route')
            target = min(enemies,key=lambda v:math.hypot(v['x']-hero['x'],v['y']-hero['y']))
            for _ in range(2):
                click(hud_point((744,522),size))
            time.sleep(.6)
            fresh = creatures(read_rows(trace)); hero=fresh[wizard['slot']]; target=fresh[target['slot']]
            point = approach_point(hero,target,size)
            record('ordinary-ground-approach', order=order, wizard=hero, target=target, frame_point=list(point))
            click(point,3); time.sleep(3)
        raise RuntimeError('Finite ground-order budget exhausted before actual melee')
    except Exception as error:
        q['error'] = str(error); record('scene-combat-refused')
        raise
    finally:
        for button in held:
            xt.XTestFakeButtonEvent(display,button,0,0)
        x.XFlush(display); x.XCloseDisplay(display)
